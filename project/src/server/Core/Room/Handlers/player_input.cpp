/*
** EPITECH PROJECT, 2026
** T-type
** File description:
** player_input.cpp
*/

#include "room.hpp"

namespace r_type {

void Room::HandlePlayerInput(const Endpoint& from, const ArgsList& args) {
    if (!net.HasClient(from) || args.size() != 1 || args[0].size() != 3)
        return;
    const auto player = _players.find(from);
    if (player == _players.end())
        return;
    const ecs::Entity entity = player->second;
    if (!registre.alive(entity))
        return;
    const std::uint32_t entityId = std::get<std::uint32_t>(args[0][0]);
    const std::uint8_t action = std::get<std::uint8_t>(args[0][1]);
    const std::uint8_t state = std::get<std::uint8_t>(args[0][2]);
    if (entityId != ecs::entityValue(entity))
        return;
    if (action > 4)
        return;
    if (state > static_cast<std::uint8_t>(InputState::Pressed))
        return;
    if (_pendingInputs.size() >= MAX_PENDING_INPUTS)
        _pendingInputs.pop_front();
    _pendingInputs.push_back({
        from,
        entityId,
        action,
        static_cast<InputState>(state)
    });
}

std::deque<Room::PlayerInputEvent> Room::ConsumePlayerInputs() {
    std::deque<PlayerInputEvent> inputs;
    inputs.swap(_pendingInputs);
    return inputs;
}

}
