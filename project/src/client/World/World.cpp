/*
** EPITECH PROJECT, 2026
** T-type
** File description:
** World.cpp
*/

#include "World.hpp"

namespace r_type::client {

World::World() {
    _registry.registerComponent<NetIdentity>();
    _registry.registerComponent<Transform>();
    _registry.registerComponent<Appearance>();
}

void World::Spawn(const EntityData& data) {
    const int color = _player && *_player == data.id ? _playerColor : -1;
    const auto it = _entities.find(data.id);
    if (it != _entities.end()) {
        _registry.get<Transform>(it->second) = {static_cast<float>(data.x), static_cast<float>(data.y)};
        _registry.get<Appearance>(it->second) = {data.kind, color};
        return;
    }
    if (_registry.aliveCount() >= ecs::kMaxEntities)
        return;
    const ecs::Entity entity = _registry.spawn();
    _registry.add<NetIdentity>(entity, NetIdentity{data.id});
    _registry.add<Transform>(entity, Transform{static_cast<float>(data.x), static_cast<float>(data.y)});
    _registry.add<Appearance>(entity, Appearance{data.kind, color});
    _entities.emplace(data.id, entity);
}

void World::Update(std::uint32_t id, std::int32_t x, std::int32_t y) {
    const auto it = _entities.find(id);
    if (it != _entities.end())
        _registry.get<Transform>(it->second) = {static_cast<float>(x), static_cast<float>(y)};
}

void World::Destroy(std::uint32_t id) {
    const auto it = _entities.find(id);
    if (it == _entities.end())
        return;
    _registry.kill(it->second);
    _entities.erase(it);
}

void World::Replace(const std::vector<EntityData>& entities) {
    for (const auto& [id, entity] : _entities)
        _registry.kill(entity);
    _entities.clear();
    for (const EntityData& data : entities)
        Spawn(data);
}

void World::SetPlayer(std::uint32_t id, std::int32_t x, std::int32_t y, std::uint8_t color) {
    _player = id;
    _playerColor = color;
    Spawn({id, KIND_PLAYER, x, y});
}

void World::Clear() {
    for (const auto& [id, entity] : _entities)
        _registry.kill(entity);
    _entities.clear();
    _player.reset();
    _playerColor = -1;
}

}
