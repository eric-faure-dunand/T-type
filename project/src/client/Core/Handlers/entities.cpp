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
    const std::uint32_t id = std::get<std::uint32_t>(args[0][0]);
    const std::uint8_t character = std::get<std::uint8_t>(args[0][3]);
    _world.SetPlayer(id, std::get<std::int32_t>(args[0][1]), std::get<std::int32_t>(args[0][2]), character);
    if (character != _character)
        Send(Command::ChooseCharacter, {{id, _character}});
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
    if (_scene != Scene::Game)
        return;
    for (const Args& record : args) {
        if (record.size() != 5)
            continue;
        const EntityData entity{std::get<std::uint32_t>(record[0]), std::get<std::uint8_t>(record[1]),
            std::get<std::int32_t>(record[2]), std::get<std::int32_t>(record[3]),
            std::get<std::uint8_t>(record[4])};
        if (_destroyed.contains(entity.id))
            continue;
        const int boss = static_cast<int>(entity.color) - FIRST_BOSS;
        if (entity.kind == KIND_ENEMY && boss >= 0 && boss < static_cast<int>(BOSS_NAMES.size())) {
            const std::string name = BOSS_NAMES[static_cast<std::size_t>(boss)];
            const bool known = _boss && _boss->id == entity.id;
            if (!_transfer && !(known && _boss->name == name))
                ShowMessage("BOSS : " + name);
            _boss = BossBar{entity.id, name, known ? _boss->percent : 100};
        }
        if (!_transfer)
            _world.Spawn(entity);
        else if (_transfer->entities.size() < _transfer->expected || _transfer->entities.contains(entity.id))
            _transfer->entities[entity.id] = entity;
    }
}

void ClientCore::OnEntityUpdate(const ArgsList& args) {
    if (_scene != Scene::Game)
        return;
    for (const Args& record : args) {
        if (record.size() != 3)
            continue;
        const std::uint32_t id = std::get<std::uint32_t>(record[0]);
        if (!_destroyed.contains(id))
            _world.Update(id, std::get<std::int32_t>(record[1]), std::get<std::int32_t>(record[2]));
    }
}

void ClientCore::OnEntityDestroy(const ArgsList& args) {
    if (_scene != Scene::Game)
        return;
    for (const Args& record : args) {
        if (record.size() != 1)
            continue;
        const std::uint32_t id = std::get<std::uint32_t>(record[0]);
        if (_world.Player() == id) {
            Send(Command::DisconnectRoom, {{id}});
            ReturnToLobby();
            return;
        }
        _destroyed.insert(id);
        if (_boss && _boss->id == id)
            _boss.reset();
        if (_transfer)
            _transfer->entities.erase(id);
        _world.Destroy(id);
    }
}

void ClientCore::OnBossHealth(const ArgsList& args) {
    if (_scene != Scene::Game)
        return;
    for (const Args& record : args) {
        if (record.size() != 2)
            continue;
        const std::uint32_t id = std::get<std::uint32_t>(record[0]);
        if (_destroyed.contains(id))
            continue;
        const int percent = std::min<int>(std::get<std::uint8_t>(record[1]), 100);
        if (_boss && _boss->id == id)
            _boss->percent = percent;
        else
            _boss = BossBar{id, "BOSS", percent};
    }
}

}
