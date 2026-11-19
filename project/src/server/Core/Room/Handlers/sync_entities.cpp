/*
** EPITECH PROJECT, 2026
** T-type
** File description:
** sync_entities.cpp
*/

#include "room.hpp"

namespace r_type {

static std::uint8_t WireKind(game::EntityKind kind) {
    switch (kind) {
        case game::EntityKind::Player:
            return 1;
        case game::EntityKind::Enemy:
            return 2;
        default:
            return 3;
    }
}

static std::uint8_t WireVariant(ecs::Registry& registre, ecs::Entity entity, game::EntityKind kind) {
    if (const game::Player* player = registre.tryGet<game::Player>(entity))
        return static_cast<std::uint8_t>(player->id - 1);
    if (const game::Enemy* enemy = registre.tryGet<game::Enemy>(entity))
        return static_cast<std::uint8_t>(enemy->type);
    return kind == game::EntityKind::EnemyMissile ? 1 : 0;
}

Args Room::SpawnArgs(ecs::Entity entity) {
    const game::Position& position = registre.get<game::Position>(entity);
    const game::Kind& kind = registre.get<game::Kind>(entity);
    return {
        ecs::entityValue(entity),
        WireKind(kind.value),
        static_cast<std::int32_t>(position.x),
        static_cast<std::int32_t>(position.y),
        WireVariant(registre, entity, kind.value)
    };
}

void Room::SendList(const Endpoint& client, Command command, const ArgsList& records) {
    for (std::size_t first = 0; first < records.size(); first += MAX_RECORDS_PER_MESSAGE) {
        const std::size_t last = std::min(records.size(), first + MAX_RECORDS_PER_MESSAGE);
        net.SendTo(client, command, ArgsList(records.begin() + static_cast<std::ptrdiff_t>(first),
            records.begin() + static_cast<std::ptrdiff_t>(last)));
    }
}

void Room::Broadcast(Command command, const ArgsList& records) {
    if (records.empty())
        return;
    for (const auto& [client, entity] : _players) {
        if (!net.HasClient(client))
            continue;
        SendList(client, command, records);
    }
}

void Room::SyncEntities() {
    std::unordered_set<std::uint32_t> alive;
    ArgsList spawns;
    ArgsList destroys;

    registre.each<game::Kind, game::Position>(
        [this, &alive, &spawns](ecs::Entity entity, const game::Kind&, const game::Position&) {
            const std::uint32_t id = ecs::entityValue(entity);
            alive.insert(id);
            if (_synced.insert(id).second)
                spawns.push_back(SpawnArgs(entity));
        }
    );
    for (auto it = _synced.begin(); it != _synced.end();) {
        if (alive.contains(*it)) {
            ++it;
            continue;
        }
        destroys.push_back({*it});
        _bossHealth.erase(*it);
        it = _synced.erase(it);
    }
    Broadcast(Command::EntitySpawn, spawns);
    Broadcast(Command::EntityDestroy, destroys);
    SyncBossHealth();
    if (++_tick % (TICK_RATE / ROOM_UPDATE_RATE) != 0)
        return;
    ArgsList updates;
    registre.each<game::Kind, game::Position>(
        [&updates](ecs::Entity entity, const game::Kind&, const game::Position& position) {
            updates.push_back({
                ecs::entityValue(entity),
                static_cast<std::int32_t>(position.x),
                static_cast<std::int32_t>(position.y)
            });
        }
    );
    Broadcast(Command::EntityUpdate, updates);
}

void Room::SyncBossHealth() {
    registre.each<game::Boss, game::Health>([this](ecs::Entity entity, game::Boss&, game::Health& health) {
        const std::uint32_t id = ecs::entityValue(entity);
        const int current = std::max(health.current, 0);
        const auto percent = static_cast<std::uint8_t>(health.max > 0 ? current * 100 / health.max : 0);
        const auto known = _bossHealth.find(id);
        if (known != _bossHealth.end() && known->second == percent)
            return;
        _bossHealth[id] = percent;
        Broadcast(Command::BossHealth, {{id, percent}});
    });
}

}
