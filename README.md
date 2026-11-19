# R-Type

A multiplayer space shooter developed in C++ as part of the EPITECH Advanced C++ project.

The project follows a client-server architecture, with an authoritative game server responsible for multiplayer communication and game state management.

It includes a custom UDP networking protocol with authenticated encryption, compression and secure session management.

## Features

### Multiplayer

- Client-server architecture
- UDP-based communication
- Multiplayer lobby and room management
- Support for multiple connected clients
- Server-authoritative networking

### Secure networking

- LZ4 payload compression
- AES-256-GCM authenticated encryption
- X25519 key exchange
- HKDF session key derivation
- Ed25519 server authentication
- Secure client-server handshake
- Independent encrypted sessions
- Replay attack protection

### Game architecture

- Modular C++ implementation
- Separation between client, server and shared components
- Binary network protocol
- Automated tests for networking and security components

## Requirements

The project is built using CMake and requires a compatible C++ development environment.

The secure networking implementation uses OpenSSL and LZ4. Other project dependencies are managed through the existing CMake configuration.

## Build

Clone the repository and navigate to its root directory.

Configure the project:

```bash
cmake -S . -B build
```

Compile:

```bash
cmake --build build -j4
```

Run the commands below from the repository root.

## Quick start

### 1. Generate the server keys

Create the local key directory:

```bash
mkdir -p .local/keys
```

Generate an Ed25519 private key:

```bash
openssl genpkey -algorithm Ed25519 \
    -out .local/keys/server-private.pem
```

Generate the corresponding public key:

```bash
openssl pkey \
    -in .local/keys/server-private.pem \
    -pubout \
    -out .local/keys/server-public.pem
```

Restrict private key permissions:

```bash
chmod 600 .local/keys/server-private.pem
```

Never commit or share the server private key.

### 2. Start the server

```bash
./r-type_server
```

The server loads its private key from `.local/keys/server-private.pem` by default.

### 3. Start a client

Open a second terminal:

```bash
./r-type_client 127.0.0.1
```

Connect to `127.0.0.1` from the game interface.

Controls: arrows or WASD (ZQSD on a French keyboard) to move, SPACE to fire, F to switch auto-fire, ESC to leave. Press O in the menu or in the room list for the accessibility options.

The client loads the trusted server public key from `.local/keys/server-public.pem`.

### 4. Connect additional clients

Launch another client in a separate terminal:

```bash
./r-type_client 127.0.0.1
```

Each client establishes its own secure session with the server.

For remote connections, use the server's network IP address and install the corresponding trusted public key on each client computer.

See the secure networking setup guide for detailed instructions.

## Security configuration

Default key paths:

| File | Purpose |
|---|---|
| `.local/keys/server-private.pem` | Server identity private key |
| `.local/keys/server-public.pem` | Trusted server public key |

Custom paths can be configured using environment variables:

- `RTYPE_SERVER_PRIVATE_KEY`
- `RTYPE_SERVER_PUBLIC_KEY`

For example:

```bash
RTYPE_SERVER_PRIVATE_KEY=/path/to/private.pem ./r-type_server
```

The default paths are relative to the current working directory.

## Run tests

Execute the automated test suite:

```bash
ctest --test-dir build --output-on-failure
```

The secure networking integration was validated with 113 passing automated tests and manual local multiplayer connections.

## Project structure

```text
.
├── CMakeLists.txt
├── project/
│   ├── include/
│   │   ├── client/
│   │   ├── server/
│   │   └── shared/
│   ├── src/
│   │   ├── client/
│   │   ├── server/
│   │   └── shared/
│   └── tests/
├── doc/
└── README.md
```

The project separates client-specific, server-specific and shared networking functionality.

## Documentation

Additional documentation is available in the `doc/` directory.

- [Accessibility](doc/Accessibility.md) — What the game offers to players with motor, visual, hearing or cognitive disabilities, and how to use it.
- [Secure Networking Setup](doc/SECURE_NETWORK_SETUP.md) — Key generation, server and client setup, multiplayer testing and troubleshooting.
- [Secure Networking Architecture](doc/SECURE_NETWORK_ARCHITECTURE.md) — Cryptographic design, handshake, secure sessions, datagram protection and replay prevention.

## Development guidelines

Contributors should follow the existing project conventions:

- Keep implementations modular.
- Use clear, descriptive names.
- Separate declarations and implementations into `.hpp` and `.cpp` files.
- Keep functions focused on a single responsibility.
- Preserve compatibility with the existing networking protocol.
- Add or update tests when changing network or security behavior.
- Run the test suite before opening a Pull Request.

## Security notes

The server private key must never be committed to the repository.

Clients must verify the server identity using a trusted public key.

The secure networking layer provides confidentiality, integrity and authentication for protected network communication. It does not replace gameplay validation, application authorization or UDP reliability mechanisms.

## Authors

Developed as an EPITECH team project.
