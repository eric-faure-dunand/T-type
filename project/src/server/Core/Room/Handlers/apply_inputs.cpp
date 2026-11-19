/*
** EPITECH PROJECT, 2026
** T-type
** File description:
** apply_inputs.cpp
*/

#include "room.hpp"

namespace r_type {

void Room::ApplyInputs() {
    const auto inputs = ConsumePlayerInputs();

    for (const auto& input : inputs) {
        const auto player = _players.find(input.client);
        if (player == _players.end())
            continue;
        const ecs::Entity entity = player->second;
        if (!registre.alive(entity))
            continue;
        if (ecs::entityValue(entity) != input.entityId)
            continue;
        game::applyAction(registre.get<game::Input>(entity), input.inputFlags,
            input.state == InputState::Pressed);
    }
}

}
