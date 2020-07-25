//
// Created by henrik on 03.09.19
// Copyright 2019 Net64 Coop Project
// Licensed under GPLv3
// Refer to the LICENSE file included
//

#include "client.hpp"
#include "net64/client/game_logger.hpp"
#include "net64/client/local_player_manager.hpp"
#include "net64/client/player_list_manager.hpp"
#include "net64/net/errors.hpp"


namespace Net64
{
Client::Client()
{
    // Components
    chat_client_ = new ChatClient;
    add_component(chat_client_);
    add_component(new Game::Logger);
    add_component(new LocalPlayerManager);
    add_component(new PlayerListManager);
    add_component(new ChatClient);
}

Client::~Client()
{
    net_message_handlers_.clear();
    connection_event_handlers_.clear();
    tick_handlers_.clear();
    game_message_handlers_.clear();

    for(auto& iter : components_)
    {
        iter.deleter(iter.component);
    }
}

void Client::set_chat_callback(ChatCallback fn)
{
    chat_client_->set_chat_callback(fn);
}

void Client::hook(Memory::MemHandle mem_hdl, HookedCallback fn)
{
    mem_hdl_ = std::move(mem_hdl);
    hooked_callback_ = std::move(fn);

    hooking_ = true;
}

void Client::connect(const char* ip,
                     std::uint16_t port,
                     std::string username,
                     std::chrono::seconds timeout,
                     ConnectCallback connect_callback,
                     DisconnectCallback disconnect_callback)
{
    assert(!peer_);

    logger()->info("Connecting to {}:{} as {}", ip, port, username);

    connect_callback_ = std::move(connect_callback);
    disconnect_callback_ = std::move(disconnect_callback);
    connect_timeout_ = timeout;
    username_ = std::move(username);

    host_.reset(enet_host_create(nullptr, 1, Net::channel_count(), 0, 0));
    if(!host_)
    {
        logger()->error("Failed to create ENet host");
        connect_callback_(make_error_code(Net::Error::ENET_HOST_CREATION));
        return;
    }

    ENetAddress addr;
    // @todo: check if ip is a domain or ip address
    if(enet_address_set_host(&addr, ip) != 0)
    {
        logger()->error("Failed to establish connection: Hostname resolution failed");
        connect_callback_(make_error_code(Net::Error::UNKOWN_HOSTNAME));
        return;
    }
    addr.port = port;

    PeerHandle peer(enet_host_connect(host_.get(), &addr, Net::channel_count(), Net::PROTO_VER));
    if(!peer)
    {
        logger()->error("Failed to establish connection: Failed to create ENet peer");
        connect_callback_(make_error_code(Net::Error::ENET_PEER_CREATION));
        return;
    }

    connection_start_time_ = Clock::now();
    peer_ = std::move(peer);
}

void Client::disconnect(std::chrono::seconds timeout)
{
    if(connecting())
    {
        logger()->info("Aborting connection attempt");
        peer_.reset();
        host_.reset();
        connect_callback_(make_error_code(Net::Error::TIMED_OUT));
        disconnect_callback_({});
        return;
    }

    if(!connected())
        return;

    logger()->info("Disconnecting from server");

    disconnect_timeout_ = timeout;

    enet_peer_disconnect(peer_.get(), static_cast<std::uint32_t>(Net::C_DisconnectCode::USER_DISCONNECT));
    disconnection_start_time_ = Clock::now();
}

void Client::unhook()
{
    hooking_ = false;

    rcv_queue_ = {};
    snd_queue_ = {};
    net64_header_ = {};
    mem_hdl_ = {};

    disconnect(std::chrono::seconds(2));

    logger()->info("Unhooked");
}

void Client::tick()
{
    if(hooking_)
    {
        try_hook();
        return;
    }

    if(connecting() && Clock::now() - connection_start_time_ > connect_timeout_)
    {
        // Connection timed out
        disconnect(std::chrono::seconds(0));
    }
    if(disconnecting() && (Clock::now() - disconnection_start_time_) > disconnect_timeout_)
    {
        // Disconnecting timed out
        on_disconnect();
        peer_.reset();
        host_.reset();
        disconnect_callback_({});
    }

    if(host_)
    {
        for(ENetEvent evt; enet_host_service(host_.get(), &evt, 0) > 0;)
        {
            switch(evt.type)
            {
            case ENET_EVENT_TYPE_CONNECT:
                logger()->info("Established connection");
                if(connect_callback_)
                    connect_callback_({});
                on_connect();
                break;
            case ENET_EVENT_TYPE_DISCONNECT:
                logger()->info("Disconnected from server");
                if(disconnect_callback_)
                    disconnect_callback_({});
                on_disconnect();
                // ENet already took care of deallocation
                (void)peer_.release();
                host_.reset();
                return;
                break;
            case ENET_EVENT_TYPE_RECEIVE: {
                PacketHandle packet(evt.packet);
                on_net_message(*packet);
                break;
            }
            }
        }
    }

    if(hooked())
    {
        for(Game::MsgQueue::n64_message_t msg{}; rcv_queue_->poll(msg);)
        {
            on_game_message(msg);
        }
    }
}

bool Client::hooked() const
{
    return net64_header_.valid();
}

bool Client::connecting() const
{
    return (peer_ != nullptr && peer_->state == ENET_PEER_STATE_CONNECTING);
}

bool Client::connected() const
{
    return (peer_ != nullptr && peer_->state == ENET_PEER_STATE_CONNECTED);
}

bool Client::disconnecting() const
{
    return (peer_ != nullptr && peer_->state == ENET_PEER_STATE_DISCONNECTING);
}

bool Client::disconnected() const
{
    return !peer_;
}

void Client::try_hook()
{
    if(mem_hdl_->read<u32>(Game::FixedAddr::MAGIC_NUMBER) != Game::MAGIC_NUMBER)
        return;

    hooking_ = false;

    net64_header_ = {*mem_hdl_, mem_hdl_->read<n64_addr_t>(Game::FixedAddr::HEADER_PTR)};
    rcv_queue_ = Game::MsgQueue::Receiver(net64_header_->field(&Game::net64_header_t::receive_queue));
    snd_queue_ = Game::MsgQueue::Sender(net64_header_->field(&Game::net64_header_t::send_queue));

    if(net64_header_->field(&Game::net64_header_t::compat_version) != Game::CLIENT_COMPAT_VER)
    {
        logger()->error("Incompatible patch version. Patch version: {}, Client version: {}",
                        net64_header_->field(&Game::net64_header_t::compat_version).read(),
                        Game::CLIENT_COMPAT_VER);
        hooked_callback_(make_error_code(Net64::ClientError::INCOMPATIBLE_GAME));
        return;
    }

    logger()->info("Initialized Net64 client version {} (Compatibility: {})",
                   net64_header_->field(&Game::net64_header_t::version).read(),
                   net64_header_->field(&Game::net64_header_t::compat_version).read());

    hooked_callback_({});
}

void Client::on_connect()
{
    for(auto iter : connection_event_handlers_)
    {
        iter->on_connect(*this);
    }
}

void Client::on_disconnect()
{
    username_.clear();

    for(auto iter : connection_event_handlers_)
    {
        iter->on_disconnect(*this);
    }
}

void Client::on_net_message(const ENetPacket& packet)
{
    std::istringstream strm;

    strm.str({reinterpret_cast<const char*>(packet.data), packet.dataLength});
    try
    {
        std::unique_ptr<INetMessage> msg{INetMessage::parse_message(strm)};

        for(auto handler : net_message_handlers_)
        {
            handler->handle_message(*msg, *this);
        }
    }
    catch(const std::exception& e)
    {
        logger()->warn("Failed to parse network message: {}", e.what());
    }
}

void Client::on_game_message(const Game::MsgQueue::n64_message_t& message)
{
    for(auto iter : game_message_handlers_)
    {
        iter->push_message(*this, message);
    }
}

void Client::send(const INetMessage& msg)
{
    if(!peer_)
        return;

    std::ostringstream strm;
    {
        cereal::PortableBinaryOutputArchive ar(strm);

        msg.serialize_msg(ar);
    }

    PacketHandle packet(enet_packet_create(strm.str().data(), strm.str().size(), Net::channel_flags(msg.channel())));

    enet_peer_send(peer_.get(), static_cast<std::uint8_t>(msg.channel()), packet.release());
}

void Client::send_chat_message(std::string msg)
{
    chat_client_->send(*this, std::move(msg));
}

ClientSharedData Client::get_client_shared_data(Badge<ClientDataAccess>)
{
    return ClientSharedData{mem_hdl_, snd_queue_, player_, remote_players_};
}

std::string Client::username() const
{
    return username_;
}

const std::unordered_map<Net::player_id_t, RemotePlayer>& Client::remote_players() const
{
    return remote_players_;
}

} // namespace Net64

namespace
{
static const struct Net64ClientErrorCategory : std::error_category
{
    const char* name() const noexcept override { return "net64_client_error"; }

    std::string message(int ev) const override
    {
        using namespace Net64;

        switch(static_cast<ClientError>(ev))
        {
        case ClientError::INCOMPATIBLE_GAME:
            return "Game is incompatible with client";
        }

        return "[Unkown Error]";
    }

} net64_client_error_category_l;

} // namespace

namespace Net64
{
std::error_code make_error_code(Net64::ClientError e)
{
    return {static_cast<int>(e), net64_client_error_category_l};
}

} // namespace Net64
