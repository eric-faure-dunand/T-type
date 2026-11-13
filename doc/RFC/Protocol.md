# R-Type — Binary UDP protocol specification

**Status:** Proposed Part 1 protocol, issue #19 ([2.1]). This is a wire-format contract for implementation, not a claim that the server/client already implement these messages.

**Dependencies:** The network team has approved the Part 1 replicated-data baseline from issue #9, documented in [ReplicatedComponents.md](ReplicatedComponents.md). The payloads below implement that baseline. Additional synchronized components require an explicit protocol revision.

See [ReplicatedComponents.md](ReplicatedComponents.md)
for the list of synchronized game data and update frequencies.

## 1. Common encoding

Every UDP datagram has the 17-byte header documented in [Readme.md](Readme.md). All integers are unsigned and big-endian unless stated otherwise. Signed coordinates are encoded as **signed 32-bit two's-complement integers** in big-endian order. No compiler-native structs, padding, pointers, or C++ object layouts are transmitted.

Part 1 constraints:

- `Magic = 0x5254`.
- `17 <= datagram_size <= 1200`.
- `Size = datagram_size - 17`, maximum 1183.
- `Message id` identifies the logical message. It is chosen by the sender and shared by all the fragments of the message (§3.2).
- `Sequence number` is the 1-based fragment index and `Sequence length` the total fragment count of the logical message: `1 <= Sequence number <= Sequence length <= 32`. A message that is not fragmented is fragment 1 of 1. A `Sequence length` above 1 is only valid for a list command (see below). Neither field is a datagram counter nor an acknowledgement: the header carries no ordering information.
- Each payload carries exactly **one record** laid out by the command's fixed schema in §3, except the list commands `ROOM_LIST`, `ENTITY_UPDATE`, `ENTITY_SPAWN`, `ENTITY_DESTROY`, `ACK` and `RESEND`, whose payload is a list of 0 to N records of the same schema. A list too long for one datagram is fragmented (§3.2).
- A client-to-server packet is accepted only if its command is permitted in that direction and its connection is authorized for the referenced entity.

**Coordinates:** `x` and `y` are integer world coordinates in game units (the initial implementation may use pixel-equivalent units). Position is absolute, not a relative delta. The world origin and coordinate bounds are determined by the gameplay implementation and must be validated server-side.

**Input actions:** one byte identifying a single action: `0x00` = up, `0x01` = down, `0x02` = left, `0x03` = right, `0x04` = fire; `0x05`–`0xFF` are reserved for future actions. Each `PLAYER_INPUT` carries exactly one action, so up to 256 actions can be added without changing the field type. Conflicting opposite directions held at the same time are resolved by the server (for example, cancel each other).

## 2. Command registry

| Code | Command | Direction | Handled by | Size | Reliable | Purpose |
|---|---|---|---|---:|:---:|---|
| `0x01` | `CONNECT_REQUEST` | Client → Server | Server, Room | 0 | yes | Ask to connect |
| `0x02` | `CONNECT_ACCEPT` | Server → Client | Server, Room | 0 | yes | Accept the connection |
| `0x03` | `DISCONNECT` | Both | Server | 0 | yes | End the connection to the server |
| `0x04` | `CREATE_ROOM` | Client → Server | Server | 0 | yes | Create a room and enter it |
| `0x05` | `JOIN_ROOM` | Client → Server | Server | 4 | yes | Ask to enter a room |
| `0x06` | `ROOM_JOINED` | Server → Client | Server | 4 | yes | Give the port of the room to connect to |
| `0x07` | `LIST_ROOMS` | Client → Server | Server | 0 | yes | Ask for the rooms with free slots |
| `0x08` | `ROOM_LIST` | Server → Client | Server | 5 × N | yes | List the rooms with free slots |
| `0x09` | `DISCONNECT_ROOM` | Client → Server | Room | 4 | yes | Leave the room, giving the player entity |
| `0x10` | `PLAYER_INPUT` | Client → Server | Room | 6 | no | Submit input press/release events for a controlled entity |
| `0x11` | `CHOOSE_CHARACTER` | Client → Server | Room | 5 | yes | Ask to play a given character |
| `0x12` | `CHOOSE_LEVEL` | Client → Server | Room | 5 | yes | Ask the room to play a given level |
| `0x20` | `INITIAL_STATE_BEGIN` | Server → Client | Room | 4 | yes | Begin an initial state transfer |
| `0x21` | `ENTITY_UPDATE` | Server → Client | Room | 12 × N | no | Publish the authoritative positions of the entities |
| `0x22` | `ENTITY_SPAWN` | Server → Client | Room | 14 × N | yes | Create or refresh entity representations |
| `0x23` | `ENTITY_DESTROY` | Server → Client | Room | 4 × N | yes | Remove entities |
| `0x24` | `INITIAL_STATE_END` | Server → Client | Room | 4 | yes | End an initial state transfer |
| `0x25` | `PLAYER_CREATION` | Server → Client | Room | 13 | yes | Tell a client which entity it controls |
| `0x26` | `BOSS_HEALTH` | Server → Client | Room | 5 | yes | Give the health of a boss |
| `0x27` | `LEVEL_START` | Server → Client | Room | 1 | yes | Tell the clients which level the room plays |
| `0x30` | `ACK` | Both | Network layer | 4 × N | no | Acknowledge reliable messages |
| `0x31` | `RESEND` | Both | Network layer | 8 × N | no | Ask for the missing fragments of a reliable message |
| `0x84` | `ERROR` | Both | Server, Room | 2 | no | Report a rejected datagram to its sender |

