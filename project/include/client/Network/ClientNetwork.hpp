/*
** EPITECH PROJECT, 2026
** T-type
** File description:
** ClientNetwork.hpp
*/

#ifndef CLIENT_NETWORK_HPP
    #define CLIENT_NETWORK_HPP

    #include <atomic>
    #include <chrono>
    #include <cstdint>
    #include <deque>
    #include <map>
    #include <mutex>
    #include <optional>
    #include <string>
    #include <thread>
    #include <unordered_set>
    #include <vector>

    #include "clock.hpp"
    #include "network.hpp"
    #include "ClientEvents.hpp"

namespace r_type::client {

constexpr std::chrono::seconds CONNECT_TIMEOUT{3};
constexpr std::chrono::milliseconds INPUT_RESEND_DELAY{100};
constexpr std::size_t MAX_TRANSFER_ENTITIES = 4096;

class ClientNetwork {
    enum class Stage {
        Offline,
        ConnectingServer,
        Lobby,
        JoiningRoom,
        InRoom,
        ReturningToLobby
    };

    struct Transfer {
        std::uint32_t expected = 0;
        std::map<std::uint32_t, EntityData> entities;
    };

    UdpSocket _socket;
    Clock _clock;
    std::thread _thread;
    std::atomic_bool _running = false;
    std::mutex _mutex;
    std::deque<NetRequest> _requests;
    std::vector<NetEvent> _events;

    Stage _stage = Stage::Offline;
    std::chrono::steady_clock::time_point _stageSince;
    std::string _ip;
    std::uint16_t _roomPort = 0;
    std::optional<std::uint32_t> _player;
    std::uint8_t _held = 0;
    std::chrono::steady_clock::time_point _lastInputSent;
    std::unordered_set<std::uint32_t> _destroyed;
    std::optional<Transfer> _transfer;

    void Run();
    std::deque<NetRequest> TakeRequests();
    void Process(const NetRequest& request);
    void Emit(NetEvent event);
    void SetStage(Stage stage);
    bool Send(Command command, const ArgsList& args);
    void SendInput(std::uint8_t flags, InputState state);
    void SetHeld(std::uint8_t held);
    void ResendInputs();
    void CheckTimeouts();
    void ConnectServer(const std::string& ip);
    void ReturnToLobby();
    void ResetRoom();
    void GoOffline(const std::string& reason, bool whileConnecting);
    void SayGoodbye();

    void RegisterHandlers();
    void OnConnectAccept();
    void OnDisconnect();
    void OnRoomList(const ArgsList& args);
    void OnRoomJoined(const ArgsList& args);
    void OnError(const ArgsList& args);
    void OnPlayerCreation(const ArgsList& args);
    void OnInitialStateBegin(const ArgsList& args);
    void OnInitialStateEnd(const ArgsList& args);
    void OnEntitySpawn(const ArgsList& args);
    void OnEntityUpdate(const ArgsList& args);
    void OnEntityDestroy(const ArgsList& args);

public:
    ClientNetwork();
    ~ClientNetwork();

    ClientNetwork(const ClientNetwork&) = delete;
    ClientNetwork& operator=(const ClientNetwork&) = delete;

    void Start();
    void Stop();
    void Push(NetRequest request);
    std::vector<NetEvent> ConsumeEvents();
};

}

#endif
