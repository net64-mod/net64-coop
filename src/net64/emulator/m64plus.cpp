//
// Created by henrik on 10.04.19
// Copyright 2019 Net64 Coop Project
// Licensed under GPLv3
// Refer to the LICENSE file included
//

#include "m64plus.hpp"

#include <algorithm>
#include <cmath>
#include "net64/memory/util.hpp"


namespace Net64::Emulator
{

namespace
{

bool failed(Mupen64Plus::Error err)
{
    return err != Mupen64Plus::Error::SUCCESS;
}

}

Mupen64Plus::Mupen64Plus(const std::string& lib_path, std::string root_path, std::string data_path)
:core_{lib_path, std::move(root_path), std::move(data_path)}
{
    assert(!has_instance);
    has_instance = true;
}

Mupen64Plus::~Mupen64Plus()
{
    if(core_.handle())
    {
        stop();
        if(rom_loaded_)
            unload_rom();
    }
    has_instance = false;
}

void Mupen64Plus::add_plugin(const std::string& lib_path)
{
    std::lock_guard g(mutex_);
    assert(!running());

    Plugin plugin{ core_, lib_path };
    plugins_[plugin.info().type] = std::move(plugin);
}

void Mupen64Plus::remove_plugin(M64PTypes::m64p_plugin_type type)
{
    std::lock_guard g(mutex_);
    assert(!running());

    plugins_[type] = {};
}

void Mupen64Plus::load_rom(void* rom_data, std::size_t n)
{
    std::lock_guard g(mutex_);
    assert(!rom_loaded_);

    auto ret {core_.do_cmd(M64PTypes::M64CMD_ROM_OPEN, static_cast<int>(n), rom_data)};
    if(failed(ret))
    {
        std::system_error err(make_error_code(ret), "Failed to load ROM image");
        logger()->error(err.what());
        throw err;
    }
    rom_loaded_ = true;
}

void Mupen64Plus::unload_rom()
{
    std::lock_guard g(mutex_);
    assert(rom_loaded_);
    assert(!running());

    rom_loaded_ = false;

    auto ret{core_.do_cmd(M64PTypes::M64CMD_ROM_CLOSE, 0, nullptr)};
    if(failed(ret))
    {
        auto errc{make_error_code(ret)};
        logger()->warn("Failed to correctly unload ROM: {}", errc.message());
    }
}

void Mupen64Plus::execute()
{
    // NOTE: This code is unfortunately complicated. Take care to not violate the assumptions
    // documented below and in the stop() function.
    assert(rom_loaded_);

    // Take ownership of mutex_, this guarantees nobody will change state_ while we own mutex_.
    // Because we are manually managing mutex_ no exceptions can be thrown while it's locked.
    mutex_.lock();

    // Do nothing if the emulator is not stopped
    if(state_ != State::Stopped)
    {
        mutex_.unlock();
        return;
    }

    state_ = State::Starting;
    core_.prepare_config_file();
    attach_plugins();
    core_.set_state_callback([this](M64PTypes::m64p_core_param param_type, int new_value) noexcept
    {
        if(param_type == M64PTypes::M64CORE_EMU_STATE)
        {
            switch (new_value)
            {
            case M64PTypes::M64EMU_STOPPED:
                // This is to guarantee execute() owns mutex_ when do_cmd returns
                mutex_.lock();
                state_ = State::Stopped;
                break;
            case M64PTypes::M64EMU_RUNNING:
                if(state_ == State::Starting)
                {
                    // To guarantee that stop() cannot be entered while starting we kept mutex_
                    // now that we are running we want to be able to stop, so unlock it.
                    state_ = State::Running;
                    mutex_.unlock();
                    loginfo_noexcept("Started n64 emulation");
                }
                break;
            case M64PTypes::M64EMU_PAUSED:
                mutex_.lock();
                state_ = State::Paused;
                mutex_.unlock();
                break;
            default:
                assert(false);
            }
        }
    });
    loginfo_noexcept("Starting n64 emulation");
    auto ret{core_.do_cmd(M64PTypes::M64CMD_EXECUTE, 0, nullptr)};
    detach_plugins();
    assert(state_ == State::Stopped);

    if(failed(ret))
    {
        std::system_error err{make_error_code(ret), "Failed to execute ROM image"};
        mutex_.unlock();
        logger()->error(err.what());
        throw err;
    }

    loginfo_noexcept("Stopped n64 emulation");
    mutex_.unlock();
}

void Mupen64Plus::stop()
{
    // Take ownership of mutex_, this guarantees nobody will change state_
    // while we own mutex_.
    mutex_.lock();

    // Do nothing if already stopped or stopping
    if(state_ == State::Stopped || state_ == State::Stopping)
    {
        mutex_.unlock();
        return;
    }

    // We cannot be starting because execute() owns mutex_ until state_ == State::Started,
    // because we own mutex_ execute() cannot be between Starting and Started.
    assert(state_ == State::Running || state_ == State::Paused);

    // Stop the emulator
    loginfo_noexcept("Stopping n64 emulation");
    core_.do_cmd(M64PTypes::M64CMD_STOP, 0, nullptr);
    state_ = State::Stopping;
    mutex_.unlock();

    // Wait for the emulator to be stopped
    while(state_ != State::Stopped)
    {
        std::this_thread::yield();
        // Workaround for: https://github.com/mupen64plus/mupen64plus-core/issues/681
        core_.do_cmd(M64PTypes::M64CMD_STOP, 0, nullptr);
    }
}

bool Mupen64Plus::running() const
{
    return state_ != State::Stopped;
}

bool Mupen64Plus::rom_loaded() const
{
    return rom_loaded_;
}

void Mupen64Plus::attach_plugins() noexcept
{
    // We already own mutex_
    using namespace M64PTypes;

    core_.attach_plugin(plugins_[M64PLUGIN_GFX].value());
    core_.attach_plugin(plugins_[M64PLUGIN_AUDIO].value());
    core_.attach_plugin(plugins_[M64PLUGIN_INPUT].value());
    core_.attach_plugin(plugins_[M64PLUGIN_RSP].value());
}

void Mupen64Plus::detach_plugins() noexcept
{
    // We already own mutex_
    using namespace M64PTypes;

    core_.detach_plugin(plugins_[M64PLUGIN_GFX]->info().type);
    core_.detach_plugin(plugins_[M64PLUGIN_AUDIO]->info().type);
    core_.detach_plugin(plugins_[M64PLUGIN_INPUT]->info().type);
    core_.detach_plugin(plugins_[M64PLUGIN_RSP]->info().type);
}

void Mupen64Plus::check_bounds(addr_t addr, usize_t size)
{
    if(addr + size > RAM_SIZE)
    {
        // Out of bounds
        auto errc{make_error_code(Error::INVALID_ADDR)};
        logger()->error("Out of bounds memory access at address {:#x} (size: {:#x})", addr, size);
        throw std::system_error(errc);
    }
}

void Mupen64Plus::loginfo_noexcept(const char* msg) noexcept
{
    try
    {
        logger()->info(msg);
    }
    catch (...)
    {
    }
}

bool Mupen64Plus::has_plugin(M64PTypes::m64p_plugin_type type) const
{
    std::lock_guard g(const_cast<Mupen64Plus*>(this)->mutex_);

    return plugins_[type].has_value();
}

void Mupen64Plus::read_memory(addr_t addr, void* data, usize_t n)
{
    check_bounds(addr, n);

    auto ptr{get_mem_ptr<u8>()};

    // First copy the aligned chunk

    // Calc unaligned leftovers
    usize_t begin_left{BSWAP_SIZE - (addr % BSWAP_SIZE)},
            end_left{(addr + n) % BSWAP_SIZE};

    // Address and length of aligned part
    addr_t aligned_addr{addr + begin_left},
           aligned_len{n - end_left - (aligned_addr - addr)};


    // Copy & bswap the aligned chunk
    std::copy_n(ptr + aligned_addr, aligned_len, reinterpret_cast<u8*>(data) + (aligned_addr - addr));
    Memory::bswap32(reinterpret_cast<u8*>(data) + (aligned_addr - addr), aligned_len);


    // Take care of the unaligned beginning
    auto begin_addr{aligned_addr - BSWAP_SIZE};
    u32 begin_word{};
    read(begin_addr, begin_word);
    Memory::bswap32(&begin_word, sizeof(begin_word));
    std::copy_n(reinterpret_cast<u8*>(&begin_word) + (BSWAP_SIZE - begin_left),
                begin_left, reinterpret_cast<u8*>(data));

    // Unaligned ending
    auto end_addr{aligned_addr + aligned_len};
    u32 end_word{};
    read(end_addr, end_word);
    Memory::bswap32(&end_word, sizeof(end_word));
    std::copy_n(reinterpret_cast<u8*>(&end_word), end_left, reinterpret_cast<u8*>(data) + begin_left + aligned_len);
}

void Mupen64Plus::write_memory(addr_t addr, const void* data, usize_t n)
{
    auto ptr{get_mem_ptr<u8>()};

    // Calc unaligned padding
    usize_t begin_padding{addr % BSWAP_SIZE},
            end_padding{BSWAP_SIZE - ((addr + n) % BSWAP_SIZE)};

    // Aligned chunk
    addr_t aligned_addr{addr - begin_padding},
           aligned_len{n + begin_padding + end_padding};


    check_bounds(aligned_addr, aligned_len);


    std::vector<u8> buf(aligned_len);

    // Read leftovers from ram
    u32 leftovers[2];
    read(aligned_addr, leftovers[0]);
    read(aligned_addr + aligned_len - BSWAP_SIZE, leftovers[1]);

    Memory::bswap32(leftovers, sizeof(leftovers));

    // Write leftovers into buffer
    std::copy_n(reinterpret_cast<u8*>(&leftovers), sizeof(leftovers[0]), buf.begin());
    std::copy_n(reinterpret_cast<u8*>(&leftovers[1]), sizeof(leftovers[1]), buf.end() - BSWAP_SIZE);

    // Write actual data into the buffer
    std::copy_n(reinterpret_cast<const u8*>(data), n, buf.begin() + begin_padding);
    Memory::bswap32(buf.data(), buf.size());

    std::copy_n(buf.begin(), buf.size(), ptr + aligned_addr);
}

void Mupen64Plus::read(addr_t addr, u8& val)
{
    check_bounds(addr, sizeof(val));

    auto ptr{get_mem_ptr<u8>()};

    addr_t real_addr{addr - (2 * (addr % BSWAP_SIZE)) + (BSWAP_SIZE - 1)};

    val = ptr[real_addr];
}

void Mupen64Plus::read(addr_t addr, u16& val)
{
    check_bounds(addr, sizeof(val));

    u8 tmp[2];
    read(addr, tmp[0]);
    read(addr + 1, tmp[1]);

    val = static_cast<u16>((static_cast<u16>(tmp[0]) & 0xffu) << 8u | static_cast<u16>(tmp[1]));
}

void Mupen64Plus::read(addr_t addr, u32& val)
{
    check_bounds(addr, sizeof(val));

    u8 tmp[4];
    read(addr, tmp[0]);
    read(addr + 1, tmp[1]);
    read(addr + 2, tmp[2]);
    read(addr + 3, tmp[3]);

    val = static_cast<u32>(static_cast<u32>(tmp[0]) << 24u | static_cast<u32>(tmp[1]) << 16u |
                           static_cast<u32>(tmp[2]) << 8u | static_cast<u32>(tmp[3]));
}

void Mupen64Plus::read(addr_t addr, u64& val)
{
    check_bounds(addr, sizeof(val));

    u32 tmp[2];
    read(addr, tmp[0]);
    read(addr + 4, tmp[1]);

    val = static_cast<u64>(static_cast<u64>(tmp[0]) << 32u | static_cast<u64>(tmp[1]));
}

void Mupen64Plus::read(addr_t addr, f32& val)
{
    // @todo: make this work unaligned
    assert(addr % BSWAP_SIZE == 0);

    check_bounds(addr, sizeof(val));

    auto ptr{get_mem_ptr<u8>()};

    std::copy_n(ptr + addr, sizeof(val), reinterpret_cast<u8*>(&val));

    Memory::bswap32(&val, sizeof(val));
}

void Mupen64Plus::read(addr_t addr, f64& val)
{
    // @todo: make this work unaligned
    assert(addr % BSWAP_SIZE == 0);

    check_bounds(addr, sizeof(val));

    auto ptr{get_mem_ptr<u8>()};

    std::copy_n(ptr + addr, sizeof(val), reinterpret_cast<u8*>(&val));

    Memory::bswap32(&val, sizeof(val));
}

void Mupen64Plus::write(addr_t addr, u8 val)
{
    check_bounds(addr, sizeof(val));

    auto ptr{get_mem_ptr<u8>()};

    addr_t real_addr{addr - (2 * (addr % BSWAP_SIZE)) + (BSWAP_SIZE - 1)};

    ptr[real_addr] = val;
}

void Mupen64Plus::write(addr_t addr, u16 val)
{
    check_bounds(addr, sizeof(val));

    write(addr + 1, static_cast<u8>(val & 0xffu));
    write(addr, static_cast<u8>((val & 0xff00u) >> 8u));
}

void Mupen64Plus::write(addr_t addr, u32 val)
{
    check_bounds(addr, sizeof(val));

    write(addr + 3, static_cast<u8>(val & 0xffu));
    write(addr + 2, static_cast<u8>((val & 0xff00u) >> 8u));
    write(addr + 1, static_cast<u8>((val & 0xff0000u) >> 16u));
    write(addr, static_cast<u8>((val & 0xff000000u) >> 24u));
}

void Mupen64Plus::write(addr_t addr, u64 val)
{
    check_bounds(addr, sizeof(val));

    write(addr + 4, static_cast<u32>(val & 0xffffffffu));
    write(addr, static_cast<u32>((val & 0xffffffff00000000u) >> 32u));
}

void Mupen64Plus::write(addr_t addr, f32 val)
{
    // @todo: make this work unaligned
    assert(addr % BSWAP_SIZE == 0);

    check_bounds(addr, sizeof(val));

    auto ptr{get_mem_ptr<u8>()};

    Memory::bswap32(&val, sizeof(val));

    std::copy_n(reinterpret_cast<u8*>(&val), sizeof(val), ptr + addr);
}

void Mupen64Plus::write(addr_t addr, f64 val)
{
    // @todo: make this work unaligned
    assert(addr % BSWAP_SIZE == 0);

    check_bounds(addr, sizeof(val));

    auto ptr{get_mem_ptr<u8>()};

    Memory::bswap32(&val, sizeof(val));

    std::copy_n(reinterpret_cast<u8*>(&val), sizeof(val), ptr + addr);
}

} // Net64::Emulator


