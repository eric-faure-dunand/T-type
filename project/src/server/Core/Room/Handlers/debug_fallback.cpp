/*
** EPITECH PROJECT, 2026
** T-type
** File description:
** debug_fallback.cpp
*/

#include "room.hpp"

namespace r_type {

void Room::DebugFallback(const Endpoint& from, Command command, const ArgsList&) {
    std::clog << "[ROOM " << _id << "] Unhandled command "
              << static_cast<int>(command) << " from "
              << from.address().to_string() << ":" << from.port()
              << '\n';
}

}
