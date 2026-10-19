/*
** EPITECH PROJECT, 2026
** T-type
** File description:
** ClientNetwork.cpp
*/

#include "ClientNetwork.hpp"

namespace r_type::client {

ClientNetwork::ClientNetwork(): _socket(Role::Client) {
    RegisterHandlers();
}

ClientNetwork::~ClientNetwork() {
    Stop();
}

void ClientNetwork::Start() {
    if (_thread.joinable())
        return;
    _running = true;
    _thread = std::thread(&ClientNetwork::Run, this);
}

void ClientNetwork::Stop() {
    if (!_thread.joinable())
        return;
    _running = false;
    _thread.join();
    SayGoodbye();
    _socket.Close();
}

void ClientNetwork::Push(NetRequest request) {
    std::lock_guard<std::mutex> lock(_mutex);
    _requests.push_back(std::move(request));
}

std::vector<NetEvent> ClientNetwork::ConsumeEvents() {
    std::vector<NetEvent> events;
    std::lock_guard<std::mutex> lock(_mutex);
    events.swap(_events);
    return events;
}

void ClientNetwork::Run() {
    while (_running) {
        if (!_clock.Tick()) {
            std::this_thread::sleep_for(std::chrono::microseconds(500));
            continue;
        }
        for (const NetRequest& request : TakeRequests())
            Process(request);
        if (_socket.IsOpen())
            _socket.Poll();
        CheckTimeouts();
        ResendInputs();
    }
}

std::deque<NetRequest> ClientNetwork::TakeRequests() {
    std::deque<NetRequest> requests;
    std::lock_guard<std::mutex> lock(_mutex);
    requests.swap(_requests);
    return requests;
}

void ClientNetwork::Process(const NetRequest& request) {
    std::visit(Overloaded{
        [this](const request::Connect& r) {
            if (_stage == Stage::Offline)
                ConnectServer(r.ip);
        },
        [this](const request::ListRooms&) {
            if (_stage == Stage::Lobby)
                Send(Command::ListRooms, {{}});
        },
        [this](const request::CreateRoom&) {
            if (_stage == Stage::Lobby)
                Send(Command::CreateRoom, {{}});
        },
        [this](const request::JoinRoom& r) {
            if (_stage == Stage::Lobby)
                Send(Command::JoinRoom, {{r.id}});
        },
        [this](const request::Input& r) {
            SetHeld(r.held);
        },
        [this](const request::LeaveRoom&) {
            if (_stage != Stage::InRoom && _stage != Stage::JoiningRoom)
                return;
            Send(Command::DisconnectRoom, {{_player.value_or(0)}});
            ReturnToLobby();
        },
        [this](const request::Disconnect&) {
            SayGoodbye();
            ResetRoom();
            _socket.Close();
            SetStage(Stage::Offline);
        },
    }, request);
}

void ClientNetwork::Emit(NetEvent event) {
    std::lock_guard<std::mutex> lock(_mutex);
    _events.push_back(std::move(event));
}

void ClientNetwork::SetStage(Stage stage) {
    _stage = stage;
    _stageSince = std::chrono::steady_clock::now();
}

bool ClientNetwork::Send(Command command, const ArgsList& args) {
    try {
        _socket.Send(command, args);
    } catch (const IError&) {
        return false;
    }
    return true;
}

void ClientNetwork::SendInput(std::uint8_t flags, InputState state) {
    if (!_player || flags == 0)
        return;
    Send(Command::PlayerInput, {{*_player, flags, static_cast<std::uint8_t>(state)}});
}

void ClientNetwork::SetHeld(std::uint8_t held) {
    held &= INPUT_MASK;
    const std::uint8_t pressed = held & ~_held;
    const std::uint8_t released = _held & ~held;
    _held = held;
    if (_stage != Stage::InRoom)
        return;
    SendInput(pressed, InputState::Pressed);
    SendInput(released, InputState::Released);
}

void ClientNetwork::ResendInputs() {
    const auto now = std::chrono::steady_clock::now();
    if (_stage != Stage::InRoom || !_player || now - _lastInputSent < INPUT_RESEND_DELAY)
        return;
    _lastInputSent = now;
    SendInput(_held, InputState::Pressed);
    SendInput(static_cast<std::uint8_t>(~_held & INPUT_MASK), InputState::Released);
}

void ClientNetwork::CheckTimeouts() {
    if (std::chrono::steady_clock::now() - _stageSince < CONNECT_TIMEOUT)
        return;
    if (_stage == Stage::ConnectingServer) {
        GoOffline("The server does not answer", true);
    } else if (_stage == Stage::JoiningRoom) {
        Emit(event::Notice{"The room does not answer"});
        ReturnToLobby();
    } else if (_stage == Stage::ReturningToLobby) {
        GoOffline("Connection to the server lost", false);
    }
}

void ClientNetwork::ConnectServer(const std::string& ip) {
    try {
        _socket.Close();
        _ip = ip;
        _socket.Connect(ip, DEFAULT_PORT);
        SetStage(Stage::ConnectingServer);
    } catch (const IError&) {
        _socket.Close();
        Emit(event::ConnectionFailed{"Invalid IP address: " + ip});
    }
}

void ClientNetwork::ReturnToLobby() {
    ResetRoom();
    try {
        _socket.Connect(_ip, _socket.ServerPort());
        SetStage(Stage::ReturningToLobby);
    } catch (const IError&) {
        GoOffline("Connection to the server lost", false);
    }
}

void ClientNetwork::ResetRoom() {
    _player.reset();
    _destroyed.clear();
    _transfer.reset();
    _roomPort = 0;
}

void ClientNetwork::GoOffline(const std::string& reason, bool whileConnecting) {
    ResetRoom();
    _socket.Close();
    SetStage(Stage::Offline);
    if (whileConnecting)
        Emit(event::ConnectionFailed{reason});
    else
        Emit(event::Disconnected{reason});
}

void ClientNetwork::SayGoodbye() {
    if (_stage == Stage::Offline || !_socket.IsOpen())
        return;
    if (_stage == Stage::InRoom)
        Send(Command::DisconnectRoom, {{_player.value_or(0)}});
    try {
        asio::error_code ec;
        const asio::ip::address address = asio::ip::make_address(_ip, ec);
        if (!ec)
            _socket.SendTo(Endpoint(address, _socket.ServerPort()), Command::Disconnect, {{}});
    } catch (const IError&) {
        return;
    }
}

}
