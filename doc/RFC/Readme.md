# R-Type — Network protocol (RFC)

This document describes the binary UDP frames exchanged by the R-Type client and server. The server is authoritative: clients send input intentions; the server validates them, advances the simulation, and publishes entity state.

## Frame layout

The existing frame layout defined by the server team is retained unchanged:

```text
+---------------+---------+-----------------+-----------------+--------+----------------+
| Magic number  | Command | Sequence number | Sequence length | Size   | Message        |
| 2 bytes       | 1 byte  | 4 bytes         | 4 bytes         | 2 bytes| Size bytes     |
+---------------+---------+-----------------+-----------------+--------+----------------+
```

| Offset | Field | Type | Meaning |
|---:|---|---|---|
| 0 | Magic number | uint16 | `0x5254` (ASCII `RT`), used to identify R-Type frames |
| 2 | Command | uint8 | Message type (see [Protocol.md](Protocol.md)) |
| 3 | Sequence number | uint32 | Sequence identifier; in Part 1, a monotonically increasing datagram identifier per sender/session |
| 7 | Sequence length | uint32 | Total byte length of the logical message; in Part 1 this equals `Size` |
| 11 | Size | uint16 | Number of payload bytes in this datagram |
| 13 | Message | byte[Size] | Binary command payload |

All multi-byte integers use **network byte order (big-endian)**. There is no padding or null terminator between fields. The header is exactly **13 bytes**. The total datagram length must equal `13 + Size`.

For Part 1, application-level fragmentation and reassembly are **not supported**. Each datagram contains one complete logical message; therefore `Sequence length == Size`. The field names and widths are preserved for future work on fragmentation in Part 2. Future fragmentation semantics must be specified and versioned before implementation; this RFC does not assume that a fragment number is encoded in the existing header.

The maximum accepted UDP datagram is **1200 bytes** (13-byte header + up to **1187 payload bytes**). This is an application limit intended to reduce IP fragmentation risk, not a guarantee that all networks avoid it.

The magic number is a format marker, **not** authentication or an anti-cheat mechanism.

## Design rules

- Entity identifiers are `uint32`, assigned by the server. An entity ID identifies a controllable or simulated entity, not a hardcoded ship or player class.
- The server associates a connection with the entity IDs that connection is allowed to control. The server never trusts an entity ID provided by the client on its own.
- The client sends inputs, not authoritative positions. The server sends initial entity state and subsequent absolute-state updates, allowing clients to recover from lost updates.
- UDP packets may be lost, duplicated, or reordered. This RFC defines validation and message formats, **not reliable delivery**. Critical events and initial synchronization need recovery or retransmission before production use.
- Reserved command codes must not be treated as implemented features.

See [Protocol.md](Protocol.md) for command codes, payload formats, examples, and validation.