In the *Direction* column, *Server* means the host side: the server socket or a room socket. The *Handled by* column says where the command is meant to be handled (§2.1). `CONNECT_REQUEST`, `CONNECT_ACCEPT` and `DISCONNECT` are handled by the network layer itself, except that a room ignores `DISCONNECT` (§2.1). `ACK` and `RESEND` are consumed by the network layer (§3.2): they are never handed to the game and never answered. Any other valid command for which the receiver has no handler is answered with `ERROR COMMAND_NOT_HANDLED`, except `ERROR`, which is never answered.

The *Reliable* column tells whether the command is acknowledged and retransmitted (§3.2). The flag belongs to the command, not to the datagram: sender and receiver both read it from this registry. Every command is reliable except five. `PLAYER_INPUT` is repeated by the client (§3) and `ENTITY_UPDATE` carries an absolute position that the next update corrects, so acknowledging them would only cost bandwidth. `ACK` and `RESEND` are not acknowledged, otherwise acknowledgements would never end, and `ERROR` is never answered (§5).

Unknown and reserved command codes are rejected. No command is reserved at the moment. All payload layouts below are exact: additional bytes are invalid.

## 2.1. Server and room sockets

The server listens on one UDP port (default 6767) for the lobby. Each room listens on its own UDP port, chosen by the operating system when the room is created.

1. The client connects to the server (`CONNECT_REQUEST` → `CONNECT_ACCEPT`). The server accepts a limited number of clients (default 50); beyond, it answers `ERROR FULL`.
2. The client asks for the rooms (`LIST_ROOMS` → `ROOM_LIST`), then creates a room (`CREATE_ROOM`) or joins one by id (`JOIN_ROOM`). The server answers `ROOM_JOINED` with the room port and announces the client endpoint to that room. The creator of a room enters it like any other client.
3. The client sends `CONNECT_REQUEST` to the room port, from the same UDP endpoint. The room only accepts endpoints announced by the server; it answers `CONNECT_ACCEPT`, or `ERROR FULL` when it already holds its maximum number of clients (default 4). After a refusal, the client connects back to the server port it kept and may list and join again.
4. Game commands are exchanged with the room. The client stays connected to the server while it is in a room.
5. `DISCONNECT_ROOM` sent to the room leaves the room. `DISCONNECT` sent to the server ends the connection and also removes the client from its room. A room ignores `DISCONNECT`.

On the client side, the client talks to one host at a time: the last server or room it sent `CONNECT_REQUEST` to, always from the same UDP endpoint. It keeps the server port from its first connection, so that it can come back to the lobby by sending `DISCONNECT_ROOM` to the room, then `CONNECT_REQUEST` to the server again; the server answers `CONNECT_ACCEPT` since the client is still in its list.

Whatever the reason a client leaves a room (`DISCONNECT_ROOM`, removal because it sent `DISCONNECT` to the server, or the room disconnecting it), the room is notified with the client endpoint, so that the game never keeps a motionless player entity. `DISCONNECT_ROOM` also gives the id of the player entity. The implementation is described in [doc/Network.md](../Network.md).

A client is in at most one room: `CREATE_ROOM` or `JOIN_ROOM` from a client already connected to a room is answered with `ERROR ALREADY_IN_ROOM`. The number of rooms is limited by the server (default 12); beyond, `CREATE_ROOM` is answered with `ERROR TOO_MANY_ROOMS`. Room ids are assigned by the server and never reused while it runs. A room is destroyed as soon as it becomes empty after having had clients, or one minute after its last announcement if nobody connected; its port is then closed. Limits and timeouts are implementation defaults, not wire constants.

