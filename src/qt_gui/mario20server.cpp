#include "mario20server.hpp"

#include <QtWebSockets/QWebSocket>
#include <QtWebSockets/QWebSocketServer>

QT_USE_NAMESPACE

namespace Mario20
{

Server::Server(QObject* parent)
    : QObject(parent)
    , m_web_socket_server(new QWebSocketServer("Mario20Server", QWebSocketServer::NonSecureMode, this))
{
    m_timer = new QTimer(this);
    m_timer->setInterval(10);
    connect(m_timer, &QTimer::timeout, this, &Server::on_timeout);
    connect(m_web_socket_server, &QWebSocketServer::newConnection, this, &Server::on_new_connection);
    connect(m_web_socket_server, &QWebSocketServer::closed, this, &Server::on_closed);
    connect(this, &Server::stop, this, &Server::on_stop);
}

Server::~Server()
{
    m_web_socket_server->close();
}

bool Server::listen(uint16_t port)
{
    if (m_web_socket_server->listen(QHostAddress::Any, port))
    {
        printf("listening on %u...\n", port);
        m_timer->start();
        return true;
    }
    printf("failed to listen: %s\n", m_web_socket_server->errorString().toUtf8().constData());
    return false;
}

bool Server::is_listening() const
{
    return m_web_socket_server->isListening();
}

void Server::on_new_connection()
{
    puts("Server::on_new_connection()");
    QWebSocket* socket = m_web_socket_server->nextPendingConnection();
    auto client = find_client(nullptr);
    if (client != m_clients.end())
    {
        *client = ClientData(socket, uint8_t(client - m_clients.begin()));
        printf("new client connected %u\n", client->id);
        connect(socket, &QWebSocket::binaryMessageReceived, this, &Server::on_binary_message);
        connect(socket, &QWebSocket::disconnected, this, &Server::on_socket_disconnected);
    }
    else
    {
        puts("server full!");
        socket->close();
    }
}

void Server::on_binary_message(const QByteArray& message)
{
    QWebSocket* client_socket = qobject_cast<QWebSocket*>(sender());
    auto client = find_client(client_socket);
    if (client == m_clients.end())
        return;

    switch ((PacketType)message[0])
    {
    case PacketType::Handshake:
    {
        client->major = message[1];
        client->minor = message[2];
        printf("[%u] handshake %u.%u\n", client->id, client->major, client->minor);
        if (client->major != COMPAT_MAJOR_VERSION || client->minor != COMPAT_MINOR_VERSION)
        {
            client->send_packet(PacketType::WrongVersion, { client->major, client->minor });
            return;
        }
        client->character = (Character)message[3];
        int username_len = message[4];
        for (int i = 0; i < username_len; i++)
            client->username.push_back(message[5 + i]);
        client->send_packet(PacketType::Handshake, { client->id });
        client->send_packet(PacketType::GameMode, { (uint8_t)GameMode::Normal });
        printf("[%u] %s joined\n", client->id, client->username.c_str());
    }
    break;

    case PacketType::PlayerData:
    {
        memcpy(client->player_data.data(), message.data() + 1, client->player_data.size());
    }
    break;

    case PacketType::RoundtripPing:
    {
        client->socket->sendBinaryMessage(message);
    }
    break;

    case PacketType::CharacterSwitch:
    {
        client->character = (Character)message[1];
    }
    break;

    default:
    {
        printf("[%u] unsupported message %u\n", client->id, message[0]);
    }
    break;
    }
}

void Server::on_socket_disconnected()
{
    QWebSocket* client_socket = qobject_cast<QWebSocket*>(sender());
    if (client_socket)
    {
        auto client = find_client(client_socket);
        if (client != m_clients.end())
            *client = ClientData{};
        client_socket->deleteLater();
    }
}

void Server::on_stop()
{
    if (is_listening())
        m_web_socket_server->close();
}

void Server::on_closed()
{
    for (auto& client : m_clients)
    {
        delete client.socket;
        client = {};
    }
}

void Server::on_timeout()
{
    if (!is_listening())
    {
        puts("server not listening, stopping timer...");
        m_timer->stop();
    }
    std::vector<uint8_t> payload;
    for (auto& client : m_clients)
    {
        if (!client.socket)
            continue;
        if (client.player_data[3])
        {
            client.player_data[3] = client.id;
            for (size_t i = 0; i < client.player_data.size(); i++)
                payload.push_back(client.player_data[i]);
        }
    }
    auto compressed = compress(payload.data(), payload.size());
    for (auto& client : m_clients)
    {
        if (client.socket)
            client.send_packet(PacketType::PlayerData, compressed);
    }
}

void Server::ClientData::send_packet(PacketType type, const void* payload, size_t size)
{
    QByteArray message;
    message.resize(size + 1);
    message[0] = (char)type;
    memcpy(message.data() + 1, payload, size);
    socket->sendBinaryMessage(message);
}

}