namespace
{

const struct M64PlusErrorCategory : std::error_category
{
    const char* name() const noexcept override
    {
        return "mupen64plus";
    }
    std::string message(int ev) const override
    {
        using Net64::Emulator::M64PlusHelper::Error;
        
        switch(static_cast<Error>(ev))
        {
        case Error::NOT_INIT:
            return "Function is disallowed before InitMupen64Plus() is called";
        case Error::ALREADY_INIT:
            return "InitMupen64Plus() was called twice";
        case Error::INCOMPATIBLE:
            return "API versions between components are incompatible";
        case Error::INPUT_ASSERT:
            return "Invalid parameters for function call";
        case Error::INPUT_INVALID:
            return "Invalid input data";
        case Error::INPUT_NOT_FOUND:
            return "The input parameter(s) specified a particular item which was not found";
        case Error::NO_MEMORY:
            return "Memory allocation failed";
        case Error::FILES:
            return "Error opening, creating, reading, or writing to a file";
        case Error::INTERNAL:
            return "Internal error (bug)";
        case Error::INVALID_STATE:
            return "Current program state does not allow operation";
        case Error::PLUGIN_FAIL:
            return "A plugin function returned a fatal error";
        case Error::SYSTEM_FAIL:
            return "A system function call, such as an SDL or file operation, failed";
        case Error::UNSUPPORTED:
            return "Function call is not supported";
        case Error::WRONG_TYPE:
            return "A given input type parameter cannot be used for desired operation";

        case Error::LIB_LOAD_FAILED:
            return "Failed to load library file";
        case Error::INVALID_ADDR:
            return "Tried to access out of bounds memory";
        case Error::SYM_NOT_FOUND:
            return "A symbol required by the API could not be located in the specified module";
        default:
            return "[Unknown Error]";
        }
    }
}m64p_error_category_g;

} // anonymous

namespace Net64::Emulator::M64PlusHelper
{

std::error_code make_error_code(Error e) noexcept
{
    return {static_cast<int>(e), m64p_error_category_g};
}

} // Net64::Emulator::M64PlusHelper
