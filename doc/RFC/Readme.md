# R-Type — Network protocol (RFC)

This document describes the binary UDP frames exchanged by the R-Type client and server. The server is authoritative: clients send input intentions; the server validates them, advances the simulation, and publishes entity state.

## Frame layout

```text
+---------------+---------+------------+-----------------+-----------------+--------+----------------+
| Magic number  | Command | Message id | Sequence number | Sequence length | Size   | Message        |
| 2 bytes       | 1 byte  | 4 bytes    | 4 bytes         | 4 bytes         | 2 bytes| Size bytes     |
+---------------+---------+------------+-----------------+-----------------+--------+----------------+
```

| Offset | Field | Type | Meaning |
|---:|---|---|---|
| 0 | Magic number | uint16 | `0x5254` (ASCII `RT`), used to identify R-Type frames |
| 2 | Command | uint8 | Message type (see [Protocol.md](Protocol.md)) |
| 3 | Message id | uint32 | Identifier of the logical message, chosen by the sender and shared by all its fragments; used for acknowledgements and retransmission requests |
| 7 | Sequence number | uint32 | Fragment index of this datagram within its logical message, starting at 1; `1` for a message that is not fragmented |
| 11 | Sequence length | uint32 | Total number of fragments of the logical message; `1` for a message that is not fragmented |
| 15 | Size | uint16 | Number of payload bytes in this datagram |
| 17 | Message | byte[Size] | Binary command payload |

All multi-byte integers use **network byte order (big-endian)**. There is no padding or null terminator between fields. The header is exactly **17 bytes**. The total datagram length must equal `17 + Size`.

A message that fits in one datagram is sent with `Sequence number == 1` and `Sequence length == 1` (fragment 1 of 1). A list command whose records do not fit in one datagram is split into several datagrams, each one carrying the same `Message id` and the fragment `i` of `N` in `Sequence number` and `Sequence length`. The fragments of a reliable list are reassembled; those of a list that is not reliable are used one by one. The fragmentation and reassembly rules are in [Protocol.md §3.2](Protocol.md#32-fragmentation-and-reliable-delivery). `Message id` only identifies a message: the header carries **no ordering information**.

The maximum accepted UDP datagram is **1200 bytes** (17-byte header + up to **1183 payload bytes**). This is an application limit intended to reduce IP fragmentation risk, not a guarantee that all networks avoid it.

The magic number is a format marker, **not** authentication or an anti-cheat mechanism.

## Design rules

- Entity identifiers are `uint32`, assigned by the server. An entity ID identifies a controllable or simulated entity, not a hardcoded ship or player class.
- The server associates a connection with the entity IDs that connection is allowed to control. The server never trusts an entity ID provided by the client on its own.
- The client sends inputs, not authoritative positions. The server sends initial entity state and subsequent absolute-state updates, allowing clients to recover from lost updates.
- UDP packets may be lost, duplicated, or reordered. The commands flagged *reliable* in [Protocol.md §2](Protocol.md#2-command-registry) are acknowledged and retransmitted, and a reliable message received twice is delivered once (§3.2). The other commands are sent once and may be lost. Reordering is never corrected.
- Reserved command codes must not be treated as implemented features.
- A rejected datagram may be answered with an `ERROR` command carrying an error code; the rejected payload is never echoed back.
- The server listens on one port for the lobby (connection, room list, room creation and join); each room listens on its own port for the game. A client leaves a room with `DISCONNECT_ROOM` and the server with `DISCONNECT`. See [Protocol.md §2.1](Protocol.md#21-server-and-room-sockets).
- A valid command for which the receiver has no handler is answered with `ERROR COMMAND_NOT_HANDLED`.

See [Protocol.md](Protocol.md) for command codes, payload formats, examples, and validation, and [doc/Network.md](../Network.md) for the implementation of the network layer.
