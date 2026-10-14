#include <algorithm>
#include <span>
#include <utility>

#include "network.hpp"

namespace r_type {

static std::string Reason(const asio::error_code& ec) {
    return " (" + ec.message() + ")";
}

UdpSocket::UdpSocket(Role role): _role(role), _socket(_context) {}

void UdpSocket::Open(const Endpoint& local) {
    Close();
    asio::error_code ec;
    _socket.open(local.protocol(), ec);
    if (ec)
        throw Error("UdpSocket: cannot create socket" + Reason(ec) + ".");
    _socket.bind(local, ec);
    if (ec) {
        Close();
        throw Error("UdpSocket: cannot bind port " + std::to_string(local.port()) + Reason(ec) + ".");
    }
    StartReceive();
}

void UdpSocket::Bind(std::uint16_t port) {
    Open(Endpoint(asio::ip::udp::v4(), port));
}

void UdpSocket::Connect(const std::string& ip, std::uint16_t port) {
    if (_role != Role::Client)
        throw Error("UdpSocket: only a client can connect to a server.");
    asio::error_code ec;
    const asio::ip::address address = asio::ip::make_address(ip, ec);
    if (ec || !address.is_v4())
        throw Error("UdpSocket: invalid ip address '" + ip + "'.");
    Open(Endpoint(asio::ip::udp::v4(), 0));
    _server = Endpoint(address, port);
    Send(Command::ConnectRequest, {{}});
}

void UdpSocket::Disconnect() {
    if (_role != Role::Client)
        throw Error("UdpSocket: only a client can disconnect from its server.");
    _connected = false;
    Send(Command::Disconnect, {{}});
}

void UdpSocket::Disconnect(const Endpoint& client) {
    if (_role != Role::Server)
        throw Error("UdpSocket: only a server can disconnect a client.");
    if (std::erase(_clients, client) == 0)
        return;
    SendTo(client, Command::Disconnect, {{}});
}

void UdpSocket::SendTo(const Endpoint& to, Command command, const ArgsList& args) {
    if (!_socket.is_open())
        throw Error("UdpSocket: socket is not open.");
    const std::vector<std::uint8_t> datagram = EncodePacket(command, args, _role);
    asio::error_code ec;
    _socket.send_to(asio::buffer(datagram), to, 0, ec);
    if (ec)
        throw Error("UdpSocket: cannot send data" + Reason(ec) + ".");
}

void UdpSocket::Send(Command command, const ArgsList& args) {
    if (!_server)
        throw Error("UdpSocket: no server to send to, call Connect first.");
    SendTo(*_server, command, args);
}

void UdpSocket::On(Command command, Handler handler) {
    _handlers[command].push_back(std::move(handler));
}

void UdpSocket::SetFallback(FallbackHandler fallback) {
    _fallback = std::move(fallback);
}

std::size_t UdpSocket::Poll() {
    if (_context.stopped())
        _context.restart();
    return _context.poll();
}

std::uint16_t UdpSocket::LocalPort() const {
    asio::error_code ec;
    const Endpoint local = _socket.local_endpoint(ec);
    if (ec)
        throw Error("UdpSocket: cannot query local port" + Reason(ec) + ".");
    return local.port();
}

void UdpSocket::Close() {
    asio::error_code ec;
    _socket.close(ec);
    _server.reset();
    _connected = false;
    _clients.clear();
}

void UdpSocket::StartReceive() {
    _socket.async_receive_from(asio::buffer(_buffer), _sender,
        [this](const asio::error_code& ec, std::size_t size) {OnReceive(ec, size);});
}

void UdpSocket::OnReceive(const asio::error_code& ec, std::size_t size) {
    if (ec == asio::error::operation_aborted || !_socket.is_open())
        return;
    if (ec && ec != asio::error::message_size) {
        StartReceive();
        return;
    }
    if (ec == asio::error::message_size)
        size = _buffer.size();

    const DecodeResult result = DecodePacket(std::span<const std::uint8_t>(_buffer.data(), size), _role);
    const Endpoint sender = _sender;
    StartReceive();

    if (result.ok)
        Handle(sender, result.command, result.args);
    else
        ReplyError(sender, result.error, result.rejectedCommand);
}

void UdpSocket::Handle(const Endpoint& sender, Command command, const ArgsList& args) {
    if (_role == Role::Server) {
        const bool known = std::find(_clients.begin(), _clients.end(), sender) != _clients.end();
        if (command == Command::ConnectRequest) {
            if (!known)
                _clients.push_back(sender);
            Reply(sender, Command::ConnectAccept, {{}});
        } else if (!known) {
            if (command != Command::Disconnect)
                ReplyError(sender, NetError::NotConnected, static_cast<std::uint8_t>(command));
            return;
        } else if (command == Command::Disconnect)
            std::erase(_clients, sender);
    } else if (command == Command::ConnectAccept || command == Command::Disconnect) {
        if (!_server || sender != *_server)
            return;
        _connected = command == Command::ConnectAccept;
    }
    Dispatch(command, args);
}

void UdpSocket::Dispatch(Command command, const ArgsList& args) const {
    const auto it = _handlers.find(command);
    if (it != _handlers.end() && !it->second.empty()) {
        for (const Handler& handler : it->second)
            handler(args);
        return;
    }
    if (_fallback)
        _fallback(command, args);
}

void UdpSocket::Reply(const Endpoint& to, Command command, const ArgsList& args) {
    const std::vector<std::uint8_t> datagram = EncodePacket(command, args, _role);
    asio::error_code ec;
    _socket.send_to(asio::buffer(datagram), to, 0, ec);
}

void UdpSocket::ReplyError(const Endpoint& to, NetError error, std::uint8_t rejectedCommand) {
    if (error == NetError::DatagramTooShort || error == NetError::BadMagic
        || rejectedCommand == static_cast<std::uint8_t>(Command::Error))
        return;
    Reply(to, Command::Error, {{static_cast<std::uint8_t>(error), rejectedCommand}});
}

}