## 3. Payload layouts

Offsets in this section are relative to the **start of the payload** (immediately after byte 12 of the common header).

### `CONNECT_REQUEST` (`0x01`)

Empty payload. The client asks the server, or a room (§2.1), whether it may connect. The receiver adds the sender endpoint to its list of connected clients and answers `CONNECT_ACCEPT`. A request from an endpoint that is already connected is answered again (e.g. when the previous `CONNECT_ACCEPT` was lost) without adding it twice. A retransmission of the same message (same `Message id`, §3.2) is only acknowledged. When its list is full, the receiver answers `ERROR FULL`. A room also answers `ERROR NOT_CONNECTED` to an endpoint the server did not announce. A UDP endpoint is not proof of identity; spoofing protection and authentication are future work.

### `CONNECT_ACCEPT` (`0x02`)

Empty payload. The server or the room accepts the connection. The client only takes into account a `CONNECT_ACCEPT` coming from its current host (§2.1). Nothing else is assigned: no entity.

### `DISCONNECT` (`0x03`)

Empty payload. Never answered, but acknowledged like any reliable message (§3.2).

- Sent by a client to the server: ends the connection. The server removes the sender from its list of connected clients and from its room. A `DISCONNECT` from an endpoint that is not connected is ignored.
- Sent by a client to a room: ignored. A client leaves a room with `DISCONNECT_ROOM`.
- Sent by the server or a room to a client: the client is removed from that list and considers itself disconnected from its current host. A client ignores a `DISCONNECT` that does not come from its current host.

### `CREATE_ROOM` (`0x04`)

Empty payload. The server creates a room, announces the sender to it and answers `ROOM_JOINED`. Refused with `ERROR ALREADY_IN_ROOM` or `ERROR TOO_MANY_ROOMS` (§2.1).

### `JOIN_ROOM` (`0x05`)

| Offset | Field | Type | Description |
|---:|---|---|---|
| 0 | `room_id` | uint32 | Id of the room, as given by `ROOM_LIST` |

The server announces the sender to the room and answers `ROOM_JOINED`, without checking whether the room is full: the room itself refuses with `FULL` (§2.1). Refused with `ERROR ROOM_NOT_FOUND` for an unknown id, and with `ERROR ALREADY_IN_ROOM`.

### `ROOM_JOINED` (`0x06`)

| Offset | Field | Type | Description |
|---:|---|---|---|
| 0 | `port` | uint32 | UDP port of the room on the server's IP address, always below 65536 |

The client then sends `CONNECT_REQUEST` to that port, from the same UDP endpoint, and keeps the server port to come back later. The room id is not repeated: the client only needs the port.

### `LIST_ROOMS` (`0x07`)

Empty payload. The server answers `ROOM_LIST`.

### `ROOM_LIST` (`0x08`)

A list of records, one per room with at least one free slot. `Size` must be a multiple of 5; an empty payload means that no room is available. A datagram holds at most 236 records; a longer list is fragmented (§3.2), up to 32 fragments.

| Offset in record | Field | Type | Description |
|---:|---|---|---|
| 0 | `room_id` | uint32 | Id to send in `JOIN_ROOM` |
| 4 | `free_slots` | uint8 | Number of clients the room can still accept |

### `DISCONNECT_ROOM` (`0x09`)

| Offset | Field | Type | Description |
|---:|---|---|---|
| 0 | `entity_id` | uint32 | Player entity controlled by the client in this room |

Sent by a client to its room to leave it. Never answered. The room removes the sender from its list of connected clients and notifies the game, which removes the player entity. The client stays in the server's list and goes back to the lobby by connecting to the server again (§2.1). As for any client-provided entity id, the room must check that the sender controls `entity_id` before acting on that entity. Sent to the server, it is answered with `ERROR COMMAND_NOT_HANDLED`.

### `PLAYER_INPUT` (`0x10`)

| Offset | Field | Type | Description |
|---:|---|---|---|
| 0 | `entity_id` | uint32 | Entity the client intends to control |
| 4 | `action` | uint8 | One action, see *Input actions* (§1) |
| 5 | `input_state` | uint8 | `0x00` = Released, `0x01` = Pressed |

The client sends a `Pressed` event when an action is activated and a `Released` event when it is deactivated, one `PLAYER_INPUT` per action: if several actions change at the same time, several packets are sent. The server maintains the state of each action between events until the corresponding release is received. A reserved action code is ignored by the server.

