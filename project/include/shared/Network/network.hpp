#ifndef NETWORK_HPP
    #define NETWORK_HPP

    #define DEFAULT_PORT 6767

    #include <array>
    #include <cstddef>
    #include <cstdint>
    #include <functional>
    #include <map>
    #include <mutex>
    #include <optional>
    #include <string>
    #include <vector>
    #include <iostream>
    #include <chrono>

    #include <asio/io_context.hpp>
    #include <asio/ip/udp.hpp>
    #include <asio/post.hpp>

    #include "Error.hpp"
    #include "channel.hpp"
    #include "packet.hpp"
    #include "protocol.hpp"
    #include <algorithm>
    #include <span>
    #include <utility>

    #include "Encryption/ServerIdentity.hpp"
    #include "Encryption/SecureSession.hpp"
    #include "Network/SecureHandshake.hpp"
    #include "Network/SecureSessionManager.hpp"
    #include <memory>

namespace r_type {

using Endpoint = asio::ip::udp::endpoint;
using Handler = std::function<void(const Endpoint& from, const ArgsList& args)>;
using FallbackHandler = std::function<void(const Endpoint& from, Command command, const ArgsList& args)>;

class UdpSocket {
    Role _role;
    asio::io_context _context;
    asio::ip::udp::socket _socket;
    Endpoint _sender;
    std::optional<Endpoint> _host;
    std::optional<Endpoint> _server;
    bool _connected = false;
    mutable std::mutex _mutex;
    std::vector<Endpoint> _clients;
    std::vector<Endpoint> _announced;
    std::size_t _maxClients;
    bool _room;
    std::array<std::uint8_t, MAX_DATAGRAM_SIZE + 1> _buffer{};
    std::map<Command, std::vector<Handler>> _handlers;
    FallbackHandler _fallback;
    std::map<Endpoint, Channel> _channels;
    std::uint32_t _nextMessageId = 1;

    std::unique_ptr<ServerIdentity> _identity;
    std::optional<ServerIdentity::PublicKey> _trustedServerKey;
    std::map<Endpoint, SecureSession> _pendingSessions;
    std::map<Endpoint, std::chrono::steady_clock::time_point> _pendingSince;
    SecureSessionManager _secureSessions;

    void Open(const Endpoint& local);
    void StartReceive();
    void StartHandshake(const Endpoint& server);
    void HandleHandshake(const Endpoint& sender,
        std::span<const std::uint8_t> datagram);
    void HandleHandshakeRequest(const Endpoint& sender,
        std::span<const std::uint8_t> datagram);
    void HandleHandshakeResponse(const Endpoint& sender,
        std::span<const std::uint8_t> datagram);
    void HandleHandshakeConfirmation(const Endpoint& sender,
        std::span<const std::uint8_t> datagram);
    void OnReceive(const asio::error_code& ec, std::size_t size);
    void Handle(const Endpoint& sender, const DecodeResult& packet);
    void Deliver(const Endpoint& sender, Command command, const ArgsList& args);
    bool IsRepeat(const Endpoint& sender, std::uint32_t messageId);
    bool IsPeer(const Endpoint& sender) const;
    void OnAck(const Endpoint& sender, const ArgsList& acks);
    void OnResend(const Endpoint& sender, const ArgsList& requests);
    asio::error_code Transmit(const Endpoint& to, const Datagram& datagram);
    void Flush();
    bool HandleHost(const Endpoint& sender, Command command);
    bool HandleClient(const Endpoint& sender, Command command);
    bool AcceptClient(const Endpoint& sender);
    bool Drop(const Endpoint& client);
    bool Dispatch(const Endpoint& sender, Command command, const ArgsList& args) const;
    void Reply(const Endpoint& to, Command command, const ArgsList& args);
    void ReplyError(const Endpoint& to, NetError error, std::uint8_t rejectedCommand);

    void SendHandshakeReady(const Endpoint& client);

    void ClearExpiredHandshakes();

public:
    explicit UdpSocket(Role role, std::size_t maxClients = 0, bool room = false);
    ~UdpSocket();

    UdpSocket(const UdpSocket&) = delete;
    UdpSocket& operator=(const UdpSocket&) = delete;

    void Bind(std::uint16_t port = DEFAULT_PORT);
    void Connect(const std::string& ip, std::uint16_t port = DEFAULT_PORT);
    void Disconnect();
    void Disconnect(const Endpoint& client);
    bool IsConnected() const {return _connected;};
    std::uint16_t ServerPort() const;

    std::vector<Endpoint> Clients() const;
    std::size_t ClientCount() const;
    std::size_t AnnouncedCount() const;
    bool HasClient(const Endpoint& client) const;
    void Announce(const Endpoint& client);
    void RemoveClient(const Endpoint& client);
    std::size_t GetMaxClients() const;
    void SetMaxClients(std::size_t maxClients);

    void SendTo(const Endpoint& to, Command command, const ArgsList& args);
    void Send(Command command, const ArgsList& args);

    void On(Command command, Handler handler);
    void SetFallback(FallbackHandler fallback);
    std::size_t Poll();

    std::uint16_t LocalPort() const;
    bool IsOpen() const {return _socket.is_open();};
    void Close();

    void SetServerIdentity(const std::string& privateKeyPath);
    void SetTrustedServerKey(const ServerIdentity::PublicKey& publicKey);

    bool HasSecureSession(const Endpoint& endpoint) const;
};

}

#endif
