#pragma once

#include <QByteArray>
#include <cstdint>
#include <cassert>
#include <stdexcept>

namespace Mario20
{

const int MAJOR_VERSION = 2;
const int MINOR_VERSION = 0;
const uint8_t COMPAT_MAJOR_VERSION = 0;
const uint8_t COMPAT_MINOR_VERSION = 4;
const int UPDATE_RATE = 10;
const int MAX_CHAT_LENGTH = 24;
const int HANDSHAKE_LENGTH = MAX_CHAT_LENGTH + 5;

enum class PacketType : uint8_t
{
    Handshake = 0,
    PlayerData = 1,
    GameMode = 2,
    ChatMessage = 3,
    CharacterSwitch = 4,
    RoundtripPing = 5,
    WrongVersion = 6,
};

enum class Character : uint8_t
{
    Mario,
    Luigi,
    Yoshi,
    Wario,
    Peach,
    Toad,
    Waluigi,
    Rosalina, // not working?
};

enum class GameMode : uint8_t
{
    Normal,
};

QByteArray compress(const void* data, size_t size);
QByteArray decompress(const void* data, size_t size);
std::string tohex(const QByteArray& b);

}