Because UDP can lose packets, the client repeats the `Pressed` event of every held action periodically (every 100 ms), and repeats each `Released` event a few times after the release (3 times, every 100 ms). Both are idempotent for the server.

The server checks that the sender is allowed to control `entity_id`. Actions are requests, never a client-provided resulting position. Input rate limiting and replay handling belong in the server implementation.

### `CHOOSE_CHARACTER` (`0x11`)

| Offset | Field | Type | Description |
|---:|---|---|---|
| 0 | `entity_id` | uint32 | Player entity controlled by the client in this room |
| 4 | `character` | uint8 | `0x00` Ana, `0x01` Soha, `0x02` Hilal, `0x03` Eric; others reserved |

Sent by a client after `PLAYER_CREATION`, when the character it picked differs from the one the room gave it. Never answered directly. If no other player of the room plays this character, the room gives it to the sender and sends `ENTITY_SPAWN` for the player entity to every client, with the new character in `color`. Otherwise the request is ignored and the player keeps its character. The room checks that the sender controls `entity_id`. The client sends it once and never repeats it, so the command is reliable (§3.2): a lost datagram is retransmitted by the network layer.

### `CHOOSE_LEVEL` (`0x12`)

| Offset | Field | Type | Description |
|---:|---|---|---|
| 0 | `entity_id` | uint32 | Player entity controlled by the client in this room |
| 4 | `level` | uint8 | Place of the level in the list of `assets/levels/*.json` sorted by file name (shipped files: `0x00` argentine, `0x01` corse, `0x02` tunisie, `0x03` turquie). Values past the end of the list are ignored |

Sent by the client that created the room, after `PLAYER_CREATION`. A new room plays no level: players can move and shoot, but no enemy comes. The first valid `CHOOSE_LEVEL` loads the level and starts it, and the room answers every client with `LEVEL_START`. Later requests are ignored, as are requests from a sender that does not control `entity_id`, or for a level that does not exist. The server and the clients must ship the same level files. If the room cannot read the level file, it logs the error and keeps waiting.

### `INITIAL_STATE_BEGIN` (`0x20`)

| Offset | Field | Type | Description |
|---:|---|---|---|
| 0 | `entity_count` | uint32 | Number of entities expected in this transfer: the number of records of its `ENTITY_SPAWN` list |

On receipt, the client starts a fresh initial-state collection. The server sends all the existing entities in one `ENTITY_SPAWN` list, then `INITIAL_STATE_END`. Because UDP can lose messages, a client **must not** treat a partial transfer as a complete snapshot. If the count is not reached or the end marker is missing, the client should request a new connection/state transfer after a timeout; timeout values are implementation policy.

### `ENTITY_UPDATE` (`0x21`)

A list of records, one per entity. `Size` must be a multiple of 12. A datagram holds at most 98 records, and a longer list is fragmented (§3.2). The fragments of this list are not reassembled: each datagram is given to the game as a list of its own, so a lost fragment only loses its own records. The server builds the list once for all the entities and sends it to every client.

| Offset | Field | Type | Description |
|---:|---|---|---|
| 0 | `entity_id` | uint32 | Existing entity identifier |
| 4 | `x` | int32 | Authoritative absolute X coordinate |
| 8 | `y` | int32 | Authoritative absolute Y coordinate |

An update for an unknown entity must not implicitly create it. The client may defer or ignore it pending a spawn or state resynchronization. The header carries no ordering information (§1): a delayed update may overwrite a more recent position. Positions are absolute and sent at 20 Hz (one list every 50 ms, `ROOM_UPDATE_RATE` in `room.hpp`), so the next update corrects it.

### `ENTITY_SPAWN` (`0x22`)

A list of records, one per entity. `Size` must be a multiple of 14. A datagram holds at most 84 records. The command is reliable, so a list longer than one datagram is reassembled and given to the game at once (§3.2). The server groups the entities created during the same tick in one message per client, and does not send an empty list.

| Offset | Field | Type | Description |
|---:|---|---|---|
| 0 | `entity_id` | uint32 | Server-assigned entity ID |
| 4 | `entity_kind` | uint8 | `0x01` player ship, `0x02` enemy, `0x03` projectile; others reserved |
| 5 | `x` | int32 | Initial authoritative X coordinate |
| 9 | `y` | int32 | Initial authoritative Y coordinate |
| 13 | `color` | uint8 | Variant, depends on `entity_kind`. Player: its character, `0` to `3` (see `CHOOSE_CHARACTER`), unique among the players of the room; it also gives the player colour. Enemy: its type, see below. Projectile: `0x00` fired by a player, `0x01` fired by an enemy |

