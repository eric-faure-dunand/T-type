#include "core.hpp"

namespace r_type {

Core::Core(std::uint16_t port): _socket(Role::Server, DEFAULT_SERVER_CLIENTS) {
    _socket.On(Command::CreateRoom, [this](const Endpoint& from, const ArgsList&) {
        CreateRoom(from);
    });
    _socket.On(Command::JoinRoom, [this](const Endpoint& from, const ArgsList& args) {
        JoinRoom(from, std::get<std::uint32_t>(args[0][0]));
    });
    _socket.On(Command::ListRooms, [this](const Endpoint& from, const ArgsList&) {
        Reply(from, Command::RoomList, ListRooms());
    });
    _socket.On(Command::Disconnect, [this](const Endpoint& from, const ArgsList&) {
        LeaveRooms(from);
    });
    _socket.Bind(port);
}

void Core::Run() {
    while (SERVER_RUN) {
        if (_clock.Tick()) {
            _socket.Poll();
            CloseRooms();
        }
    }
}

void Core::CreateRoom(const Endpoint& client) {
    if (InRoom(client)) {
        Refuse(client, NetError::AlreadyInRoom, Command::CreateRoom);
        return;
    }
    if (_rooms.size() >= _maxRooms) {
        Refuse(client, NetError::TooManyRooms, Command::CreateRoom);
        return;
    }
    std::unique_ptr<Room> room;
    try {
        room = std::make_unique<Room>(_nextRoomId);
    } catch (const Error&) {
        Refuse(client, NetError::TooManyRooms, Command::CreateRoom);
        return;
    }
    _nextRoomId++;
    LeaveRooms(client);
    room->Announce(client);
    room->Start();
    Reply(client, Command::RoomJoined, {{static_cast<std::uint32_t>(room->Port())}});
    _rooms.push_back(std::move(room));
}

void Core::JoinRoom(const Endpoint& client, std::uint32_t roomId) {
    const auto it = std::find_if(_rooms.begin(), _rooms.end(),
        [roomId](const std::unique_ptr<Room>& room) {return room->Id() == roomId;});
    if (it == _rooms.end()) {
        Refuse(client, NetError::RoomNotFound, Command::JoinRoom);
        return;
    }
    if (InRoom(client)) {
        Refuse(client, NetError::AlreadyInRoom, Command::JoinRoom);
        return;
    }
    LeaveRooms(client);
    (*it)->Announce(client);
    Reply(client, Command::RoomJoined, {{static_cast<std::uint32_t>((*it)->Port())}});
}

ArgsList Core::ListRooms() const {
    ArgsList list;
    for (const std::unique_ptr<Room>& room : _rooms) {
        const std::size_t free = room->FreeSlots();
        if (free > 0)
            list.push_back({room->Id(), static_cast<std::uint8_t>(std::min<std::size_t>(free, 0xFF))});
    }
    return list;
}

bool Core::InRoom(const Endpoint& client) const {
    return std::any_of(_rooms.begin(), _rooms.end(),
        [&client](const std::unique_ptr<Room>& room) {return room->HasClient(client);});
}

void Core::LeaveRooms(const Endpoint& client) {
    for (const std::unique_ptr<Room>& room : _rooms)
        room->RemoveClient(client);
}

void Core::CloseRooms() {
    std::erase_if(_rooms, [](const std::unique_ptr<Room>& room) {return room->ShouldClose();});
}

void Core::Reply(const Endpoint& client, Command command, const ArgsList& args) {
    try {
        _socket.SendTo(client, command, args);
    } catch (const IError&) {
        return;
    }
}

void Core::Refuse(const Endpoint& client, NetError error, Command command) {
    Reply(client, Command::Error, {{static_cast<std::uint8_t>(error), static_cast<std::uint8_t>(command)}});
}

}
