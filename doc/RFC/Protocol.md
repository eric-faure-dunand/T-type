# R-Type — Binary UDP protocol specification

**Status:** Proposed Part 1 protocol, issue #19 ([2.1]). This is a wire-format contract for implementation, not a claim that the server/client already implement these messages.

**Dependencies:** The network team has approved the Part 1 replicated-data baseline from issue #9, documented in [ReplicatedComponents.md](ReplicatedComponents.md). The payloads below implement that baseline. Additional synchronized components require an explicit protocol revision.

See [ReplicatedComponents.md](ReplicatedComponents.md)
for the list of synchronized game data and update frequencies.

## 1. Common encoding

Every UDP datagram has the 13-byte header documented in [Readme.md](Readme.md). All integers are unsigned and big-endian unless stated otherwise. Signed coordinates are encoded as **signed 32-bit two's-complement integers** in big-endian order. No compiler-native structs, padding, pointers, or C++ object layouts are transmitted.

Part 1 constraints:

- `Magic = 0x5254`.
- `13 <= datagram_size <= 1200`.
- `Size = datagram_size - 13`, maximum 1187.
- `Sequence number = 1` and `Sequence length = 1` because one complete message is carried per datagram (fragment 1 of 1).
- `Sequence number` is the 1-based fragment index and `Sequence length` the total fragment count of the logical message. Neither is a datagram counter nor an acknowledgement: the header carries no ordering information.
- Each payload carries exactly **one record** laid out by the command's fixed schema in §3. Carrying a list of records in one logical message is future work.
- A client-to-server packet is accepted only if its command is permitted in that direction and its connection is authorized for the referenced entity.

**Coordinates:** `x` and `y` are integer world coordinates in game units (the initial implementation may use pixel-equivalent units). Position is absolute, not a relative delta. The world origin and coordinate bounds are determined by the gameplay implementation and must be validated server-side.

**Input flags:** one byte, with bit 0 = up, bit 1 = down, bit 2 = left, bit 3 = right, bit 4 = fire; bits 5–7 must be zero. Multiple bits may be set. Conflicting opposite directions are resolved by the server (for example, cancel each other).

## 2. Command registry

| Code | Command | Direction | Size | Purpose |
|---|---|---|---:|---|
| `0x01` | `CONNECT_REQUEST` | Client → Server | 0 | Ask the server to connect |
| `0x02` | `CONNECT_ACCEPT` | Server → Client | 0 | Accept the connection |
| `0x03` | `DISCONNECT` | Both | 0 | End the connection |
| `0x10` | `PLAYER_INPUT` | Client → Server | 5 | Submit input for a controlled entity |
| `0x20` | `INITIAL_STATE_BEGIN` | Server → Client | 4 | Begin an initial state transfer |
| `0x21` | `ENTITY_UPDATE` | Server → Client | 12 | Publish authoritative entity position |
| `0x22` | `ENTITY_SPAWN` | Server → Client | 13 | Create or refresh an entity representation |
| `0x23` | `ENTITY_DESTROY` | Server → Client | 4 | Remove an entity |
| `0x24` | `INITIAL_STATE_END` | Server → Client | 4 | End an initial state transfer |
| `0x30` | `ACK` | Reserved | — | Future reliable delivery; not used in Part 1 |
| `0x84` | `ERROR` | Both | 2 | Report a rejected datagram to its sender |

Unknown and reserved command codes are rejected. All payload layouts below are exact: additional bytes are invalid.

## 3. Payload layouts

Offsets in this section are relative to the **start of the payload** (immediately after byte 12 of the common header).

### `CONNECT_REQUEST` (`0x01`)

Empty payload. The client asks the server whether it may connect. The server adds the sender endpoint to its list of connected clients and answers `CONNECT_ACCEPT`. A request from an endpoint that is already connected is answered again (e.g. when the previous `CONNECT_ACCEPT` was lost) without adding it twice. A UDP endpoint is not proof of identity; spoofing protection and authentication are future work.