Enemy types: `0x00` ballon, `0x01` supporter, `0x02` defenseur, `0x03` scorpion, `0x04` harissa, `0x05` simit, `0x06` cay, `0x07` chat, `0x08` grognard, `0x09` sanglier, and the bosses `0x0A` El Diez, `0x0B` couscoussier, `0x0C` döner, `0x0D` Napoléon; others reserved. A client that does not know a value draws a generic enemy.

`entity_kind` is a rendering/spawning hint, not a C++ class identity. The `entity_kind` values are the approved Part 1 wire-level baseline from issue #9; adding more kinds or ECS fields requires a documented protocol revision. A duplicate spawn for the same live entity must not create a second entity.

### `ENTITY_DESTROY` (`0x23`)

A list of records, one per entity. `Size` must be a multiple of 4. A datagram holds at most 295 records. The command is reliable and reassembled like `ENTITY_SPAWN`. The server groups the entities destroyed during the same tick in one message per client.

| Offset | Field | Type | Description |
|---:|---|---|---|
| 0 | `entity_id` | uint32 | Entity to remove |

Unknown IDs may be ignored. The server **must not reuse an entity ID during a game** in Part 1. A delayed spawn for an already destroyed ID must not recreate that entity.

### `INITIAL_STATE_END` (`0x24`)

| Offset | Field | Type | Description |
|---:|---|---|---|
| 0 | `entity_count` | uint32 | Must match the begin marker and the number of collected unique entity IDs |

A matching count alone does **not** prove the transfer is complete. The rules in §3.1 apply. The messages of the transfer are reliable (§3.2), but the transfer itself has no transfer ID: a message abandoned by the network layer after its last retransmission still requires the client to retry.

### `BOSS_HEALTH` (`0x26`)

| Offset | Field | Type | Description |
|---:|---|---|---|
| 0 | `entity_id` | uint32 | The boss (an enemy whose type is `0x0A` or more, see `ENTITY_SPAWN`) |
| 4 | `health_percent` | uint8 | Remaining health, `0` to `100` |

Sent by the room to every client when a boss appears and each time its health percentage changes, and to a client after its initial state transfer for every live boss. It only drives the display: the boss disappears with `ENTITY_DESTROY` like any entity. A value for an unknown entity may be kept until its `ENTITY_SPAWN` arrives.

### `LEVEL_START` (`0x27`)

| Offset | Field | Type | Description |
|---:|---|---|---|
| 0 | `level` | uint8 | The level the room plays, same values as `CHOOSE_LEVEL` |

Sent by the room to every client when a level starts, and to a client after its initial state transfer if a level is already running. The client uses it to show the level's background and name.

### `PLAYER_CREATION` (`0x25`)

| Offset | Field | Type | Description |
|---:|---|---|---|
| 0 | `entity_id` | uint32 | Player entity controlled by this client |
| 4 | `x` | int32 | Initial authoritative X coordinate |
| 8 | `y` | int32 | Initial authoritative Y coordinate |
| 12 | `color` | uint8 | Player character, `0` to `3`: the first one free in the room (see `CHOOSE_CHARACTER`) |

Sent by the room to one client only, right after it accepted its `CONNECT_REQUEST`. It is the only way for a client to learn the `entity_id` to put in `PLAYER_INPUT` and `DISCONNECT_ROOM`; until it is received, the client sends no input. The other clients learn about this player through a regular `ENTITY_SPAWN`. A duplicate `PLAYER_CREATION` for the same entity must not create a second entity.

### `ACK` (`0x30`)

A list of records. `Size` must be a multiple of 4.

| Offset in record | Field | Type | Description |
|---:|---|---|---|
| 0 | `message_id` | uint32 | `Message id` of a reliable message that was completely received |

Sent by the receiver of reliable messages (§3.2). One `ACK` carries the ids of all the messages completed since the previous one. Never answered. An id the receiver of the `ACK` is not waiting for is ignored.

### `RESEND` (`0x31`)

A list of records. `Size` must be a multiple of 8.

| Offset in record | Field | Type | Description |
|---:|---|---|---|
| 0 | `message_id` | uint32 | `Message id` of a reliable fragmented message that is incomplete |
| 4 | `fragment_index` | uint32 | Index (`Sequence number`) of a fragment that has not been received |

