# R-Type — Network layer (developer guide)

This guide explains how to use the network code of the project: the shared UDP layer (`UdpSocket`), the server lobby (`Core`), the rooms (`Room`) and the tick `Clock`. The binary wire format, command codes and validation rules are specified in the RFC: [Readme](RFC/Readme.md) and [Protocol](RFC/Protocol.md).

## Files

| Path | Content |
|---|---|
| `project/include/shared/Network/protocol.hpp` | Constants, `Command`, `NetError`, `Role`, command registry (direction and argument schema of each command) |
| `project/include/shared/Network/packet.hpp` | `Arg`, `Args`, `ArgsList`, `EncodePacket`, `DecodePacket` |
| `project/include/shared/Network/network.hpp` | `UdpSocket`, `Endpoint`, `Handler`, `FallbackHandler` |
| `project/include/shared/clock.hpp` | `Clock`, `TICK_RATE` |
| `project/include/server/Core/core.hpp` | `Core`: server lobby, owns the rooms |
| `project/include/server/Core/Room/room.hpp` | `Room`: one game room, its own socket and thread |

Asio (standalone, 1.38.2) is fetched by CMake through `cmake/CPM.cmake`; nothing has to be installed.

## Packets and arguments

A packet is a command and an `ArgsList`, a list of records. Each record (`Args`) is the list of the command's arguments, in the order of its schema (see the registry in `protocol.cpp` and RFC §3):

```cpp
using Arg = std::variant<std::uint8_t, std::uint32_t, std::int32_t>;
using Args = std::vector<Arg>;
using ArgsList = std::vector<Args>;
```

- Every command carries exactly one record, except `ROOM_LIST`, which carries 0 to N records.
- A command without arguments is sent with `{{}}` (one empty record).
- The C++ type of each argument must match the schema: `42u` is a `uint32`, `-100` an `int32`, and a `uint8` must be written `std::uint8_t{1}`. A mismatch throws `Error`.

```cpp
net.Send(Command::PlayerInput, {{entityId, std::uint8_t{0x01}}});   // entity_id u32, input_flags u8
auto id = std::get<std::uint32_t>(args[0][0]);                      // first argument of the first record
```

## UdpSocket

### Roles

| Socket | Construction |
|---|---|
| Server (lobby) | `UdpSocket(Role::Server, 50)` |
| Room | `UdpSocket(Role::Server, 4, true)` |
| Client | `UdpSocket(Role::Client)` |

The second argument is the maximum number of connected clients, the third one makes the socket a room. The defaults live in `Core` (`DEFAULT_SERVER_CLIENTS = 50`) and `Room` (`DEFAULT_ROOM_CLIENTS = 4`). A room differs from the server in two ways only:

- it accepts `CONNECT_REQUEST` only from endpoints announced with `Announce()`;
- it ignores `DISCONNECT`: a client leaves a room with `DISCONNECT_ROOM`.

### Host side (server and rooms)

| Method | Use |
|---|---|
| `Bind(port)` | Open the socket; `Bind(0)` lets the system choose the port, read it with `LocalPort()` |
| `Clients()`, `ClientCount()`, `HasClient(endpoint)` | Connected clients |
| `GetMaxClients()`, `SetMaxClients(n)` | Limit; lowering it disconnects nobody, it only refuses newcomers with `ERROR FULL` |
| `Announce(endpoint)` | Room only: allow this endpoint to connect |
| `RemoveClient(endpoint)` | Remove a client without sending anything; callable from another thread (see below) |
| `Disconnect(endpoint)` | Kick a client: remove it and send it `DISCONNECT` |

### Client side

| Method | Use |
|---|---|
| `Connect(ip, port)` | Send `CONNECT_REQUEST` to a host. The first call opens the socket and remembers this host as the server. Later calls keep the same socket (a room only accepts the endpoint the server announced) and change the current host. |
| `IsConnected()` | `true` once the current host answered `CONNECT_ACCEPT` |
| `Send(command, args)` | Send to the current host |
| `Disconnect()` | Send `DISCONNECT` to the current host (only the server handles it) |
| `ServerPort()` | Port of the server given to the first `Connect`, to come back to the lobby |

