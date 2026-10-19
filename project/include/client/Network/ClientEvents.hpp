/*
** EPITECH PROJECT, 2026
** T-type
** File description:
** ClientEvents.hpp
*/

#ifndef CLIENT_EVENTS_HPP
    #define CLIENT_EVENTS_HPP

    #include <cstdint>
    #include <string>
    #include <variant>
    #include <vector>

namespace r_type::client {

constexpr std::uint8_t KIND_PLAYER = 0x01;
constexpr std::uint8_t KIND_ENEMY = 0x02;
constexpr std::uint8_t KIND_PROJECTILE = 0x03;

constexpr std::uint8_t INPUT_UP = 1 << 0;
constexpr std::uint8_t INPUT_DOWN = 1 << 1;
constexpr std::uint8_t INPUT_LEFT = 1 << 2;
constexpr std::uint8_t INPUT_RIGHT = 1 << 3;
constexpr std::uint8_t INPUT_FIRE = 1 << 4;
constexpr std::uint8_t INPUT_MASK = 0x1F;

struct RoomInfo {
    std::uint32_t id = 0;
    std::uint8_t freeSlots = 0;
};

struct EntityData {
    std::uint32_t id = 0;
    std::uint8_t kind = 0;
    std::int32_t x = 0;
    std::int32_t y = 0;
};

namespace event {

struct Connected {};
struct ConnectionFailed {
    std::string reason;
};
struct Disconnected {
    std::string reason;
};
struct Notice {
    std::string message;
};
struct Rooms {
    std::vector<RoomInfo> rooms;
};
struct RoomEntered {};
struct BackToLobby {};
struct PlayerCreated {
    std::uint32_t id = 0;
    std::int32_t x = 0;
    std::int32_t y = 0;
    std::uint8_t color = 0;
};
struct Spawn {
    EntityData entity;
};
struct Update {
    std::uint32_t id = 0;
    std::int32_t x = 0;
    std::int32_t y = 0;
};
struct Destroy {
    std::uint32_t id = 0;
};
struct Snapshot {
    std::vector<EntityData> entities;
};

}

using NetEvent = std::variant<event::Connected, event::ConnectionFailed, event::Disconnected, event::Notice,
    event::Rooms, event::RoomEntered, event::BackToLobby, event::PlayerCreated, event::Spawn, event::Update,
    event::Destroy, event::Snapshot>;

namespace request {

struct Connect {
    std::string ip;
};
struct ListRooms {};
struct CreateRoom {};
struct JoinRoom {
    std::uint32_t id = 0;
};
struct Input {
    std::uint8_t held = 0;
};
struct LeaveRoom {};
struct Disconnect {};

}

using NetRequest = std::variant<request::Connect, request::ListRooms, request::CreateRoom, request::JoinRoom,
    request::Input, request::LeaveRoom, request::Disconnect>;

template <class... Ts>
struct Overloaded : Ts... {
    using Ts::operator()...;
};
template <class... Ts>
Overloaded(Ts...) -> Overloaded<Ts...>;

}

#endif
