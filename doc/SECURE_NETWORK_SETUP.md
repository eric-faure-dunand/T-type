# Secure UDP Networking — Setup and Testing Guide

## Overview

The R-Type project uses an authenticated and encrypted UDP transport for client-server communication.

This guide explains how to configure the server identity, build the project, launch the server and clients, and verify local multiplayer connectivity.

## 1. Requirements

- A Linux development environment
- CMake and a compatible C++ compiler
- The project dependencies configured through CMake
- OpenSSL development libraries
- Access to the project repository

## 2. Build the project

From the repository root:

```bash
cmake -S . -B build
cmake --build build -j4
```

The server and client executables can then be launched from the repository root.

## 3. Generate the server identity

The server uses an Ed25519 private key to authenticate itself to clients.

Create a local directory for the keys:

```bash
mkdir -p .local/keys
```

Generate the private key:

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

Restrict access to the private key:

```bash
chmod 600 .local/keys/server-private.pem
```

**Security rules:**

- Never commit or distribute the server private key.
- Distribute only the public key to clients.
- The client must trust the public key belonging to the server it connects to.
- Do not generate a different key pair for each client.

## 4. Start the server

From the repository root:

```bash
./r-type_server
```

By default, the server reads its private key from:

```text
.local/keys/server-private.pem
```

To use a different location:

```bash
RTYPE_SERVER_PRIVATE_KEY=/path/to/server-private.pem ./r-type_server
```

Keep the server terminal open while testing.

## 5. Start a client

Open another terminal at the repository root:

```bash
./r-type_client 127.0.0.1
```

The client reads the trusted server public key from:

```text
.local/keys/server-public.pem
```

Alternatively, specify a custom path:

```bash
RTYPE_SERVER_PUBLIC_KEY=/path/to/server-public.pem ./r-type_client 127.0.0.1
```

In the game interface, connect to the local server using:

```text
127.0.0.1
```

## 6. Test with multiple clients

Keep the server running and open two additional terminals.

Terminal 1 — Server:

```bash
./r-type_server
```

Terminal 2 — First client:

```bash
./r-type_client 127.0.0.1
```

Terminal 3 — Second client:

```bash
./r-type_client 127.0.0.1
```

Both clients should be able to establish their own secure connection to the same server.

Each client has an independent secure session.

## 7. Connect from another computer

For testing across different computers:

1. Start the server on the host computer.
2. Identify the server's local IP address.
3. Copy the server's public key to the client computer through a trusted channel.
4. Place the public key in `.local/keys/server-public.pem` on the client computer.
5. Launch the client using the server's IP address instead of `127.0.0.1`.

Example:

```bash
./r-type_client 192.168.1.100
```

Replace the example address with the actual server IP.

Ensure that both computers can communicate over the network and that the server's UDP port is accessible.

## 8. Run automated tests

From the repository root:

```bash
ctest --test-dir build --output-on-failure
```

At the time of the secure UDP integration, the test suite contained 113 passing tests.

The tests cover compression, encryption, key exchange, key derivation, authentication, handshake messages, session management, packet validation and replay protection.

## 9. Troubleshooting

### Missing environment variable

If the server reports:

```text
Missing environment variable: RTYPE_SERVER_PRIVATE_KEY
```

Make sure the project has been rebuilt with the updated security configuration and that the private key exists.

### Cannot open trusted server public key

Verify that the public key exists at the expected location and that the client is launched from the repository root.

### The server does not answer

Check that:

- The server executable is running.
- The server and client are using the correct IP address.
- The UDP connection is not blocked by the firewall.
- The client trusts the correct server public key.

### Authentication failure

Verify that the trusted public key corresponds to the server's private key.

Do not disable server authentication to bypass the error.

### Build errors

Reconfigure and rebuild the project:

```bash
cmake -S . -B build
cmake --build build -j4
```

## 10. Important notes

The default key paths are relative to the current working directory. Run the executables from the repository root or provide explicit key paths through environment variables.

Local private keys and other sensitive files must remain outside version control.

The secure UDP transport protects network messages, but it does not replace application-level validation, authorization, or gameplay logic.
