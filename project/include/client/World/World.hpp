/*
** EPITECH PROJECT, 2026
** T-type
** File description:
** World.hpp
*/

#ifndef CLIENT_WORLD_HPP
    #define CLIENT_WORLD_HPP

    #include <cstdint>
    #include <optional>
    #include <unordered_map>
    #include <vector>

    #include "Registry.hpp"

namespace r_type::client {

constexpr std::uint8_t KIND_PLAYER = 0x01;
constexpr std::uint8_t KIND_ENEMY = 0x02;
constexpr std::uint8_t KIND_PROJECTILE = 0x03;

struct EntityData {
    std::uint32_t id = 0;
    std::uint8_t kind = 0;
    std::int32_t x = 0;
    std::int32_t y = 0;
};

struct NetIdentity {
    std::uint32_t id = 0;
};

struct Transform {
    float x = 0.f;
    float y = 0.f;
};

struct Appearance {
    std::uint8_t kind = 0;
    int color = -1;
};

class World {
    ecs::Registry _registry;
    std::unordered_map<std::uint32_t, ecs::Entity> _entities;
    std::optional<std::uint32_t> _player;
    int _playerColor = -1;

public:
    World();

    void Spawn(const EntityData& data);
    void Update(std::uint32_t id, std::int32_t x, std::int32_t y);
    void Destroy(std::uint32_t id);
    void Replace(const std::vector<EntityData>& entities);
    void SetPlayer(std::uint32_t id, std::int32_t x, std::int32_t y, std::uint8_t color);
    void Clear();

    std::optional<std::uint32_t> Player() const {return _player;};
    int PlayerColor() const {return _playerColor;};
    std::size_t Count() const {return _entities.size();};
    ecs::Registry& Registry() {return _registry;};
};

}

#endif
