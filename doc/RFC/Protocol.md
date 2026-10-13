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
- `Sequence length = Size` because one complete message is carried per datagram.
- `Sequence number` is a `uint32` datagram identifier incremented by each sender. It wraps modulo 2^32. It is not a reliable-delivery acknowledgement and is not a fragment index.
- A client-to-server packet is accepted only if its command is permitted in that direction and its connection is authorized for the referenced entity.

**Coordinates:** `x` and `y` are integer world coordinates in game units (the initial implementation may use pixel-equivalent units). Position is absolute, not a relative delta. The world origin and coordinate bounds are determined by the gameplay implementation and must be validated server-side.

**Input flags:** one byte, with bit 0 = up, bit 1 = down, bit 2 = left, bit 3 = right, bit 4 = fire; bits 5–7 must be zero. Multiple bits may be set. Conflicting opposite directions are resolved by the server (for example, cancel each other).

## 2. Command registry

| Code | Command | Direction | Payload size | Purpose |
|---|---|---|---:|---|
| `0x01` | `CONNECT_REQUEST` | Client → Server | 0 | Request a session |
| `0x02` | `CONNECT_ACCEPT` | Server → Client | 8 | Assign session and initial controlled entity |
| `0x03` | `DISCONNECT` | Both | 4 | End a session |
| `0x10` | `PLAYER_INPUT` | Client → Server | 9 | Submit input for a controlled entity |
| `0x20` | `INITIAL_STATE_BEGIN` | Server → Client | 4 | Begin an initial state transfer |
| `0x21` | `ENTITY_UPDATE` | Server → Client | 12 | Publish authoritative entity position |
| `0x22` | `ENTITY_SPAWN` | Server → Client | 13 | Create or refresh an entity representation |
| `0x23` | `ENTITY_DESTROY` | Server → Client | 4 | Remove an entity |
| `0x24` | `INITIAL_STATE_END` | Server → Client | 4 | End an initial state transfer |
| `0x30` | `ACK` | Reserved | — | Future reliable delivery; not used in Part 1 |

Unknown and reserved command codes are rejected. All payload layouts below are exact: additional bytes are invalid.

## 3. Payload layouts

Offsets in this section are relative to the **start of the payload** (immediately after byte 12 of the common header).

### `CONNECT_REQUEST` (`0x01`)

Empty payload. The server uses the UDP sender endpoint as the provisional connection identity. A UDP endpoint is not proof of identity; spoofing protection and authentication are future work.

### `CONNECT_ACCEPT` (`0x02`)

| Offset | Field | Type | Description |
|---:|---|---|---|
| 0 | `session_id` | uint32 | Server-assigned session identifier |
| 4 | `controlled_entity_id` | uint32 | Initial entity controlled by this session |

The session ID and entity ID are distinct concepts, even when the player initially controls one ship. The server owns the session-to-entity authorization mapping. Session IDs must not be treated as secret authentication tokens.

### `DISCONNECT` (`0x03`)

| Offset | Field | Type | Description |
|---:|---|---|---|
| 0 | `session_id` | uint32 | Session requesting closure or being closed |

The server verifies the source endpoint/session association before honoring a client disconnect.

### `PLAYER_INPUT` (`0x10`)

| Offset | Field | Type | Description |
|---:|---|---|---|
| 0 | `session_id` | uint32 | Issued by the server at connection |
| 4 | `entity_id` | uint32 | Entity the session intends to control |
| 8 | `input_flags` | uint8 | Movement and fire bitmask |

The server checks the session/endpoint and whether the session controls `entity_id`. Input flags are requests, never a client-provided resulting position. Input rate limiting and replay handling belong in the server implementation.

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

An update for an unknown entity must not implicitly create it. The client may defer or ignore it pending a spawn or state resynchronization. For each entity, the client tracks the most recent accepted `ENTITY_UPDATE` sequence number and ignores older or duplicate position updates for that entity (with `uint32` wraparound handled correctly). **Do not** use one global last-seen sequence number to reject every command: an `ENTITY_SPAWN` or `ENTITY_DESTROY` may arrive after a newer position update due to UDP reordering.