The client only processes datagrams coming from its current host and ignores any other sender.

### Sending

`SendTo(endpoint, command, args)` and `Send(command, args)` check the command direction and the arguments against the schema, and throw `Error` on a mistake or a system failure. In a handler on the server side, prefer a send that cannot stop the server (see `Core::Reply`).

### Receiving: handlers

```cpp
net.On(Command::JoinRoom, [&](const Endpoint& from, const ArgsList& args) {
    net.SendTo(from, Command::RoomJoined, {{port}});
});
```

- A handler receives the sender and the arguments, and answers itself with `SendTo`.
- Several handlers can be registered for the same command; they are called in registration order.
- The network handles these commands itself, then calls their handlers if any (no error when there is none):

| Command | Done by the network |
|---|---|
| `CONNECT_REQUEST` (host) | Add the sender (or refuse with `FULL` / `NOT_CONNECTED`), answer `CONNECT_ACCEPT` |
| `CONNECT_ACCEPT` (client) | `IsConnected()` becomes `true` |
| `DISCONNECT` (server) | Remove the sender. Ignored by a room. |
| `DISCONNECT` (client) | `IsConnected()` becomes `false` |

- Any other valid command without handler: the fallback (`SetFallback`, useful for debugging) is called if it is set, then the sender receives `ERROR COMMAND_NOT_HANDLED`. An `ERROR` is never answered.
- Except for `CONNECT_REQUEST`, a host only processes datagrams from connected clients; others receive `ERROR NOT_CONNECTED`.
- Invalid datagrams are answered with an `ERROR` according to RFC §5, and never throw.

### DISCONNECT handlers are always called

Whenever a client leaves the list of a host socket, the `DISCONNECT` handlers of that socket are called once, with the client endpoint. The game can rely on them to remove the player entity:

