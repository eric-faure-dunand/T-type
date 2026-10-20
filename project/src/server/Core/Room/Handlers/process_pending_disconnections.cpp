/*
** EPITECH PROJECT, 2026
** T-type
** File description:
** process_pending_disconnections.cpp
*/

#include "room.hpp"

namespace r_type {

void Room::ProcessPendingDisconnections() {
    std::deque<Endpoint> pending;
    {
        std::lock_guard<std::mutex> lock(_disconnectionMutex);
        pending.swap(_pendingDisconnections);
    }
    for (const Endpoint& client : pending) {
        HandlePlayerDisconnection(client);
    }
}

}