### `CONNECT_ACCEPT` (`0x02`)

Empty payload. The server accepts the connection. The client considers itself connected only when `CONNECT_ACCEPT` comes from the server it asked. Nothing else is assigned: no entity, no room.

> **TODO:** room creation / join commands will be added later; the ids they need will be defined there.

### `DISCONNECT` (`0x03`)

Empty payload. Ends the connection, from either side, and is never answered.

- Sent by a client: the server removes the sender from its list of connected clients. A `DISCONNECT` from an endpoint that is not connected is ignored.
- Sent by the server: the server removes the client from its list, and the client considers itself disconnected. A client ignores a `DISCONNECT` that does not come from its server.

> **TODO:** a `room_id` will be added with room creation / join; clients will send `0`.

### `PLAYER_INPUT` (`0x10`)

| Offset | Field | Type | Description |
|---:|---|---|---|
| 0 | `entity_id` | uint32 | Entity the client intends to control |
| 4 | `input_flags` | uint8 | Movement and fire bitmask |

The server checks that the sender is allowed to control `entity_id`. Input flags are requests, never a client-provided resulting position. Input rate limiting and replay handling belong in the server implementation.

### `INITIAL_STATE_BEGIN` (`0x20`)

| Offset | Field | Type | Description |
|---:|---|---|---|
| 0 | `entity_count` | uint32 | Number of `ENTITY_SPAWN` messages expected in this transfer |

On receipt, the client starts a fresh initial-state collection. The server sends one `ENTITY_SPAWN` for each existing entity, then `INITIAL_STATE_END`. Because UDP can lose messages, a client **must not** treat a partial transfer as a complete snapshot. If the count is not reached or the end marker is missing, the client should request a new connection/state transfer after a timeout; timeout values are implementation policy.

### `ENTITY_UPDATE` (`0x21`)

| Offset | Field | Type | Description |
|---:|---|---|---|
| 0 | `entity_id` | uint32 | Existing entity identifier |
| 4 | `x` | int32 | Authoritative absolute X coordinate |
| 8 | `y` | int32 | Authoritative absolute Y coordinate |

An update for an unknown entity must not implicitly create it. The client may defer or ignore it pending a spawn or state resynchronization. Part 1 carries no ordering information (§1): a delayed update may overwrite a more recent position. Positions are absolute and sent at a target 20 Hz, so the next update corrects it.

### `ENTITY_SPAWN` (`0x22`)

| Offset | Field | Type | Description |
|---:|---|---|---|
| 0 | `entity_id` | uint32 | Server-assigned entity ID |
| 4 | `entity_kind` | uint8 | `0x01` player ship, `0x02` enemy, `0x03` projectile; others reserved |
| 5 | `x` | int32 | Initial authoritative X coordinate |
| 9 | `y` | int32 | Initial authoritative Y coordinate |

`entity_kind` is a rendering/spawning hint, not a C++ class identity. The `entity_kind` values are the approved Part 1 wire-level baseline from issue #9; adding more kinds or ECS fields requires a documented protocol revision. A duplicate spawn for the same live entity must not create a second entity.

### `ENTITY_DESTROY` (`0x23`)

| Offset | Field | Type | Description |
|---:|---|---|---|
| 0 | `entity_id` | uint32 | Entity to remove |

Unknown IDs may be ignored. The server **must not reuse an entity ID during a game** in Part 1. A delayed spawn for an already destroyed ID must not recreate that entity.

### `INITIAL_STATE_END` (`0x24`)

| Offset | Field | Type | Description |
|---:|---|---|---|
| 0 | `entity_count` | uint32 | Must match the begin marker and the number of collected unique entity IDs |

A matching count alone does **not** prove the transfer is complete. The rules in §3.1 apply. Part 1 does not implement reliable snapshot transfer; explicit transfer IDs, retransmission of individual fragments, and robust recovery are Part 2 candidates.

### `ERROR` (`0x84`)

