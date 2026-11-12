# R-Type — Network layer (developer guide)

This guide explains how to use the network code of the project: the shared UDP layer (`UdpSocket`), the server lobby (`Core`), the rooms (`Room`) and the tick `Clock`. The binary wire format, command codes and validation rules are specified in the RFC: [Readme](RFC/Readme.md) and [Protocol](RFC/Protocol.md).

## Files

| Path | Content |
|---|---|
| `project/include/shared/Network/protocol.hpp` | Constants, `Command`, `NetError`, `Role`, command registry (direction and argument schema of each command) |
| `project/include/shared/Network/packet.hpp` | `Arg`, `Args`, `ArgsList`, `Datagram`, `EncodeMessage`, `DecodePacket` |
| `project/include/shared/Network/channel.hpp` | `Channel`: acknowledgements, retransmissions and reassembly for one peer (no socket inside) |
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

- Every command carries exactly one record, except the list commands: `ROOM_LIST`, `ENTITY_UPDATE`, `ENTITY_SPAWN` and `ENTITY_DESTROY` (0 to N records each), and `ACK` and `RESEND`, which only the network layer sends. Even a single entity is sent as a list of one record: the handlers of these commands walk all the records of `args`.
- A command without arguments is sent with `{{}}` (one empty record).
- The C++ type of each argument must match the schema: `42u` is a `uint32`, `-100` an `int32`, and a `uint8` must be written `std::uint8_t{1}`. A mismatch throws `Error`.

