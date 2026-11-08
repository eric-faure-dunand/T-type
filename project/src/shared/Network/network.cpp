#include "network.hpp"

namespace r_type {

static std::string Reason(const asio::error_code& ec) {
    return " (" + ec.message() + ")";
}

static std::uint8_t Code(Command command) {
    return static_cast<std::uint8_t>(command);
}

static TimePoint Now() {
    return std::chrono::steady_clock::now();
}

UdpSocket::UdpSocket(Role role, std::size_t maxClients, bool room):
    _role(role), _socket(_context), _maxClients(maxClients), _room(room) {}

UdpSocket::~UdpSocket() {
    if (!_socket.is_open())
        return;
    if (_role == Role::Client) {
        if (_server)
            Reply(*_server, Command::Disconnect, {{}});
        return;
    }
    std::lock_guard<std::mutex> lock(_mutex);
    for (const Endpoint& client : _clients)
        Reply(client, Command::Disconnect, {{}});
}

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
    if (!_socket.is_open()) {
        Open(Endpoint(asio::ip::udp::v4(), 0));
        _server = Endpoint(address, port);
    }
    _host = Endpoint(address, port);
    _connected = false;
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
    if (!Drop(client))
        return;
    Dispatch(client, Command::Disconnect, {{}});
    SendTo(client, Command::Disconnect, {{}});
}

std::uint16_t UdpSocket::ServerPort() const {
    if (!_server)
        throw Error("UdpSocket: no server, call Connect first.");
    return _server->port();
}

std::vector<Endpoint> UdpSocket::Clients() const {
    std::lock_guard<std::mutex> lock(_mutex);
    return _clients;
}

std::size_t UdpSocket::ClientCount() const {
    std::lock_guard<std::mutex> lock(_mutex);
    return _clients.size();
}

std::size_t UdpSocket::AnnouncedCount() const {
    std::lock_guard<std::mutex> lock(_mutex);
    return _announced.size();
}

bool UdpSocket::HasClient(const Endpoint& client) const {
    std::lock_guard<std::mutex> lock(_mutex);
    return std::find(_clients.begin(), _clients.end(), client) != _clients.end();
}

void UdpSocket::Announce(const Endpoint& client) {
    std::lock_guard<std::mutex> lock(_mutex);
    if (std::find(_announced.begin(), _announced.end(), client) == _announced.end())
        _announced.push_back(client);
}

void UdpSocket::RemoveClient(const Endpoint& client) {
    {
        std::lock_guard<std::mutex> lock(_mutex);
        std::erase(_announced, client);
    }
    asio::post(_context, [this, client]() {
        if (Drop(client))
            Dispatch(client, Command::Disconnect, {{}});
    });
}

std::size_t UdpSocket::GetMaxClients() const {
    std::lock_guard<std::mutex> lock(_mutex);
    return _maxClients;
}

void UdpSocket::SetMaxClients(std::size_t maxClients) {
    std::lock_guard<std::mutex> lock(_mutex);
    _maxClients = maxClients;
}

void UdpSocket::SendTo(const Endpoint& to, Command command, const ArgsList& args) {
    if (!_socket.is_open())
        throw Error("UdpSocket: socket is not open.");
    const std::uint32_t messageId = _nextMessageId++;
    std::vector<Datagram> datagrams = EncodeMessage(command, args, _role, messageId);
    for (const Datagram& datagram : datagrams) {
        const asio::error_code ec = Transmit(to, datagram);
        if (ec)
            throw Error("UdpSocket: cannot send data" + Reason(ec) + ".");
    }
    if (FindCommand(Code(command))->reliable)
        _channels[to].Track(messageId, std::move(datagrams), Now());
}

void UdpSocket::Send(Command command, const ArgsList& args) {
    if (!_host)
        throw Error("UdpSocket: no host to send to, call Connect first.");
    SendTo(*_host, command, args);
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
    const std::size_t handled = _context.poll();
    Flush();
    return handled;
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
    _host.reset();
    _server.reset();
    _connected = false;
    std::lock_guard<std::mutex> lock(_mutex);
    _clients.clear();
    _announced.clear();
    _channels.clear();
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
        Handle(sender, result);
    else
        ReplyError(sender, result.error, result.rejectedCommand);
}

void UdpSocket::Handle(const Endpoint& sender, const DecodeResult& packet) {
    if (packet.command == Command::Ack) {
        OnAck(sender, packet.args);
        return;
    }
    if (packet.command == Command::Resend) {
        OnResend(sender, packet.args);
        return;
    }
    const bool reliable = FindCommand(Code(packet.command))->reliable;
    if (reliable && IsRepeat(sender, packet.messageId))
        return;
    const bool accepted = _role == Role::Server ? HandleHost(sender, packet.command) : HandleClient(sender, packet.command);
    if (!accepted) {
        if (reliable)
            Reply(sender, Command::Ack, {{packet.messageId}});
        return;
    }
    if (!reliable) {
        Deliver(sender, packet.command, packet.args);
        return;
    }
    if (!IsPeer(sender)) {
        Reply(sender, Command::Ack, {{packet.messageId}});
        Deliver(sender, packet.command, packet.args);
        return;
    }
    const std::optional<ArgsList> args = _channels[sender].Receive(packet, reliable, Now());
    if (args)
        Deliver(sender, packet.command, *args);
}