| Offset | Field | Type | Description |
|---:|---|---|---|
| 0 | `error_code` | uint8 | Reason of the rejection, see the table below |
| 1 | `rejected_command` | uint8 | Command byte of the rejected datagram, `0x00` when it could not be read |

Sent by either side in response to a rejected datagram, following the rules of §5. Its header uses `Sequence number = 1` and `Sequence length = 1` like any Part 1 datagram. A received `ERROR` is informational and is never answered with another `ERROR`.

| Code | Name | Rejection |
|---|---|---|
| `0x01` | `DATAGRAM_TOO_SHORT` | Fewer than 13 bytes (never sent, local diagnostic only) |
| `0x02` | `DATAGRAM_TOO_LONG` | More than 1200 bytes |
| `0x03` | `BAD_MAGIC` | Magic number is not `0x5254` (never sent, local diagnostic only) |
| `0x04` | `SIZE_MISMATCH` | `Size != received_bytes - 13` |
| `0x05` | `FRAGMENTATION_UNSUPPORTED` | `Sequence number != 1` or `Sequence length != 1` |
| `0x06` | `UNKNOWN_COMMAND` | Command code not in the registry |
| `0x07` | `RESERVED_COMMAND` | Reserved command code (`ACK`) |
| `0x08` | `WRONG_DIRECTION` | Command not allowed in this direction |
| `0x09` | `BAD_PAYLOAD_SIZE` | Payload size does not match the command schema |
| `0x0A` | `NOT_CONNECTED` | Valid datagram sent to the server by an endpoint that is not connected (see §5) |

Error codes `0x0B`–`0xFF` are reserved for future revisions.

## 3.1. Initial-state transfer and UDP reordering (Part 1)

The existing header and payloads remain unchanged. To make the initial transfer checkable without adding a transfer ID, Part 1 uses the following **sender-side convention**:

1. For one joining client, the server sends `INITIAL_STATE_BEGIN`, exactly `entity_count` `ENTITY_SPAWN` packets, and `INITIAL_STATE_END` **consecutively** to that client. The server must not interleave other commands for that client during the transfer.
2. **Ordering is not guaranteed in Part 1.** The header carries no datagram counter (§1), so the client cannot detect that packets of a transfer were reordered. This will be revisited together with fragmentation.
3. The client commits the initial state **only** after receiving the begin marker, `entity_count` valid `ENTITY_SPAWN` commands with unique entity IDs, and a valid `INITIAL_STATE_END` with the matching count. It builds the new world state off-screen and applies it atomically. Receiving the end marker before all expected spawns must not cause an early commit.
4. Duplicate entity IDs cannot increase the number of collected entities.
5. If any expected packet is missing, invalid, or timed out, or the collection exceeds implementation-defined memory/time limits, the client discards the incomplete candidate and retries the connection/state request (`CONNECT_REQUEST`). The server treats a retry from an already associated endpoint as a request for a **fresh complete initial transfer**, not as a request to create another player. Any new transfer supersedes the previous incomplete one.
6. To keep Part 1 deterministic, the server sends the initial transfer from a consistent simulation view and does not interleave live updates into the transfer. Live events that occur during the transfer must be reconciled after the snapshot, for example by sending the latest authoritative positions and required lifecycle events after `INITIAL_STATE_END`. If the client cannot establish a consistent view, it retries synchronization rather than showing a partial world.

**Limitations:** This is a minimal Part 1 convention, not guaranteed delivery. A missing begin marker may cause a timeout and retry. A missing spawn or end marker also requires a full retry. Since the wire format has no transfer ID, the client must bound buffering and ignore abandoned transfer candidates. Full reliability and more sophisticated concurrent snapshot streaming are deferred to Part 2.

### Ordering across commands