```cpp
net.Send(Command::PlayerInput, {{entityId, std::uint8_t{0x00}, std::uint8_t{0x01}}});   // entity_id u32, action u8, input_state u8
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
| `RemoveClient(endpoint)` | Remove a client without telling it, except for the `ACK`s still queued for it; callable from another thread (see below) |
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

A list that does not fit in one datagram is split by `EncodeMessage` into fragments, sent one after the other. For a reliable command the receiver rebuilds the whole list: the sender passes the whole list, and the handler receives the whole list. A list of a command that is not reliable (`ENTITY_UPDATE`) is not rebuilt: the handler is called once per received datagram, with the records of this datagram, so a lost fragment only loses its own records. A list needing more than `MAX_FRAGMENTS` (32) datagrams makes the send throw `Error`. If the command is reliable, the datagrams are kept until the receiver acknowledges the message (next section).

### Fragmentation and reliable delivery

The wire rules are in RFC [Protocol.md §3.2](RFC/Protocol.md#32-fragmentation-and-reliable-delivery). In the code:

| What | Where |
|---|---|
| Split a message into datagrams, each with the `Message id` of the message and its fragment index | `EncodeMessage` (`packet.cpp`) |
| `reliable` flag of each command | `CommandInfo::reliable`, registry in `protocol.cpp` |
| Keep the sent datagrams, queue the `ACK`s, reassemble fragments, ignore duplicates, ask for missing fragments | `Channel` (`channel.cpp`), one per peer, owned by `UdpSocket` |
| Send and receive the datagrams, call the `Channel` | `UdpSocket` |

`Channel` never touches a socket and takes the time as an argument. It only returns what must be sent, so its logic can be tested alone. `UdpSocket` creates a `Channel` the first time it sends a reliable message to an endpoint, or receives a reliable or fragmented message from a connected endpoint, and removes it when the endpoint leaves the list of clients, when it has nothing left to do, or on `Close()`.

`Poll()` does the whole job: after processing the received datagrams, it sends the pending `ACK`s and `RESEND`s and retransmits the messages that were not acknowledged in time. A socket that is not polled regularly neither acknowledges nor retransmits.

| Constant | Value | Where |
|---|---|---|
| Wait for an `ACK` before retransmitting | 200 ms | `RESEND_DELAY`, `channel.hpp` |
| Wait before asking for a missing fragment | 50 ms | `FRAGMENT_REQUEST_DELAY`, `channel.hpp` |
| Retransmissions or `RESEND`s before giving up | 5 | `MAX_RESENDS`, `channel.hpp` |
| Incomplete messages kept per peer | 8 | `MAX_PARTIAL_MESSAGES`, `channel.hpp` |
| Message ids remembered per peer, to ignore duplicates | 1024 | `SEEN_WINDOW`, `channel.hpp` |
| Fragments per message | 32 | `MAX_FRAGMENTS`, `protocol.hpp` |

What the game must know:

- A reliable message is delivered once, even if the sender retransmitted it. It is not delivered in order: a retransmitted message arrives after the ones sent later.
- A list that is not reliable can reach the handler in several calls, one per datagram, and some of its records can be lost. A handler of `ENTITY_UPDATE` must not expect the records of all the entities in one call.
- After `MAX_RESENDS` retransmissions the message is forgotten without notice. The game does not learn that the peer is unreachable.
- A reliable message refused by the network layer (`NOT_CONNECTED`, `FULL`, ...) is acknowledged anyway, at once and without keeping any state about the sender, besides receiving its `ERROR`. The sender stops retransmitting.
- Every command is reliable except `PLAYER_INPUT`, `ENTITY_UPDATE`, `ERROR`, `ACK` and `RESEND`. The connection handshake (`CONNECT_REQUEST`, `CONNECT_ACCEPT`) and `DISCONNECT` are reliable.

To make a command reliable, set the fifth field of its entry in `protocol.cpp` to `true` and mark it in the RFC table:

```cpp
{Command::MyCommand, {Direction::ServerToClient, {ArgType::U32}, false, false, true}},   // schema, list, reserved, reliable
```

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

- `ACK` and `RESEND` are consumed by the network layer: no handler is called and they are never answered.
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

### Destruction

When a `UdpSocket` is destroyed with its socket open, it says goodbye before closing:

| Socket | Sends |
|---|---|
| Server or room | `DISCONNECT` to every connected client |
| Client | `DISCONNECT` to the server given to its first `Connect`, even when its current host is a room; the `Core` then removes it from its room, which is notified as above |

A client ignores a `DISCONNECT` that does not come from its current host: when the server stops, a client in a room is warned by its room, which `Core` destroys before the server socket. The destructor sends each `DISCONNECT` once, since it cannot retransmit it. It never calls the local handlers, since the objects they use may already be destroyed, and never throws.

### Poll and threads

`Poll()` processes the datagrams already received and returns immediately. Each socket must be polled by one thread, the one that owns it: the `Core` thread for the server, the room thread for a room, the game loop for the client. Handlers run in that thread.

Only these methods may be called from another thread: `Clients`, `ClientCount`, `AnnouncedCount`, `HasClient`, `Announce`, `RemoveClient`, `GetMaxClients`, `SetMaxClients`. Register the handlers before the polling thread starts.

## Clock

`Clock::Tick()` returns `true` when at least 1/`TICK_RATE` of a second (120 Hz) has passed since the last tick that returned `true`. `Core` and `Room` use it in their loop:

```cpp
while (APP_RUN) {
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

The room sends the state of the game to its clients with lists, built once and sent to each client, instead of one datagram per entity and per client:

| Command | When | Content |
|---|---|---|
| `ENTITY_SPAWN`, `ENTITY_DESTROY` | Every tick | The walk of `SyncEntities` over the entities that have a `Kind` and a `Position` finds the ones not yet synchronised (`_synced`) and the ones that disappeared; the spawns and the destroys of the tick are each gathered in one list, and `Broadcast` sends it once to every client |
| `ENTITY_UPDATE` | Every `TICK_RATE / ROOM_UPDATE_RATE` ticks, i.e. 20 Hz | The same walk gathers the positions of all the entities in one list, sent to every client by `Broadcast` |
| `ENTITY_SPAWN` (initial state) | When a client joins | `SendInitialState` walks the ECS once and sends all the entities in one list, between `INITIAL_STATE_BEGIN` and `INITIAL_STATE_END` |
| `ENTITY_SPAWN` (new player) | When a client joins | `BroadcastPlayerSpawn` sends the new player to the others, as a list of one record: the player is already in `_synced`, so `SyncEntities` does not send it again |

`Broadcast` and `SendInitialState` go through `SendList`, which cuts a list into messages of at most `MAX_RECORDS_PER_MESSAGE = 2048` records. A message is limited to 32 fragments, that is 2688 `ENTITY_SPAWN`, 3136 `ENTITY_UPDATE` or 9440 `ENTITY_DESTROY` records, and a room can hold up to `kMaxEntities` (5254) entities: without this cut, `SendTo` would throw in the room thread for a very full room. The clients accept the pieces one after the other (the initial state counts its records in `INITIAL_STATE_BEGIN`).

To replicate a new kind of event, gather its records in a list during the walk of `SyncEntities` and send it with `Broadcast`: never send one datagram per entity inside a loop over the clients.

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
| `ENTITY_UPDATE` rate | 20 Hz | `ROOM_UPDATE_RATE`, `room.hpp` |
| Reliability delays and limits | see the table above | `channel.hpp` |

## Known limitations

- A client that crashes or is killed before its `UdpSocket` destructor runs never sends `DISCONNECT`, so it is never removed: there is no inactivity timeout yet.
- Only the commands flagged reliable are acknowledged and retransmitted, and a message is abandoned after `MAX_RESENDS` retransmissions. Messages are never ordered, and only lists are fragmented (RFC §6).
- The client still gives up on its own timeout (`CONNECT_LIMIT`, 3 s) when the server does not answer, after the 5 retransmissions of `CONNECT_REQUEST` (about 1 s).
- The ids a socket remembers are forgotten when the peer is removed or on `Close()`. A peer that restarts under the same endpoint without having been removed counts again from 1, and its first messages are taken for duplicates.
- The `Core` and room loops do not sleep and use one CPU core each.
