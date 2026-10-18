/*
** EPITECH PROJECT, 2026
** T-type
** File description:
** debug_fallback.cpp
*/

#include "room.hpp"

namespace r_type {

void Room::DebugFallback(const Endpoint& from, Command command, const ArgsList&) {
    std::clog << Color::BRIGHT_YELLOW
              << "[ROOM " << _id << "] Unhandled command "
              << Color::RESET
              << static_cast<int>(command) << " from "
              << from.address().to_string() << ":" << from.port()
              << '\n';
}

}
