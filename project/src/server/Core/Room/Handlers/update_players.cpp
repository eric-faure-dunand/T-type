/*
** EPITECH PROJECT, 2026
** T-type
** File description:
** update_players.cpp
*/

#include "room.hpp"

namespace r_type {

void Room::UpdatePlayers() {
    constexpr std::int32_t PLAYER_SPEED = 5;
    const auto inputs = ConsumePlayerInputs();

    for (const auto& input : inputs) {
        if (input.state != InputState::Pressed)
            continue;
        const auto player = _players.find(input.client);
        if (player == _players.end())
            continue;
        const ecs::Entity entity = player->second;
        if (!registre.alive(entity))
            continue;
        if (ecs::entityValue(entity) != input.entityId)
            continue;
        Position& position = registre.get<Position>(entity);
        switch (input.inputFlags) {
            case 0:
                position.y -= PLAYER_SPEED;
                break;
            case 1:
                position.y += PLAYER_SPEED;
                break;
            case 2:
                position.x -= PLAYER_SPEED;
                break;
            case 3:
                position.x += PLAYER_SPEED;
                break;
            case 4:
                if (input.state == InputState::Pressed)
                    HandlePlayerShoot(entity);
                break;
            default:
                break;
        }
    }
}

}