Sent by the receiver of a reliable fragmented message to ask for its missing fragments (§3.2). The sender answers by sending these fragments again, unchanged. Never answered. A request for a message or a fragment the sender no longer holds is ignored.

### `ERROR` (`0x84`)

| Offset | Field | Type | Description |
|---:|---|---|---|
| 0 | `error_code` | uint8 | Reason of the rejection, see the table below |
| 1 | `rejected_command` | uint8 | Command byte of the rejected datagram, `0x00` when it could not be read |

Sent by either side in response to a rejected datagram, following the rules of §5. It is never fragmented (`Sequence number = 1`, `Sequence length = 1`). A received `ERROR` is informational and is never answered with another `ERROR`.

| Code | Name | Rejection |
|---|---|---|
| `0x01` | `DATAGRAM_TOO_SHORT` | Fewer than 17 bytes (never sent, local diagnostic only) |
| `0x02` | `DATAGRAM_TOO_LONG` | More than 1200 bytes |
| `0x03` | `BAD_MAGIC` | Magic number is not `0x5254` (never sent, local diagnostic only) |
| `0x04` | `SIZE_MISMATCH` | `Size != received_bytes - 17` |
| `0x05` | `BAD_FRAGMENT` | `Sequence number` is 0 or above `Sequence length`, `Sequence length` is above 32, or `Sequence length` is above 1 for a command that is not a list |
| `0x06` | `UNKNOWN_COMMAND` | Command code not in the registry |
| `0x07` | `RESERVED_COMMAND` | Reserved command code (none at the moment) |
| `0x08` | `WRONG_DIRECTION` | Command not allowed in this direction |
| `0x09` | `BAD_PAYLOAD_SIZE` | Payload size does not match the command schema |
| `0x0A` | `NOT_CONNECTED` | Valid datagram sent to the server or a room by an endpoint that is not connected, or `CONNECT_REQUEST` to a room from an endpoint the server did not announce (see §5) |
| `0x0B` | `FULL` | `CONNECT_REQUEST` while the server or the room holds its maximum number of clients |
| `0x0D` | `ROOM_NOT_FOUND` | `JOIN_ROOM` with an unknown room id |
| `0x0E` | `COMMAND_NOT_HANDLED` | Valid command for which the receiver has no handler (§2) |
| `0x0F` | `ALREADY_IN_ROOM` | `CREATE_ROOM` or `JOIN_ROOM` from a client already in a room |
| `0x10` | `TOO_MANY_ROOMS` | `CREATE_ROOM` while the server holds its maximum number of rooms |

Error codes `0x0C` and `0x11`–`0xFF` are reserved for future revisions.

## 3.1. Initial-state transfer and UDP reordering (Part 1)

To make the initial transfer checkable without adding a transfer ID, the following **sender-side convention** is used:

1. For one joining client, the server sends `INITIAL_STATE_BEGIN`, one `ENTITY_SPAWN` list of exactly `entity_count` records, and `INITIAL_STATE_END` **consecutively** to that client. The server must not interleave other commands for that client during the transfer.
2. **Ordering is not guaranteed.** `INITIAL_STATE_BEGIN`, `ENTITY_SPAWN` and `INITIAL_STATE_END` are reliable (§3.2), so a lost one is sent again, but a message that was retransmitted arrives after the ones sent later: `Message id` is an identifier, not an ordering counter (§1).
3. The client commits the initial state **only** after receiving the begin marker, `entity_count` valid `ENTITY_SPAWN` records with unique entity IDs, and a valid `INITIAL_STATE_END` with the matching count. It builds the new world state off-screen and applies it atomically. Receiving the end marker before all expected spawns must not cause an early commit.
4. Duplicate entity IDs cannot increase the number of collected entities.
5. If any expected packet is missing, invalid, or timed out, or the collection exceeds implementation-defined memory/time limits, the client discards the incomplete candidate and retries the connection/state request (`CONNECT_REQUEST`). The server treats a retry from an already associated endpoint as a request for a **fresh complete initial transfer**, not as a request to create another player. Any new transfer supersedes the previous incomplete one.
6. To keep Part 1 deterministic, the server sends the initial transfer from a consistent simulation view and does not interleave live updates into the transfer. Live events that occur during the transfer must be reconciled after the snapshot, for example by sending the latest authoritative positions and required lifecycle events after `INITIAL_STATE_END`. If the client cannot establish a consistent view, it retries synchronization rather than showing a partial world.

