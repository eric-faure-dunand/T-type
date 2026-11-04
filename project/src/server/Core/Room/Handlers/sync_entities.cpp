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

ArgsList Room::SpawnArgs(ecs::Entity entity) {
    const game::Position& position = registre.get<game::Position>(entity);
    const game::Kind& kind = registre.get<game::Kind>(entity);
    const game::Player* player = registre.tryGet<game::Player>(entity);
    return {{
        ecs::entityValue(entity),
        WireKind(kind.value),
        static_cast<std::int32_t>(position.x),
        static_cast<std::int32_t>(position.y),
        static_cast<std::uint8_t>(player ? player->id - 1 : 0)
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
        it = _synced.erase(it);
    }
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

}
