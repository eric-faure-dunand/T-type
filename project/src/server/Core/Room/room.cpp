/*
** EPITECH PROJECT, 2026
** T-type
** File description:
** room.cpp
*/

#include "room.hpp"

namespace r_type {

Room::Room(std::uint32_t id):
    _id(id), net(Role::Server, DEFAULT_ROOM_CLIENTS, true), _lastAnnounce(std::chrono::steady_clock::now()) {
    game::registerGameplayComponents(registre);
    game::addGameplaySystems(_systems, std::random_device{}(), game::LevelData{});

    net.On(Command::ConnectRequest, [this](const Endpoint& from, const ArgsList&) {
        HandlePlayerConnection(from);
    });
    net.On(Command::DisconnectRoom, [this](const Endpoint& from, const ArgsList&) {
        HandlePlayerDisconnection(from);
    });
    net.On(Command::Disconnect, [this](const Endpoint& from, const ArgsList&) {
        HandlePlayerDisconnection(from);
    });
    net.On(Command::PlayerInput, [this](const Endpoint& from, const ArgsList& args) {
        HandlePlayerInput(from, args);
    });
    net.On(Command::ChooseCharacter, [this](const Endpoint& from, const ArgsList& args) {
        HandleChooseCharacter(from, args);
    });
    net.On(Command::ChooseLevel, [this](const Endpoint& from, const ArgsList& args) {
        HandleChooseLevel(from, args);
    });
    net.SetFallback([this](const Endpoint& from, Command command, const ArgsList& args) {
        DebugFallback(from, command, args);
    });
    net.SetServerIdentity(SecurityConfig::serverPrivateKeyPath());
    net.Bind(0);
    _port = net.LocalPort();
}

Room::~Room() {
    Stop();
}

void Room::Start() {
    if (_thread.joinable())
        return;
    _running = true;
    _thread = std::thread(&Room::Run, this);
}

void Room::Stop() {
    _running = false;
    if (_thread.joinable())
        _thread.join();
}

void Room::Run() {
    while (APP_RUN && _running) {
        if (_clock.Tick()) {
            ProcessPendingDisconnections();
            net.Poll();
            ApplyInputs();
            _systems.run(registre, 1.f / TICK_RATE);
            SyncEntities();
            if (net.ClientCount() > 0)
                _used = true;
        }
    }
}

bool Room::ShouldClose() const {
    if (net.ClientCount() > 0)
        return false;
    if (_used && net.AnnouncedCount() == 0)
        return true;
    return std::chrono::steady_clock::now() - _lastAnnounce > ROOM_IDLE_TIMEOUT;
}

std::size_t Room::FreeSlots() const {
    const std::size_t max = net.GetMaxClients();
    const std::size_t count = net.ClientCount();
    return count >= max ? 0 : max - count;
}

bool Room::HasClient(const Endpoint& client) const {
    return net.HasClient(client);
}

void Room::Announce(const Endpoint& client) {
    _lastAnnounce = std::chrono::steady_clock::now();
    net.Announce(client);
}

void Room::RemoveClient(const Endpoint& client) {
    std::lock_guard<std::mutex> lock(_disconnectionMutex);
    _pendingDisconnections.push_back(client);
}

}