### `ENTITY_SPAWN` (`0x22`)

| Offset | Field | Type | Description |
|---:|---|---|---|
| 0 | `entity_id` | uint32 | Server-assigned entity ID |
| 4 | `entity_kind` | uint8 | `0x01` player ship, `0x02` enemy, `0x03` projectile; others reserved |
| 5 | `x` | int32 | Initial authoritative X coordinate |
| 9 | `y` | int32 | Initial authoritative Y coordinate |

`entity_kind` is a rendering/spawning hint, not a C++ class identity. The `entity_kind` values are the approved Part 1 wire-level baseline from issue #9; adding more kinds or ECS fields requires a documented protocol revision. A duplicate spawn for the same live entity must not create a second entity. The client should not overwrite a more recent authoritative position with an older delayed spawn.

### `ENTITY_DESTROY` (`0x23`)

| Offset | Field | Type | Description |
|---:|---|---|---|
| 0 | `entity_id` | uint32 | Entity to remove |

Unknown IDs may be ignored. The server **must not reuse an entity ID within a session** in Part 1. Clients track the latest lifecycle event sequence for each entity and must not apply a stale destroy after a newer lifecycle event. A delayed spawn for an already destroyed ID must not recreate that entity. Sequence comparison must handle `uint32` wraparound.

### `INITIAL_STATE_END` (`0x24`)

| Offset | Field | Type | Description |
|---:|---|---|---|
| 0 | `entity_count` | uint32 | Must match the begin marker and the number of collected unique entity IDs |

A matching count alone does **not** prove the transfer is complete. The ordered-sequence and retry rules in §3.1 apply. Part 1 does not implement reliable snapshot transfer; explicit transfer IDs, retransmission of individual fragments, and robust recovery are Part 2 candidates.

## 3.1. Initial-state transfer and UDP reordering (Part 1)

The existing header and payloads remain unchanged. To make the initial transfer checkable without adding a transfer ID, Part 1 uses the following **sender-side convention**:

1. For one joining client, the server sends `INITIAL_STATE_BEGIN`, exactly `entity_count` `ENTITY_SPAWN` packets, and `INITIAL_STATE_END` **consecutively in that client-directed sequence stream**. The server must not interleave other commands in this sequence range. The server assigns consecutive `Sequence number` values to these packets, wrapping modulo `2^32` if necessary.
2. The client identifies a candidate transfer by its `INITIAL_STATE_BEGIN` sequence number `S`. It buffers the following packets by their sequence number, **not by UDP arrival order**. A spawn for this transfer must have a sequence number in the contiguous range `S + 1` through `S + entity_count` (modulo `2^32`); the end marker must have sequence number `S + entity_count + 1`.
3. The client commits the initial state **only** after receiving the begin marker, all expected sequence slots containing valid `ENTITY_SPAWN` commands with unique entity IDs, and a valid `INITIAL_STATE_END` with the matching count. It builds the new world state off-screen and applies it atomically. Receiving the end marker first, or receiving packets out of order, must not cause an early commit.
4. Packets arriving before their begin marker may be held temporarily in a **bounded** pre-begin buffer; otherwise they are discarded. A client must never interpret an arbitrary `ENTITY_SPAWN` as part of a snapshot solely because it arrived nearby in time. Duplicate sequence numbers or duplicate entity IDs cannot increase the number of collected entities.
5. If any expected packet is missing, invalid, or timed out, or the collection exceeds implementation-defined memory/time limits, the client discards the incomplete candidate and retries the connection/state request (`CONNECT_REQUEST`). The server treats a retry from an already associated endpoint as a request for a **fresh complete initial transfer**, not as a request to create another player. Any new transfer supersedes the previous incomplete one.
6. To keep Part 1 deterministic, the server sends the initial transfer from a consistent simulation view and does not interleave live updates into its sequence range. Live events that occur during the transfer must be reconciled after the snapshot, for example by sending the latest authoritative positions and required lifecycle events after `INITIAL_STATE_END`. If the client cannot establish a consistent view, it retries synchronization rather than showing a partial world.

