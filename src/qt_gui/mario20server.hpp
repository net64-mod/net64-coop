#pragma once

#include <QObject>
#include <QTimer>
#include <QByteArray>
#include <array>
#include <vector>
#include <initializer_list>
#include "mario20protocol.hpp"

QT_FORWARD_DECLARE_CLASS(QWebSocketServer)
QT_FORWARD_DECLARE_CLASS(QWebSocket)

namespace Mario20
{

class Server : public QObject
{
    Q_OBJECT
public:
    explicit Server(QObject* parent = nullptr);
    ~Server();
    bool listen(uint16_t port);
    bool is_listening() const;

signals:
    void stop();

private slots:
    void on_new_connection();
    void on_binary_message(const QByteArray& message);
    void on_socket_disconnected();
    void on_stop();
    void on_closed();
    void on_timeout();

private:
    struct ClientData
    {
        ClientData() = default;
        ClientData(QWebSocket* socket, uint8_t id) : socket(socket), id(id) { }
        QWebSocket* socket = nullptr;
        uint8_t id = 0;
        uint8_t major = 0;
        uint8_t minor = 0;
        Character character = Character::Mario;
        std::string username;
        std::array<uint8_t, 0x18> player_data = { 0 };

        template<size_t Size>
        void send_packet(Mario20::PacketType type, const std::array<uint8_t, Size>& payload)
        {
            send_packet(type, payload.data(), payload.size());
        }

        void send_packet(PacketType type, const QByteArray& payload)
        {
            send_packet(type, payload.constData(), payload.size());
        }

        void send_packet(PacketType type, const std::vector<uint8_t>& payload)
        {
            send_packet(type, payload.data(), payload.size());
        }

        void send_packet(PacketType type, const std::initializer_list<uint8_t>& payload)
        {
            send_packet(type, std::vector<uint8_t>(payload));
        }

        void send_packet(PacketType type, const void* payload, size_t size);
    };

    auto find_client(QWebSocket* client_socket)
    {
        return std::find_if(m_clients.begin(), m_clients.end(), [client_socket](const auto& client)
        {
            return client.socket == client_socket;
        });
    }

private:
    QWebSocketServer* m_web_socket_server = nullptr;
    std::array<ClientData, 24> m_clients{};
    QTimer* m_timer = nullptr;
};

}