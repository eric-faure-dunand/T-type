/*
** EPITECH PROJECT, 2026
** T-type
** File description:
** entities.cpp
*/

#include "ClientCore.hpp"

namespace r_type::client {

void ClientCore::OnPlayerCreation(const ArgsList& args) {
    if ((_scene != Scene::Game && _scene != Scene::Joining) || _roomPort == 0
        || args.size() != 1 || args[0].size() != 4)
        return;
    _world.SetPlayer(std::get<std::uint32_t>(args[0][0]), std::get<std::int32_t>(args[0][1]),
        std::get<std::int32_t>(args[0][2]), std::get<std::uint8_t>(args[0][3]));
}

void ClientCore::OnInitialStateBegin(const ArgsList& args) {
    if (_scene != Scene::Game || args.size() != 1 || args[0].size() != 1)
        return;
    const std::uint32_t count = std::get<std::uint32_t>(args[0][0]);
    if (count <= MAX_TRANSFER_ENTITIES)
        _transfer = Transfer{count, {}};
}

void ClientCore::OnInitialStateEnd(const ArgsList& args) {
    if (!_transfer || args.size() != 1 || args[0].size() != 1)
        return;
    const std::uint32_t count = std::get<std::uint32_t>(args[0][0]);
    if (count == _transfer->expected && _transfer->entities.size() == count) {
        std::vector<EntityData> entities;
        for (const auto& [id, entity] : _transfer->entities)
            entities.push_back(entity);
        _world.Replace(entities);
    } else {
        try {
            _net.Connect(_ip, _roomPort);
        } catch (const IError&) {
            ShowMessage("Could not load the game state");
        }
    }
    _transfer.reset();
}

void ClientCore::OnEntitySpawn(const ArgsList& args) {
    if (_scene != Scene::Game || args.size() != 1 || args[0].size() != 4)
        return;
    const EntityData entity{std::get<std::uint32_t>(args[0][0]), std::get<std::uint8_t>(args[0][1]),
        std::get<std::int32_t>(args[0][2]), std::get<std::int32_t>(args[0][3])};
    if (_destroyed.contains(entity.id))
        return;
    if (!_transfer) {
        _world.Spawn(entity);
        return;
    }
    if (_transfer->entities.size() < _transfer->expected || _transfer->entities.contains(entity.id))
        _transfer->entities[entity.id] = entity;
}

void ClientCore::OnEntityUpdate(const ArgsList& args) {
    if (_scene != Scene::Game || args.size() != 1 || args[0].size() != 3)
        return;
    const std::uint32_t id = std::get<std::uint32_t>(args[0][0]);
    if (!_destroyed.contains(id))
        _world.Update(id, std::get<std::int32_t>(args[0][1]), std::get<std::int32_t>(args[0][2]));
}

void ClientCore::OnEntityDestroy(const ArgsList& args) {
    if (_scene != Scene::Game || args.size() != 1 || args[0].size() != 1)
        return;
    const std::uint32_t id = std::get<std::uint32_t>(args[0][0]);
    _destroyed.insert(id);
    if (_transfer)
        _transfer->entities.erase(id);
    _world.Destroy(id);
}

}