- The Part 1 header carries no ordering information (§1): duplicate, delayed, or reordered datagrams cannot be detected from the header. A delayed `ENTITY_UPDATE` may briefly overwrite a more recent position; the next absolute update corrects it.
- If an update arrives before its spawn, the client may buffer it briefly and apply it after the spawn, or discard it and wait for the next authoritative update.
- During a game, entity IDs are never reused. After accepting `ENTITY_DESTROY`, a client must not resurrect the same ID due to a delayed `ENTITY_SPAWN` or `ENTITY_UPDATE`.
- Lost spawn/destroy events are **not** recovered. An unknown entity, inconsistent lifecycle, or incomplete initial transfer requires resynchronization. Part 1 does not claim reliable delivery.

## 4. Example: move upward

A client controls entity `130` and presses UP (`0x01`). Example header: sequence number `1`, sequence length `1` (fragment 1 of 1), size `5`.

```text
52 54             # Magic: RT
10                # PLAYER_INPUT
00 00 00 01       # Sequence number = 1 (fragment index)
00 00 00 01       # Sequence length = 1 (fragment count)
00 05             # Payload size = 5
00 00 00 82       # Entity ID = 130
01                # UP flag
```

Total: 18 bytes. The server validates the input and later sends an `ENTITY_UPDATE` with the authoritative position; it does not blindly forward a client-computed displacement.

## 5. Validation and error handling

The receiver must validate **before** interpreting command payload fields:

1. Reject datagrams shorter than 13 bytes or longer than 1200 bytes.
2. Check magic bytes `52 54`.
3. Decode fields using big-endian, with bounds-checked reads.
4. Require `Size == received_bytes - 13`, and `Sequence number == 1` and `Sequence length == 1` in Part 1.
5. Reject unknown/reserved commands, wrong directions, and incorrect command-specific payload lengths.
6. Reject malformed flags and entity IDs the sender is not allowed to control.
7. Duplicate, delayed, and reordered UDP packets cannot be detected from the Part 1 header; apply the rules of §3 and §3.1 that do not depend on ordering. Buffering limits and timeout values are implementation policies and must be documented when implemented.
8. Never crash or terminate the server because of malformed input; discard invalid datagrams, optionally record rate-limited diagnostics.

Do not echo invalid payloads back to the sender. A datagram rejected by rules 1–5 is answered with an `ERROR` (§3) only when it is at least 13 bytes long and carries the valid magic number, and never when the rejected command is itself `ERROR` (no error ping-pong). Shorter datagrams and datagrams with a wrong magic number are discarded silently. The 15-byte `ERROR` reply can be slightly larger than the smallest rejected datagram (13 bytes); rate limiting replies is an implementation policy.

**Connected clients.** Apart from `CONNECT_REQUEST`, the server only processes valid datagrams coming from connected endpoints (§3, `CONNECT_REQUEST` / `DISCONNECT`). A valid datagram from an endpoint that is not connected is answered with `ERROR` `NOT_CONNECTED`, so that the client knows it must connect again (for example after a server restart), except `DISCONNECT` and `ERROR`, which are ignored silently.

## 6. Limitations and future extension

- **No fragmentation in Part 1.** `Sequence number` (fragment index) and `Sequence length` (fragment count) are fixed to `1`; the fragmentation encoding and reassembly algorithm are not yet defined.
- **No ordering information in Part 1.** The header has no datagram counter, so duplicate, delayed, or reordered datagrams cannot be detected.
- **No guaranteed delivery in Part 1.** UDP loss can affect connection acceptance, spawns, destroys, and initial state. A complete production implementation requires retries, recovery snapshots, or reliable messaging, with measurements in Track #2.
- **No authentication or encryption.** A magic number is not a security credential. The server must validate source endpoints, permissions, and game rules.
- **No wire version field.** The header is frozen for compatibility with Eric's design. Future incompatible changes require an explicit negotiated strategy or new magic number.
- **Issue #9 baseline approved.** The fields in [ReplicatedComponents.md](ReplicatedComponents.md) are the agreed Part 1 baseline. Health, velocity, animation, entity-control reassignment, and other future synchronized data require explicit new or revised wire messages; they cannot be inferred from C++ memory layout.
- **Lobby/multi-instance and bandwidth optimizations** belong to Part 2, not this Part 1 protocol definition.
