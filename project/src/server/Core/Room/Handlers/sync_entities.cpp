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

ArgsList Room::SpawnArgs(ecs::Entity entity) {
    const game::Position& position = registre.get<game::Position>(entity);
    const game::Kind& kind = registre.get<game::Kind>(entity);
    return {{
        ecs::entityValue(entity),
        WireKind(kind.value),
        static_cast<std::int32_t>(position.x),
        static_cast<std::int32_t>(position.y),
        WireVariant(registre, entity, kind.value)
    }};
}

void Room::Broadcast(Command command, const ArgsList& args) {
    for (const auto& [client, entity] : _players) {
        if (!net.HasClient(client))
            continue;
        net.SendTo(client, command, args);
    }
}

void Room::SyncEntities() {
    std::unordered_set<std::uint32_t> alive;

    registre.each<game::Kind, game::Position>(
        [this, &alive](ecs::Entity entity, const game::Kind&, const game::Position&) {
            const std::uint32_t id = ecs::entityValue(entity);
            alive.insert(id);
            if (_synced.insert(id).second)
                Broadcast(Command::EntitySpawn, SpawnArgs(entity));
        }
    );
    for (auto it = _synced.begin(); it != _synced.end();) {
        if (alive.contains(*it)) {
            ++it;
            continue;
        }
        Broadcast(Command::EntityDestroy, {{*it}});
        _bossHealth.erase(*it);
        it = _synced.erase(it);
    }
    SyncBossHealth();
    if (++_tick % (TICK_RATE / ROOM_UPDATE_RATE) != 0)
        return;
    registre.each<game::Kind, game::Position>(
        [this](ecs::Entity entity, const game::Kind&, const game::Position& position) {
            Broadcast(Command::EntityUpdate, {{
                ecs::entityValue(entity),
                static_cast<std::int32_t>(position.x),
                static_cast<std::int32_t>(position.y)
            }});
        }
    );
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
