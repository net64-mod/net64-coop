#include "mario20client.hpp"

QT_USE_NAMESPACE

namespace Mario20
{

Client::Client(const QUrl& url, QObject* parent) :
    QObject(parent),
    m_url(url)
{
    qRegisterMetaType<Client::State>();
    m_player_timer = new QTimer(this);
    m_player_timer->setInterval(1);
    connect(&m_websocket, &QWebSocket::connected, this, &Client::on_connected);
    connect(&m_websocket, &QWebSocket::disconnected, this, &Client::on_disconnected);
    connect(m_player_timer, &QTimer::timeout, this, &Client::on_player_timeout);
    connect(this, &Client::close, this, &Client::on_close);
}

void Client::open(MemRead memRead, MemWrite memWrite)
{
    state_changed(State::Connecting);
    m_websocket.open(m_url);
    m_read = std::move(memRead);
    m_write = std::move(memWrite);
    assert(m_read && m_write);
    m_player_timer->start();
}

void Client::set_url(const QUrl& url)
{
    m_url = url;
}

void Client::on_connected()
{
    connect(&m_websocket, &QWebSocket::binaryMessageReceived, this, &Client::on_binary_message_received);
    state_changed(State::Connected);
    m_connected = true;
    printf("[client] connected!\n");
    std::string username = "net64-coop";
    QByteArray payload;
    payload.resize(HANDSHAKE_LENGTH);
    payload.fill('\0');
    payload[0] = (uint8_t)PacketType::Handshake;
    payload[1] = COMPAT_MAJOR_VERSION;
    payload[2] = COMPAT_MINOR_VERSION;
    payload[3] = (uint8_t)Character::Mario;
    payload[4] = username.size();
    for (size_t i = 0; i < std::min(username.size(), size_t(HANDSHAKE_LENGTH - 5)); i++)
        payload[(int)i + 5] = username[i];
    m_websocket.sendBinaryMessage(payload);
}

void Client::on_disconnected()
{
    m_connected = false;
    state_changed(State::Disconnected);
}

void Client::on_binary_message_received(const QByteArray& message)
{
    auto hexstr = tohex(message);
    bool printBytes = true;
    switch ((PacketType)message[0])
    {
    case PacketType::Handshake:
    {
        uint8_t one = 1;
        write(0x365FFC, &one, 1); // let client think that he is host
        write(0x367703, &one, 1); // let client think that he has player ID 1
        m_player_id = message[1];
        printf("[client] handshake client id: %d\n", m_player_id);
    }
    break;

    case PacketType::PlayerData:
    {
        auto decompressed = decompress(message.constData() + 1, message.size() - 1);
        //printf("uncompressed.size(): %d\n", decompressed.size());
        assert(decompressed.size() % 0x18 == 0);
        int j = 2;
        for (int i = 0; i < decompressed.size(); i += 0x18)
        {
            if (m_player_id == decompressed[i + 3])
                continue;
            std::array<uint8_t, 0x18> playerData;
            memcpy(playerData.data(), decompressed.constData() + i, playerData.size());
            playerData[3] = j;
            write(0x367700 + 0x100 * j, playerData.data(), playerData.size());
            j++;
        }
        std::array<uint8_t, 0x18> empty;
        memset(empty.data(), 0, empty.size());
        for (; j < 24; j++)
        {
            write(0x367700 + 0x100 * j, empty.data(), empty.size());
        }
        printBytes = false;
    }
    break;

    case PacketType::GameMode:
    {
        printf("[client] gamemode: %d\n", message[1]);
        write(0x365FF7, message.constData() + 1, 1);
    }
    break;
    
    case PacketType::ChatMessage:
    {
        auto payload = message.constData() + 1;
        auto chatLen = *payload++;
        auto chat = std::string(payload, chatLen);
        payload += chatLen;
        auto userLen = *payload++;
        auto user = std::string(payload, userLen);
        printf("[client] chat: [%s] %s\n", user.c_str(), chat.c_str());
    }
    break;
    
    case PacketType::CharacterSwitch:
    {
        printf("[client] character switch %d\n", message[1]);
    }
    break;

    case PacketType::RoundtripPing:
    {
        printBytes = false;
    }
    break;

    case PacketType::WrongVersion:
    {
        printf("[client] wrong version %d.%d\n", message[1], message[2]);
    }
    break;

    default:
        printf("[client] received unknown message type %02X\n", (uint8_t)message[0]);
    }
    if (printBytes)
        printf("[client] received mesage: %s\n", hexstr.c_str());
}

void Client::send_packet(PacketType type, const void* payload, size_t size)
{
    QByteArray message;
    message.resize(size + 1);
    message[0] = (char)type;
    memcpy(message.data() + 1, payload, size);
    if (m_connected)
        m_websocket.sendBinaryMessage(message);
}

void Client::on_player_timeout()
{
    if (is_connected())
    {
        std::array<uint8_t, 0x18> payload;
        read(0x367700, payload.data(), payload.size());
        if (payload[0xF] != 0)
        {
            send_packet(PacketType::PlayerData, payload);
            write(0x367800, payload.data(), payload.size());
        }
    }
}

void Client::on_close()
{
    if (m_connected)
        m_websocket.close();
}

}