**Limitations:** Delivery is retried by the network layer (§3.2) but not guaranteed: after the last retransmission the sender gives up. A missing begin marker may then cause a timeout and retry, and a missing spawn or end marker requires a full retry. Since the wire format has no transfer ID, the client must bound buffering and ignore abandoned transfer candidates. Concurrent snapshot streaming is deferred to Part 2.

### Ordering across commands

- The header carries no ordering information (§1): delayed or reordered datagrams cannot be detected from the header. Only the duplicates of a reliable message are detected (§3.2). A delayed `ENTITY_UPDATE` may briefly overwrite a more recent position; the next absolute update corrects it.
- If an update arrives before its spawn, the client may buffer it briefly and apply it after the spawn, or discard it and wait for the next authoritative update.
- During a game, entity IDs are never reused. After accepting `ENTITY_DESTROY`, a client must not resurrect the same ID due to a delayed `ENTITY_SPAWN` or `ENTITY_UPDATE`.
- `ENTITY_SPAWN` and `ENTITY_DESTROY` are reliable (§3.2), so a lost one is normally recovered by retransmission, but it may arrive late and after a later event. An unknown entity, inconsistent lifecycle, or incomplete initial transfer, for example after the sender gave up, requires resynchronization.

## 3.2. Fragmentation and reliable delivery

Both mechanisms are handled by the network layer ([doc/Network.md](../Network.md)); the game only sends and receives commands. The delays and limits below are implementation defaults, not wire constants, except the limit of 32 fragments (§1).

### Message id

The sender numbers its messages with a counter that increases by one for each message sent, whatever the command (including `ACK`, `RESEND` and `ERROR`). All the fragments of a message carry the same `Message id`. The receiver identifies a message by the pair (sender endpoint, `Message id`). The counter wraps around after 2³² messages.

### Fragmentation

Only list commands are fragmented (§1). The records are never cut: a datagram holds `floor(1183 / record_size)` records at most, and the message is split in as many datagrams as needed, in the order of its records. Datagram `i` of `N` carries `Sequence number = i` and `Sequence length = N`. An empty list is one datagram. A message needing more than 32 datagrams cannot be sent.

The receiver gives a reliable message to the game once it holds the `N` fragments, in any order of arrival: it concatenates their records by increasing `Sequence number`. A repeated fragment is ignored. Fragments of the same message that disagree on the command or on `Sequence length` are discarded with the message. At most 8 incomplete messages are kept per sender; the following ones are ignored.

The lists of the commands that are not reliable (`ENTITY_UPDATE`), `ACK` and `RESEND` are never reassembled: each of their datagrams is given to the game, or used, as soon as it is received, as a list of its own. Their records are independent of each other, so a lost fragment only loses its own records, instead of the whole message.

Example: a `ROOM_LIST` of 300 rooms, `Message id = 42`.

| Datagram | `Message id` | `Sequence number` | `Sequence length` | `Size` | Records |
|---:|---:|---:|---:|---:|---:|
| 1 | 42 | 1 | 2 | 1180 | 236 |
| 2 | 42 | 2 | 2 | 320 | 64 |

### Reliable delivery

A command flagged *reliable* in §2 is sent with the following rules. The other commands are sent once.

1. **Sender.** It keeps every datagram of the message until it is acknowledged. When no `ACK` arrived 200 ms after the last transmission, it sends all the datagrams again, unchanged. After 5 retransmissions it forgets the message without any notification.
2. **Receiver.** Once the message is complete, it gives it to the game, remembers its `Message id` (the last 1024 ids of each sender) and queues an `ACK`. The queued ids of a sender are sent together, at most one `ACK` datagram per sender each time the network layer is polled.
3. **Duplicates.** A message whose id is remembered is not given to the game again, but it is acknowledged again, since its `ACK` may have been lost. A retransmission is therefore harmless for the game, whatever the command.
4. **Missing fragments.** A reliable message that stays incomplete for 50 ms after its last new fragment is not waited for until the sender's 200 ms timer: the receiver sends a `RESEND` listing each missing fragment, and repeats it every 50 ms, 5 times at most, before discarding the message. The sender answers a `RESEND` with the requested fragments only, while it still waits for the `ACK`. The complete message is then acknowledged normally.
5. **Refused messages.** The server and the rooms only process datagrams from connected endpoints (§5), and a client only from its current host. A reliable message that is refused, for example with `ERROR NOT_CONNECTED` or `ERROR FULL`, or ignored, is not delivered and not remembered, but it is acknowledged at once, in its own `ACK` datagram, so that the sender stops retransmitting it.

