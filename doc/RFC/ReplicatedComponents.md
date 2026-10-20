# R-Type — Replicated game data (Issue #9)

**Status:** Proposed network-team baseline for Part 1. Approved by the network team before closing issue #9.

This document specifies which game data crosses the client/server boundary. It describes the **wire-level representation**, not mandatory C++ ECS component class names. The server is authoritative; the client sends input intentions, not simulation results.

## Replicated entity data

| Logical data / ECS concern | Wire fields | Direction | When / target rate | Part 1 wire message |
|---|---|---|---|---|
| Network identity | `entity_id: uint32` | Server → client | On spawn, update and destruction | `ENTITY_SPAWN`, `ENTITY_UPDATE`, `ENTITY_DESTROY` |
| Entity kind / rendering hint | `entity_kind: uint8` | Server → client | On spawn and initial sync | `ENTITY_SPAWN` |
| Position / Transform | `x: int32`, `y: int32` (absolute world coordinates) | Server → client | Target **20 Hz** for active moving entities; initial state on join | `ENTITY_UPDATE`, `ENTITY_SPAWN` |
| Controlled entity association | `controlled_entity_id: uint32` | Server → owning client | Not assigned on connection; to be defined with room creation / join | — |
| Input intention | `entity_id: uint32`, `action: uint8`, `input_state: uint8` (one action per packet) | Client → server | Target **30 Hz max**, and on input changes | `PLAYER_INPUT` |
| Entity existence | `entity_id: uint32` | Server → client | On creation and destruction | `ENTITY_SPAWN`, `ENTITY_DESTROY` |
| Initial entity set | `entity_count: uint32`, then one spawn per entity | Server → joining client | Once per initial sync/retry | `INITIAL_STATE_BEGIN`, `ENTITY_SPAWN`, `INITIAL_STATE_END` |

**Encoding:** All multibyte integers are big-endian. Entity IDs are assigned by the server. Positions use signed 32-bit integer world units, matching the existing Issue #19 RFC; they are **not** `float32`. A client may interpolate locally for rendering, but interpolation state is not authoritative and is not transmitted.

## Part 1 data not replicated as standalone components

- **Velocity:** Derived locally from successive authoritative positions when useful for interpolation. No velocity field in the current `ENTITY_UPDATE` message.
- **Health:** May be maintained by the authoritative game simulation, but is **not yet represented** in the current Part 1 protocol. If health bars or damage state must be displayed, issue #19 needs an explicit versioned message or payload extension before implementation. Do not silently append health fields to existing messages.
- **Collision geometry and collision results:** The server owns collision detection; clients only receive resulting authoritative state and spawn/destruction events.
- **AI, cooldowns, physics internals, server ownership tables:** Server-side only. Clients must not send or authoritatively set these values.
- **Sprite assets, animations, audio:** Local client resources chosen using `entity_kind`; asset bytes are not sent over UDP.

## Ownership and validation

A player initially controls a ship entity, but the network model allows the server to authorize control of another entity type later. The server must verify that the sender is allowed to control the entity before processing input. The client cannot directly assign position, health, or entity creation/destruction.

## Synchronization policy

- **Input:** Send one `PLAYER_INPUT` per action on change, and repeat held actions periodically (every 100 ms) and each release a few times. A lost input packet must not leave movement stuck forever; server-side input expiry/timeout is required in the implementation.
- **Movement:** Send latest absolute positions at a target 20 Hz, rather than only relative deltas, to recover from lost UDP updates. Tune rates after bandwidth/latency measurements.
- **Spawn/destroy:** Send when events occur. UDP is not reliable; implementation must provide event recovery or resynchronization. This document does not claim reliable delivery is implemented.
- **Initial sync:** Send `INITIAL_STATE_BEGIN`, one `ENTITY_SPAWN` per entity, and `INITIAL_STATE_END`. Validate expected entity count. Missing packets require a retry/resync; a partial initial state must not be treated as complete.
- **Packet size:** Preserve the existing 13-byte header and 1200-byte maximum UDP datagram defined by issue #19. No application-level fragmentation in Part 1.

## Dependencies and follow-up

This page is the proposed baseline for issue #9 and should be reviewed by the network team and gameplay/ECS team. Issue #19 currently supports the fields in the first table. **Before implementing health UI, entity control reassignment, or richer ECS replication**, define corresponding messages in issue #19 and document their exact binary layouts. Issue #24 should use the rates and authoritative ownership rules above as its starting point, then validate them with measurements.
