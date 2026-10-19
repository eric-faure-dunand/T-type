# R-Type — Client (developer guide)

This guide explains how `r-type_client` is built: its two threads, its screens, and how it uses the shared network layer. The wire format is specified in [Protocol](RFC/Protocol.md); the shared `UdpSocket` is described in [Network](Network.md).

## Running

```sh
./r-type_client              # the menu asks for the server IP (default 127.0.0.1)
./r-type_client 10.0.0.5     # pre-fills the IP field
```

The port is always the server default, `6767`. On Windows the executable opens without a console, so it can be started with a double click.

## Architecture

```text
            main thread (variable FPS)                     network thread (120 Hz)
 ┌───────────────────────────────────────┐        ┌─────────────────────────────────────┐
 │ ClientCore                            │ Push() │ ClientNetwork                        │
 │  scenes: Menu, Lobby, Game ...        │───────>│  requests queue -> Process()         │
 │  World (ECS Registry)                 │        │  UdpSocket::Poll() -> handlers       │
 │  Display (the only Raylib user)       │<───────│  events queue <- Emit()              │
 └───────────────────────────────────────┘ Consume└─────────────────────────────────────┘
                                          Events()
```

- **Display** runs in the main thread, as Raylib requires. It draws as fast as the screen allows (V-Sync).
- **Network** runs in its own thread at 120 ticks per second (the shared `Clock`, same rate as the server). It is the only thread that touches the `UdpSocket`, as required by [Network](Network.md#poll-and-threads).
- The two threads never share game data. They exchange messages through two mutex-protected queues:
  - `NetRequest` (main → network): `Connect`, `ListRooms`, `CreateRoom`, `JoinRoom`, `Input`, `LeaveRoom`, `Disconnect`;
  - `NetEvent` (network → main): `Connected`, `ConnectionFailed`, `Rooms`, `RoomEntered`, `PlayerCreated`, `Spawn`, `Update`, `Destroy`, `Snapshot`, ...

  Both are `std::variant`s defined in `ClientEvents.hpp`. This is the same idea as the room's pending input queue on the server.

## Files

| Path | Content |
|---|---|
| `project/src/client/main.cpp` | Reads the optional IP argument, runs `ClientCore` |
| `project/include/client/Core/ClientCore.hpp`, `src/client/Core/ClientCore.cpp` | Main loop, event handling, starfield, shared UI helpers |
| `project/src/client/Core/Scenes/menu.cpp` | IP input screen |
| `project/src/client/Core/Scenes/lobby.cpp` | Room list, create / join |
| `project/src/client/Core/Scenes/game.cpp` | Draws the entities sent by the server, sends the inputs |
| `project/include/client/Network/ClientEvents.hpp` | Requests, events, wire constants (entity kinds, input flags) |
| `project/include/client/Network/ClientNetwork.hpp`, `src/client/Network/ClientNetwork.cpp` | Network thread, connection stages, timeouts, inputs |
| `project/src/client/Network/handlers.cpp` | One handler per received command |
| `project/include/client/World/World.hpp`, `src/client/World/World.cpp` | Client copy of the game, stored in the ECS `Registry` |
| `project/include/client/Display/Display.hpp`, `src/client/Display/Display.cpp` | Window, drawing, keyboard and mouse. **The only file that includes Raylib.** |

`Display.cpp` includes nothing from the project but `Display.hpp`. This keeps Raylib out of the rest of the client, and avoids name clashes: Raylib's `Color` collides with the project's `Color` namespace (`printer.hpp`), and Raylib's `DrawText` / `CloseWindow` collide with the Windows headers pulled by Asio.

## Screens

```text
 Menu ──ENTER──> Connecting ──CONNECT_ACCEPT──> Lobby ──C / ENTER──> Joining ──CONNECT_ACCEPT (room)──> Game
  ^                  │                            │                     │                                │
  └──── failure ─────┘                            └──── ESC ──> Menu    └── FULL / timeout ──> Lobby      └── ESC ──> Leaving ──> Lobby
```

Everything can be done with the keyboard only (arrows, ENTER, ESC, C, R); the mouse also works.

## Network stages

`ClientNetwork` follows RFC §2.1. The client talks to one host at a time, always from the same UDP socket:

| Stage | Host | Leaves the stage on |
|---|---|---|
| `ConnectingServer` | server | `CONNECT_ACCEPT` → `Lobby`; `ERROR` or 3 s without answer → offline |
| `Lobby` | server | `ROOM_JOINED` → connects to the room port → `JoiningRoom` |
| `JoiningRoom` | room | `CONNECT_ACCEPT` → `InRoom`; `ERROR FULL` or 3 s → back to the server |
| `InRoom` | room | `LeaveRoom` → sends `DISCONNECT_ROOM`, reconnects to the server → `ReturningToLobby` |
| `ReturningToLobby` | server | `CONNECT_ACCEPT` → `Lobby` |

Closing the window sends `DISCONNECT_ROOM` to the room and `DISCONNECT` to the server.

## Game data

- `PLAYER_CREATION` gives the client its own `entity_id` and colour. Without it, no input is sent.
- `INITIAL_STATE_BEGIN` / `ENTITY_SPAWN` x N / `INITIAL_STATE_END` are buffered and applied at once as a `Snapshot`, only if the count matches (RFC §3.1). Otherwise the client sends `CONNECT_REQUEST` to the room again to get a fresh transfer.
- A destroyed `entity_id` is remembered for the whole game: a late `ENTITY_SPAWN` or `ENTITY_UPDATE` cannot bring it back.
- `World` stores each entity in the ECS `Registry` with three components: `NetIdentity` (server id), `Transform` (position) and `Appearance` (kind, colour). The game screen draws them with `Registry::each`.

## Inputs

Arrows move, SPACE fires. When the keys change, the client sends one `PLAYER_INPUT` `Pressed` with the new keys and one `Released` with the released keys. Because UDP can lose a packet, the full state is also sent again every 100 ms: one `Pressed` with every held key and one `Released` with every other key. A lost release can therefore never leave the ship moving forever.

## Adding a command

1. Add it to `Command` (`protocol.hpp`) and to the registry (`protocol.cpp`), and document it in [Protocol](RFC/Protocol.md).
2. Add an `On...` method in `ClientNetwork` and register it in `handlers.cpp`.
3. If the main thread must know about it, add an event to `NetEvent` and handle it in `ClientCore::HandleEvent`.

## Limitations

- Entities are drawn as coloured rectangles until sprites are added to `Display`.
- No interpolation: positions jump at each `ENTITY_UPDATE` (20 Hz target).
- No sound yet.
