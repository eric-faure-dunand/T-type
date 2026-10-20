# R-Type — Client (developer guide)

This guide explains how `r-type_client` is built: its main loop, its screens, and how it uses the shared network layer. The wire format is specified in [Protocol](RFC/Protocol.md); the shared `UdpSocket` is described in [Network](Network.md).

## Running

```sh
./r-type_client              # the menu asks for the server IP (default 127.0.0.1)
./r-type_client 10.0.0.5     # pre-fills the IP field
```

The port is always the server default, `6767`. On Windows the executable opens without a console, so it can be started with a double click.

## Architecture

The client is single-threaded. `ClientCore` owns the window, the socket and the client copy of the game, the same way a server `Room` owns its socket:

```text
 ClientCore
 ├── Display _display    window, drawing, keyboard, mouse (the only Raylib user)
 ├── UdpSocket _net      shared network layer, Role::Client
 └── World _world        entities received from the server, stored in the ECS Registry

 each frame:  _net.Poll()  ->  handlers update _world / change scene
              timeouts, input resend
              draw the current scene
```

`UdpSocket::Poll()` never blocks: it handles the datagrams already received and returns. Calling it once per frame is enough, so the client needs no network thread (the subject only requires the server to be multithreaded). Handlers run inside `Poll()`, in the main thread, and can therefore change the scene and the `World` directly.

## Files

| Path | Content |
|---|---|
| `project/src/client/main.cpp` | Reads the optional IP argument, runs `ClientCore` |
| `project/include/client/Core/ClientCore.hpp` | `ClientCore`, constants (window, timeouts), input actions and key bindings, colour palette |
| `project/src/client/Core/ClientCore.cpp` | Main loop, scenes, timeouts, starfield, UI helpers |
| `project/src/client/Core/connection.cpp` | Connect to the server, go to a room and back, leave, send inputs |
| `project/src/client/Core/Handlers/connection.cpp` | Handler registration, `CONNECT_ACCEPT`, `DISCONNECT`, `ERROR` |
| `project/src/client/Core/Handlers/rooms.cpp` | `ROOM_LIST`, `ROOM_JOINED` |
| `project/src/client/Core/Handlers/entities.cpp` | `PLAYER_CREATION`, initial state, `ENTITY_SPAWN` / `UPDATE` / `DESTROY` |
| `project/src/client/Core/Scenes/menu.cpp` | IP input screen |
| `project/src/client/Core/Scenes/lobby.cpp` | Room list, create / join |
| `project/src/client/Core/Scenes/game.cpp` | Draws the entities, sends the inputs |
| `project/include/client/World/World.hpp`, `src/client/World/World.cpp` | Client copy of the game in the ECS `Registry`, entity kinds |
| `project/include/client/Display/Display.hpp`, `src/client/Display/Display.cpp` | Window, drawing, input. **The only file that includes Raylib.** |

`Display.cpp` includes nothing from the project but `Display.hpp`. This keeps Raylib out of the rest of the client, and avoids name clashes: Raylib's `Color` collides with the project's `Color` namespace (`printer.hpp`), and Raylib's `DrawText` / `CloseWindow` collide with the Windows headers pulled by Asio. It also makes the client testable: the logic can be compiled against a fake `Display`.

## Screens

The current scene is also the connection state:

| Scene | Current host | Leaves the scene on |
|---|---|---|
| `Menu` | none | ENTER → `Connecting` |
| `Connecting` | server | `CONNECT_ACCEPT` → `Lobby`; `ERROR` or 3 s without answer → `Menu` |
| `Lobby` | server | C / ENTER: `CREATE_ROOM` / `JOIN_ROOM` → `Joining`; ESC → `Menu` |
| `Joining` | server, then room | `ROOM_JOINED`: connect to the room port; room `CONNECT_ACCEPT` → `Game`; `ERROR` or 3 s → `Lobby` |
| `Game` | room | ESC: `DISCONNECT_ROOM`, reconnect to the server → `Leaving` |
| `Leaving` | server | `CONNECT_ACCEPT` → `Lobby`; 3 s → `Menu` |

The client always uses the same UDP socket, as required by RFC §2.1: a room only accepts the endpoint announced by the server. Closing the window sends `DISCONNECT_ROOM` to the room and `DISCONNECT` to the server.

Everything can be done with the keyboard only (arrows, ENTER, ESC, C, R); the mouse also works.

## Game data

- `PLAYER_CREATION` gives the client its own `entity_id` and colour. Without it, no input is sent.
- `INITIAL_STATE_BEGIN` / `ENTITY_SPAWN` x N / `INITIAL_STATE_END` are buffered and applied at once, only if the count matches (RFC §3.1). Otherwise the client sends `CONNECT_REQUEST` to the room again to get a fresh transfer.
- A destroyed `entity_id` is remembered for the whole game: a late `ENTITY_SPAWN` or `ENTITY_UPDATE` cannot bring it back.
- `World` stores each entity in the ECS `Registry` with three components: `NetIdentity` (server id), `Transform` (position) and `Appearance` (kind, colour). The game screen draws them with `Registry::each`.

## Inputs

Arrows move, SPACE fires. Keys are mapped to actions by the `BINDINGS` table in `ClientCore.hpp`:

```cpp
constexpr std::array<Binding, 5> BINDINGS = {{
    {Key::Up, Action::Up}, {Key::Down, Action::Down}, {Key::Left, Action::Left},
    {Key::Right, Action::Right}, {Key::Space, Action::Fire},
}};
```

Each frame, the game screen builds the set of held actions. For each action that changes, the client sends one `PLAYER_INPUT` (`Pressed` or `Released`). Because UDP can lose a packet:

- every held action is sent again as `Pressed` every 100 ms;
- every released action is sent again as `Released` 3 times, every 100 ms.

A lost release can therefore never leave the ship moving forever, and the traffic only grows with the number of actions actually in use.

To add an action: add a value to `Action` (and to the RFC, §1), add a `Key` to `Display` if needed, and add one line to `BINDINGS`.

## Adding a command

1. Add it to `Command` (`protocol.hpp`) and to the registry (`protocol.cpp`), and document it in [Protocol](RFC/Protocol.md).
2. Add an `On...` method to `ClientCore`, write it in the matching `Handlers/*.cpp` file, and register it in `RegisterHandlers()`.

## Limitations

- Entities are drawn as coloured rectangles until sprites are added to `Display`.
- No interpolation: positions jump at each `ENTITY_UPDATE` (20 Hz target).
- No sound yet.
