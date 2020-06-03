//
// Created by henrik on 10.04.19
// Copyright 2019 Net64 Coop Project
// Licensed under GPLv3
// Refer to the LICENSE file included
//

#pragma once

#include <array>
#include <atomic>
#include <future>
#include <string>
#include <string_view>
#include <system_error>
#include <optional>
#include <mutex>
#include "net64/emulator/emulator.hpp"
#include "net64/emulator/m64plus_error.hpp"
#include "net64/emulator/m64plus_plugin.hpp"
#include "net64/emulator/shared_library.hpp"
#include "net64/logging.hpp"


namespace Net64::Emulator
{

/**
 * Mupen64Plus instance. Only one instance of this class can be alive at a time.
 */
struct Mupen64Plus final : IEmulator
{
    using Core = M64PlusHelper::Core;
    using Plugin = M64PlusHelper::Plugin;
    using PluginInfo = M64PlusHelper::PluginInfo;
    using Error = M64PlusHelper::Error;

    static constexpr usize_t BSWAP_SIZE{4};

    /// Create emulator from library file
    explicit Mupen64Plus(const std::string& lib_path, std::string root_path, std::string data_path);

    /// Non copyable
    Mupen64Plus(Mupen64Plus&) = delete;

    /// Non movable
    Mupen64Plus(Mupen64Plus&& other) = delete;

    ~Mupen64Plus() override;

    std::unique_ptr<IAudioSettings> audio_settings() override;

    std::unique_ptr<IVideoSettings> video_settings() override;

    std::unique_ptr<IControllerSettings> controller_settings(unsigned short controller) override;

    /// Register a plugin
    void add_plugin(const std::string& lib_path);

    /// Remove a plugin
    void remove_plugin(m64p_plugin_type type);

    /// Load ROM image
    void load_rom(void* rom_data, std::size_t n) override;

    void unload_rom() override;

    //void execute(const StateCallback& fn = {}) override;
    void start(const StateCallback& fn) override;

    void stop() override;

    void join(std::error_code& exit_code) override;

    // Read & Write implementations
    void read_memory(addr_t addr, void* data, usize_t n) override;
    void write_memory(addr_t addr, const void* data, usize_t n) override;

    void read(addr_t addr, u8& val) final;
    void read(addr_t addr, u16& val) final;
    void read(addr_t addr, u32& val) final;
    void read(addr_t addr, u64& val) final;
    void read(addr_t addr, f32& val) final;
    void read(addr_t addr, f64& val) final;

    void write(addr_t addr, u8 val) final;
    void write(addr_t addr, u16 val) final;
    void write(addr_t addr, u32 val) final;
    void write(addr_t addr, u64 val) final;
    void write(addr_t addr, f32 val) final;
    void write(addr_t addr, f64 val) final;

    State state() const override;

    const char* name() const override
    {
        return "mupen64plus";
    }

    bool rom_loaded() const;

    /// Check if plugin of a type is already registered
    bool has_plugin(m64p_plugin_type type) const;

private:
    void attach_plugins() noexcept;

    void detach_plugins() noexcept;

    void prepare_config();

    inline static void logical2physical(addr_t& addr);
    inline static void check_bounds(addr_t addr, usize_t size);
    static void log_noexcept(spdlog::level::level_enum lvl, const char* msg) noexcept;

    template<typename T>
    volatile T* get_mem_ptr()
    {
        volatile auto ptr{core_.get_mem_ptr()};

        if(!ptr)
        {
            // Memory not initalized
            std::system_error err{make_error_code(Error::INVALID_STATE), "Failed to access emulator memory"};
            logger()->error(err.what());
            throw err;
        }

        return reinterpret_cast<volatile T*>(ptr);
    }

    Core core_;
    std::array<std::unique_ptr<Plugin>, M64PLUGIN_MAX> plugins_{};
    std::string emulator_root_;
    bool rom_loaded_{};

    enum class MupenState
    {
        Stopped,
        Stopping,
        Starting,
        Running,
        Paused,
        Joinable
    };
    std::atomic<MupenState> state_{MupenState::Stopped}; // Only modify while owning mutex_!
    mutable std::mutex mutex_;
    std::future<void> emulation_thread_;

    inline static bool has_instance_s{false};

    CLASS_LOGGER_("mupen64plus")
};

} // Net64::Emulator


/// Specialization for Mupen64Plus error codes
template<>
struct std::is_error_code_enum<::Net64::Emulator::Mupen64Plus::Error> : std::true_type{};
