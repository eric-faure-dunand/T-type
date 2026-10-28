#ifndef ROOM_HPP
    #define ROOM_HPP

    #include <atomic>
    #include <chrono>
    #include <cstddef>
    #include <cstdint>
    #include <thread>
    #include <iostream>
    #include <deque>
    #include <mutex>
    #include <unordered_map>

    #include "sig.hpp"
    #include "clock.hpp"
    #include "network.hpp"
    #include "Registry.hpp"
    #include "Components/PlayerComponents.hpp"
    #include "Components/ProjectileComponents.hpp"

namespace r_type {

constexpr std::size_t DEFAULT_ROOM_CLIENTS = 4;
constexpr std::size_t MAX_PENDING_INPUTS = 256;
constexpr std::chrono::minutes ROOM_IDLE_TIMEOUT{1};

class Room {

    struct PlayerInputEvent {
        Endpoint client;
        std::uint32_t entityId;
        std::uint8_t inputFlags;
        InputState state;
    };

    std::uint32_t _id;
    UdpSocket net;
    std::uint16_t _port = 0;
    std::chrono::steady_clock::time_point _lastAnnounce;
    std::atomic_bool _used = false;
    std::atomic_bool _running = false;
    Clock _clock;
    std::thread _thread;
    std::deque<PlayerInputEvent> _pendingInputs;
    std::mutex _disconnectionMutex;
    std::deque<Endpoint> _pendingDisconnections;

    ecs::Registry registre;

    std::map<Endpoint, ecs::Entity> _players;

    std::unordered_map<std::uint32_t, std::chrono::steady_clock::time_point> _lastShotTime;

    void Run();

    void DebugFallback(const Endpoint& from, Command command, const ArgsList& args);
    void HandlePlayerInput(const Endpoint& from, const ArgsList& args);
    void HandlePlayerShoot(ecs::Entity playerEntity);
    void HandlePlayerConnection(const Endpoint& from);
    void SendInitialState(const Endpoint& client);
    void BroadcastPlayerSpawn(const Endpoint& newClient, ecs::Entity entity);
    void HandlePlayerDisconnection(const Endpoint& from);

    void UpdatePlayers();
    void UpdateProjectiles();
    void DestroyOutOfBoundsProjectiles();
    void BroadcastPlayerUpdates();
    void BroadcastProjectileUpdates();

    void ProcessPendingDisconnections();

public:
    explicit Room(std::uint32_t id);
    ~Room();

    Room(const Room&) = delete;
    Room& operator=(const Room&) = delete;

    void Start();
    void Stop();
    bool ShouldClose() const;

    std::uint32_t Id() const {return _id;};
    std::uint16_t Port() const {return _port;};
    std::size_t FreeSlots() const;
    bool HasClient(const Endpoint& client) const;
    void Announce(const Endpoint& client);
    void RemoveClient(const Endpoint& client);

    std::size_t GetMaxClients() const {return net.GetMaxClients();};
    void SetMaxClients(std::size_t maxClients) {net.SetMaxClients(maxClients);};

    std::deque<PlayerInputEvent> ConsumePlayerInputs();
};

}

#endif
