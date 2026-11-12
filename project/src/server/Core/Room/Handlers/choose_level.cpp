/*
** EPITECH PROJECT, 2026
** T-type
** File description:
** choose_level.cpp
*/

#include "room.hpp"

namespace r_type {

void Room::HandleChooseLevel(const Endpoint& from, const ArgsList& args) {
    if (_level || !net.HasClient(from) || args.size() != 1 || args[0].size() != 2)
        return;
    const auto player = _players.find(from);
    if (player == _players.end() || std::get<std::uint32_t>(args[0][0]) != ecs::entityValue(player->second))
        return;
    const std::uint8_t level = std::get<std::uint8_t>(args[0][1]);
    if (level >= game::kLevelNames.size())
        return;
    StartLevel(level);
}

void Room::StartLevel(std::uint8_t level) {
    game::LevelData data;
    try {
        data = game::loadLevel(game::levelPath(level));
    } catch (const IError& error) {
        std::cerr << Color::BRIGHT_RED << "[ROOM " << _id << "] " << error.what() << Color::RESET << '\n';
        return;
    }
    _systems = ecs::SystemManager{};
    game::addGameplaySystems(_systems, std::random_device{}(), data);
    _level = level;
    Broadcast(Command::LevelStart, {{level}});
}

}
