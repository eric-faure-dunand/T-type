#ifndef CORE_HPP
    #define CORE_HPP

    #include <algorithm>
    #include <cstddef>
    #include <cstdint>
    #include <memory>
    #include <vector>

    #include "sig.hpp"
    #include "clock.hpp"
    #include "network.hpp"
    #include "room.hpp"

namespace r_type {

constexpr std::size_t DEFAULT_SERVER_CLIENTS = 50;
constexpr std::size_t DEFAULT_MAX_ROOMS = 12;

class Core {
    UdpSocket _socket;
    std::vector<std::unique_ptr<Room>> _rooms;
    std::size_t _maxRooms = DEFAULT_MAX_ROOMS;
    std::uint32_t _nextRoomId = 1;
    Clock _clock;

    void CreateRoom(const Endpoint& client);
    void JoinRoom(const Endpoint& client, std::uint32_t roomId);
    ArgsList ListRooms() const;
    bool InRoom(const Endpoint& client) const;
    void LeaveRooms(const Endpoint& client);
    void CloseRooms();
    void Reply(const Endpoint& client, Command command, const ArgsList& args);
    void Refuse(const Endpoint& client, NetError error, Command command);

public:
    explicit Core(std::uint16_t port = DEFAULT_PORT);
    ~Core() = default;

    void Run();
};

}

#endif