The `ACK` only says that the message was received, not that it was accepted: a refusal is a separate `ERROR`. Reliability does not order messages: a retransmitted message arrives after the ones sent later (§3.1).

**Adding a command.** A new command is reliable by setting the `reliable` field of its entry in the command registry (`project/src/shared/Network/protocol.cpp`), then by adding its row to the table of §2. Nothing else changes. In the same way, a command becomes a list command with the `list` field of its entry: its handlers must then walk all the records of the list.

## 4. Example: move upward

A client controls entity `130` and presses UP (action `0x00`). Example header: message id `42`, sequence number `1`, sequence length `1` (fragment 1 of 1), size `6`.

```text
52 54             # Magic: RT
10                # PLAYER_INPUT
00 00 00 2A       # Message id = 42
00 00 00 01       # Sequence number = 1 (fragment index)
00 00 00 01       # Sequence length = 1 (fragment count)
00 06             # Payload size = 6
00 00 00 82       # Entity ID = 130
00                # Action = UP
01                # Pressed
```

Total: 23 bytes.

When the player releases UP, the client sends another `PLAYER_INPUT` packet with the same entity ID and action, but with `input_state = 0x00` (Released).

The server validates the input and updates the active input state. The game loop uses this state to calculate movement and sends authoritative `ENTITY_UPDATE` messages.

## 5. Validation and error handling

The receiver must validate **before** interpreting command payload fields:

1. Reject datagrams shorter than 17 bytes or longer than 1200 bytes.
2. Check magic bytes `52 54`.
3. Decode fields using big-endian, with bounds-checked reads.
4. Require `Size == received_bytes - 17` and `1 <= Sequence number <= Sequence length <= 32`, with `Sequence length == 1` unless the command is a list.
5. Reject unknown/reserved commands, wrong directions, and incorrect command-specific payload lengths. A valid command without handler on the receiving side is answered with `COMMAND_NOT_HANDLED` (§2).
6. Reject malformed flags and entity IDs the sender is not allowed to control.
7. Reordered and delayed UDP packets cannot be detected from the header, and the duplicates of a message are only detected for reliable commands (§3.2); apply the rules of §3 and §3.1 that do not depend on ordering. Buffering limits (incomplete messages, remembered ids) and timeout values are implementation policies, documented in §3.2.
8. Never crash or terminate the server because of malformed input; discard invalid datagrams, optionally record rate-limited diagnostics.

Do not echo invalid payloads back to the sender. A datagram rejected by rules 1–5 is answered with an `ERROR` (§3) only when it is at least 17 bytes long and carries the valid magic number, and never when the rejected command is itself `ERROR` (no error ping-pong). Shorter datagrams and datagrams with a wrong magic number are discarded silently. The 19-byte `ERROR` reply can be slightly larger than the smallest rejected datagram (17 bytes); rate limiting replies is an implementation policy.

**Connected clients.** Apart from `CONNECT_REQUEST`, the server and the rooms only process valid datagrams coming from connected endpoints (§2.1, §3). A valid datagram from an endpoint that is not connected is answered with `ERROR` `NOT_CONNECTED`, so that the client knows it must connect again (for example after a server restart), except `DISCONNECT` and `ERROR`, which are ignored silently. If its command is reliable, it is also acknowledged (§3.2).

**Client side.** A client only processes datagrams coming from its current host (§2.1) and silently ignores any other sender.

## 6. Limitations and future extension

- **Fragmentation is limited to list commands.** A message is at most 32 fragments, about 37 KB (§3.2).
- **No ordering.** `Message id` identifies a message, it does not order messages: delayed or reordered datagrams cannot be detected.
- **Delivery is retried, not guaranteed.** A reliable message is abandoned after 5 retransmissions, and nobody is told. The commands that are not reliable (§2), `PLAYER_INPUT` and `ENTITY_UPDATE`, can be lost. The effect of the loss on the game still needs measurements in Track #2.
- **No authentication or encryption.** A magic number is not a security credential. The server must validate source endpoints, permissions, and game rules.
- **No wire version field.** The `Message id` field made the header grow from 13 to 17 bytes, which is incompatible with the earlier layout. Future incompatible changes require an explicit negotiated strategy or new magic number.
- **Issue #9 baseline approved.** The fields in [ReplicatedComponents.md](ReplicatedComponents.md) are the agreed Part 1 baseline. Health, velocity, animation, entity-control reassignment, and other future synchronized data require explicit new or revised wire messages; they cannot be inferred from C++ memory layout.
- **Bandwidth optimizations** belong to Part 2, not this Part 1 protocol definition.