**Limitations:** This is a minimal Part 1 convention, not guaranteed delivery. A missing begin marker may cause a timeout and retry. A missing spawn or end marker also requires a full retry. Since the wire format has no transfer ID, the client must bound buffering and ignore abandoned transfer candidates. Full reliability and more sophisticated concurrent snapshot streaming are deferred to Part 2.

### Sequence-number handling across commands

- `Sequence number` is a per-sender/session datagram counter, not an acknowledgement. A server must use a consistent outgoing counter for each client-directed stream. Two numbers `a` and `b` are compared modulo `2^32`: `a` is newer than `b` when `0 < (a - b) mod 2^32 < 2^31`; comparisons outside this window are ambiguous and require resynchronization.
- Track position freshness **per entity** for `ENTITY_UPDATE`. A newer update for entity A must not invalidate a delayed lifecycle event for entity B.
- Track entity lifecycle ordering independently of position updates. `ENTITY_SPAWN` and `ENTITY_DESTROY` are not rejected merely because a newer `ENTITY_UPDATE` arrived first. If an update arrives before its spawn, the client may buffer it briefly and apply it after spawn if it is newer, or discard it and wait for the next authoritative update.
- Within one session, entity IDs are never reused. After accepting `ENTITY_DESTROY`, a client must not resurrect the same ID due to a delayed `ENTITY_SPAWN` or `ENTITY_UPDATE`.
- Packet sequencing alone does **not** recover lost spawn/destroy events. An unknown entity, inconsistent lifecycle, or incomplete initial transfer requires resynchronization. Part 1 does not claim reliable delivery.

## 4. Example: move upward

A client controls entity `130` under session `7` and presses UP (`0x01`). Example header: sequence number `1`, sequence length `9`, size `9`.

```text
52 54             # Magic: RT
10                # PLAYER_INPUT
00 00 00 01       # Sequence number = 1
00 00 00 09       # Sequence length = 9
00 09             # Payload size = 9
00 00 00 07       # Session ID = 7
00 00 00 82       # Entity ID = 130
01                # UP flag
```

Total: 22 bytes. The server validates the input and later sends an `ENTITY_UPDATE` with the authoritative position; it does not blindly forward a client-computed displacement.

## 5. Validation and error handling

The receiver must validate **before** interpreting command payload fields:

1. Reject datagrams shorter than 13 bytes or longer than 1200 bytes.
2. Check magic bytes `52 54`.
3. Decode fields using big-endian, with bounds-checked reads.
4. Require `Size == received_bytes - 13` and `Sequence length == Size` in Part 1.
5. Reject unknown/reserved commands, wrong directions, and incorrect command-specific payload lengths.
6. Reject malformed flags and unauthorized session/entity combinations.
7. Handle duplicate, delayed, and reordered UDP packets according to §3.1. Do not apply a single global last-seen sequence filter to all message types. The replay window, buffering limits, and timeout values are implementation policies and must be documented when implemented.
8. Never crash or terminate the server because of malformed input; discard invalid datagrams, optionally record rate-limited diagnostics.

Do not echo arbitrary invalid payloads back to the sender. The protocol currently has no dedicated error-response command.

## 6. Limitations and future extension

- **No fragmentation in Part 1.** Eric's original `Sequence number` and `Sequence length` header fields are preserved, but the future fragmentation encoding and reassembly algorithm are not yet defined.
- **No guaranteed delivery in Part 1.** UDP loss can affect connection acceptance, spawns, destroys, and initial state. A complete production implementation requires retries, recovery snapshots, or reliable messaging, with measurements in Track #2.
- **No authentication or encryption.** A magic number and session ID are not security credentials. The server must validate source endpoints, permissions, and game rules.
- **No wire version field.** The header is frozen for compatibility with Eric's design. Future incompatible changes require an explicit negotiated strategy or new magic number.
- **Issue #9 baseline approved.** The fields in [ReplicatedComponents.md](ReplicatedComponents.md) are the agreed Part 1 baseline. Health, velocity, animation, entity-control reassignment, and other future synchronized data require explicit new or revised wire messages; they cannot be inferred from C++ memory layout.
- **Lobby/multi-instance and bandwidth optimizations** belong to Part 2, not this Part 1 protocol definition.