| The client leaves because | When the handlers run |
|---|---|
| It sent `DISCONNECT` to the server | Right away |
| The host kicked it with `Disconnect(endpoint)` | Right away |
| `RemoveClient(endpoint)` was called (by the room's `DISCONNECT_ROOM` handler, or by the `Core` because the client left the server) | At the next `Poll()` of that socket, in the thread that polls it |

### Poll and threads

`Poll()` processes the datagrams already received and returns immediately. Each socket must be polled by one thread, the one that owns it: the `Core` thread for the server, the room thread for a room, the game loop for the client. Handlers run in that thread.

Only these methods may be called from another thread: `Clients`, `ClientCount`, `AnnouncedCount`, `HasClient`, `Announce`, `RemoveClient`, `GetMaxClients`, `SetMaxClients`. Register the handlers before the polling thread starts.

## Clock

`Clock::Tick()` returns `true` when at least 1/`TICK_RATE` of a second (120 Hz) has passed since the last tick that returned `true`. `Core` and `Room` use it in their loop:

```cpp
while (SERVER_RUN) {
    if (_clock.Tick()) {
        _socket.Poll();
        CloseRooms();
    }
}
```

The loop does not sleep, so each of these threads keeps one CPU core busy.

## Core (server lobby)

`Core` owns the server socket and the rooms. It registers the lobby handlers:

| Command | Action |
|---|---|
| `LIST_ROOMS` | Answer `ROOM_LIST` with the rooms that have a free slot |
| `CREATE_ROOM` | Create a room, announce the sender to it, answer `ROOM_JOINED(port)` |
| `JOIN_ROOM(room_id)` | Announce the sender to the room, answer `ROOM_JOINED(port)` (the room itself refuses when full) |
| `DISCONNECT` | Remove the client from its room (`RemoveClient`) |

Refusals are sent as `ERROR` with `ROOM_NOT_FOUND`, `ALREADY_IN_ROOM` or `TOO_MANY_ROOMS`. At most `DEFAULT_MAX_ROOMS = 12` rooms exist at once; room ids start at 1 and are never reused. At each tick, `Core` destroys the rooms that are empty after having had clients, and the rooms that stayed without client for `ROOM_IDLE_TIMEOUT` (one minute) after their last announcement.

## Room

A `Room` has its own socket (port chosen by the system, `Port()`), its own thread (`Start()`, `Stop()`) and its own `Clock`. It registers a `DISCONNECT_ROOM` handler that removes the sender with `RemoveClient`, which then calls the `DISCONNECT` handlers.

Game code plugs into the room by registering handlers on `net` in the `Room` constructor, before `Start()`:

```cpp
net.On(Command::PlayerInput, [this](const Endpoint& from, const ArgsList& args) {
    // check that `from` controls args[0][0], then apply args[0][1]
});
net.On(Command::Disconnect, [this](const Endpoint& from, const ArgsList&) {
    // the client `from` left the room, whatever the reason: remove its player entity
});
net.On(Command::DisconnectRoom, [this](const Endpoint& from, const ArgsList& args) {
    // optional: the client also gave its player entity id in args[0][0]
});
```

Until a game handler exists, game commands sent to a room are answered with `ERROR COMMAND_NOT_HANDLED`.

## Client flow

```cpp
r_type::UdpSocket net(r_type::Role::Client);

net.On(r_type::Command::RoomList, [](const r_type::Endpoint&, const r_type::ArgsList& rooms) {
    for (const r_type::Args& room : rooms) {
        std::uint32_t id = std::get<std::uint32_t>(room[0]);
        std::uint8_t freeSlots = std::get<std::uint8_t>(room[1]);
    }
});
net.On(r_type::Command::RoomJoined, [&net](const r_type::Endpoint& from, const r_type::ArgsList& args) {
    std::uint16_t port = static_cast<std::uint16_t>(std::get<std::uint32_t>(args[0][0]));
    net.Connect(from.address().to_string(), port);
});
net.On(r_type::Command::Error, [](const r_type::Endpoint&, const r_type::ArgsList& args) {
    auto error = static_cast<r_type::NetError>(std::get<std::uint8_t>(args[0][0]));
    std::uint8_t rejectedCommand = std::get<std::uint8_t>(args[0][1]);
});

net.Connect("127.0.0.1", 6767);                                  // lobby
net.Send(r_type::Command::ListRooms, {{}});
net.Send(r_type::Command::JoinRoom, {{roomId}});                 // or CreateRoom, {{}}
net.Send(r_type::Command::PlayerInput, {{entityId, std::uint8_t{0x01}}});
net.Send(r_type::Command::DisconnectRoom, {{entityId}});         // leave the room
net.Connect("127.0.0.1", net.ServerPort());                      // back to the lobby
net.Disconnect();                                                // quit the server
// and call net.Poll() at every frame
```

If the room answers `ERROR FULL` to the `CONNECT_REQUEST`, connect back to the server with `ServerPort()` and list the rooms again.

## Defaults

| Value | Default | Where |
|---|---|---|
| Server port | 6767 (`-p` option) | `DEFAULT_PORT`, `network.hpp` |
| Clients per server | 50 | `DEFAULT_SERVER_CLIENTS`, `core.hpp` |
| Clients per room | 4 | `DEFAULT_ROOM_CLIENTS`, `room.hpp` |
| Rooms | 12 | `DEFAULT_MAX_ROOMS`, `core.hpp` |
| Room without client | destroyed after 1 minute | `ROOM_IDLE_TIMEOUT`, `room.hpp` |
| Tick rate | 120 Hz | `TICK_RATE`, `clock.hpp` |

## Known limitations

- A client that crashes without sending `DISCONNECT` is never removed: there is no inactivity timeout yet.
- No reliable delivery, no ordering, no fragmentation (RFC §6).
- The `Core` and room loops do not sleep and use one CPU core each.
