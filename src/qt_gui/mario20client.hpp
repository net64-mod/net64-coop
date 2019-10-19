#pragma once

#include <array>
#include <functional>
#include <atomic>
#include <QtCore/QObject>
#include <QTimer>
#include <QtWebSockets/QWebSocket>

#include "mario20protocol.hpp"

namespace Mario20
{

class Client : public QObject
{
    Q_OBJECT
public:
    using MemRead = std::function<void(uint32_t, void*, size_t)>;
    using MemWrite = std::function<void(uint32_t, const void*, size_t)>;

    explicit Client(const QUrl& url, QObject* parent = nullptr);
    void open(MemRead memRead, MemWrite memWrite);
    bool is_connected() const { return m_connected; }
    void set_url(const QUrl& url);

    enum class State
    {
        Connecting,
        Connected,
        Disconnected,
    };

signals:
    void close();
    void state_changed(State state);

private slots:
    void on_connected();
    void on_disconnected();
    void on_binary_message_received(const QByteArray& message);
    void on_player_timeout();
    void on_close();

private:
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

    void send_packet(PacketType type, const void* payload, size_t size);

    void write(uint32_t addr, const void* data, size_t size)
    {
        m_write(addr, data, size);
    }

    void read(uint32_t addr, void* data, size_t size)
    {
        m_read(addr, data, size);
    }

private:
    QWebSocket m_websocket;
    QUrl m_url;
    std::atomic<bool> m_connected = false;
    uint8_t m_player_id = 0;
    MemRead m_read = nullptr;
    MemWrite m_write = nullptr;
    QTimer* m_player_timer = nullptr;
};

}

Q_DECLARE_METATYPE(Mario20::Client::State)