void UdpSocket::Deliver(const Endpoint& sender, Command command, const ArgsList& args) {
    if (Dispatch(sender, command, args))
        return;
    if (command == Command::ConnectRequest || command == Command::ConnectAccept || command == Command::Disconnect)
        return;
    if (_fallback)
        _fallback(sender, command, args);
    ReplyError(sender, NetError::CommandNotHandled, Code(command));
}

bool UdpSocket::IsRepeat(const Endpoint& sender, std::uint32_t messageId) {
    const auto channel = _channels.find(sender);
    return channel != _channels.end() && channel->second.Repeat(messageId);
}

bool UdpSocket::IsPeer(const Endpoint& sender) const {
    if (_role == Role::Server)
        return HasClient(sender);
    return _host && sender == *_host;
}

void UdpSocket::OnAck(const Endpoint& sender, const ArgsList& acks) {
    const auto channel = _channels.find(sender);
    if (channel == _channels.end())
        return;
    for (const Args& ack : acks)
        channel->second.Acknowledge(std::get<std::uint32_t>(ack[0]));
}

void UdpSocket::OnResend(const Endpoint& sender, const ArgsList& requests) {
    const auto channel = _channels.find(sender);
    if (channel == _channels.end())
        return;
    for (const Args& request : requests) {
        const Datagram *datagram = channel->second.FindFragment(
            std::get<std::uint32_t>(request[0]), std::get<std::uint32_t>(request[1]));
        if (datagram)
            Transmit(sender, *datagram);
    }
}

asio::error_code UdpSocket::Transmit(const Endpoint& to, const Datagram& datagram) {
    asio::error_code ec;
    _socket.send_to(asio::buffer(datagram), to, 0, ec);
    return ec;
}

void UdpSocket::Flush() {
    if (!_socket.is_open())
        return;
    const TimePoint now = Now();
    for (auto it = _channels.begin(); it != _channels.end();) {
        const Endpoint peer = it->first;
        const Channel::Output output = it->second.Update(now);
        for (const Datagram& datagram : output.resend)
            Transmit(peer, datagram);
        if (!output.acks.empty())
            Reply(peer, Command::Ack, output.acks);
        if (!output.requests.empty())
            Reply(peer, Command::Resend, output.requests);
        it = it->second.Idle() ? _channels.erase(it) : std::next(it);
    }
}

bool UdpSocket::HandleHost(const Endpoint& sender, Command command) {
    if (command == Command::ConnectRequest)
        return AcceptClient(sender);
    if (!HasClient(sender)) {
        if (command != Command::Disconnect)
            ReplyError(sender, NetError::NotConnected, Code(command));
        return false;
    }
    if (command == Command::Disconnect) {
        if (_room)
            return false;
        Drop(sender);
    }
    return true;
}

bool UdpSocket::Drop(const Endpoint& client) {
    const auto channel = _channels.find(client);
    if (channel != _channels.end()) {
        const ArgsList acks = channel->second.TakeAcks();
        if (!acks.empty())
            Reply(client, Command::Ack, acks);
        _channels.erase(channel);
    }
    std::lock_guard<std::mutex> lock(_mutex);
    return std::erase(_clients, client) > 0;
}

bool UdpSocket::HandleClient(const Endpoint& sender, Command command) {
    if (!_host || sender != *_host)
        return false;
    if (command == Command::ConnectAccept)
        _connected = true;
    else if (command == Command::Disconnect)
        _connected = false;
    return true;
}

bool UdpSocket::AcceptClient(const Endpoint& sender) {
    std::optional<NetError> refusal;
    {
        std::lock_guard<std::mutex> lock(_mutex);
        if (std::find(_clients.begin(), _clients.end(), sender) == _clients.end()) {
            if (_room && std::erase(_announced, sender) == 0)
                refusal = NetError::NotConnected;
            else if (_clients.size() >= _maxClients)
                refusal = NetError::Full;
            else
                _clients.push_back(sender);
        }
    }
    if (refusal) {
        ReplyError(sender, *refusal, Code(Command::ConnectRequest));
        return false;
    }
    Reply(sender, Command::ConnectAccept, {{}});
    return true;
}

bool UdpSocket::Dispatch(const Endpoint& sender, Command command, const ArgsList& args) const {
    const auto it = _handlers.find(command);
    if (it == _handlers.end() || it->second.empty())
        return false;
    for (const Handler& handler : it->second)
        handler(sender, args);
    return true;
}

void UdpSocket::Reply(const Endpoint& to, Command command, const ArgsList& args) {
    try {
        SendTo(to, command, args);
    } catch (const Error&) {
        return;
    }
}

void UdpSocket::ReplyError(const Endpoint& to, NetError error, std::uint8_t rejectedCommand) {
    if (error == NetError::DatagramTooShort || error == NetError::BadMagic
        || rejectedCommand == Code(Command::Error))
        return;
    Reply(to, Command::Error, {{static_cast<std::uint8_t>(error), rejectedCommand}});
}

}
