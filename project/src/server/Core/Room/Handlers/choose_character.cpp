/*
** EPITECH PROJECT, 2026
** T-type
** File description:
** choose_character.cpp
*/

#include "room.hpp"

namespace r_type {

void Room::HandleChooseCharacter(const Endpoint& from, const ArgsList& args) {
    if (!net.HasClient(from) || args.size() != 1 || args[0].size() != 2)
        return;
    const auto player = _players.find(from);
    if (player == _players.end() || !registre.alive(player->second))
        return;
    const ecs::Entity entity = player->second;
    if (std::get<std::uint32_t>(args[0][0]) != ecs::entityValue(entity))
        return;
    const int character = std::get<std::uint8_t>(args[0][1]);
    if (character >= game::kCharacterCount)
        return;
    const bool taken = std::any_of(_players.begin(), _players.end(), [this, entity, character](const auto& other) {
        const game::Player* id = registre.tryGet<game::Player>(other.second);
        return other.second != entity && id && id->id - 1 == character;
    });
    if (taken)
        return;
    registre.get<game::Player>(entity).id = character + 1;
    Broadcast(Command::EntitySpawn, SpawnArgs(entity));
}

}
