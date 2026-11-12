# Secure UDP Networking — Technical Architecture

## 1. Overview

The R-Type networking system uses UDP for communication between clients and the authoritative game server.

The secure networking layer adds compression, authenticated encryption, server authentication, session key establishment and replay protection while preserving the existing application protocol.

The implementation is based on the following technologies:

| Component | Technology | Purpose |
|---|---|---|
| Compression | LZ4 | Reduce payload size |
| Encryption | AES-256-GCM | Protect packet confidentiality and integrity |
| Key exchange | X25519 | Establish shared session secrets |
| Key derivation | HKDF | Derive encryption keys |
| Server authentication | Ed25519 | Verify the server's identity |
| Replay protection | Sequence numbers and replay window | Reject duplicated or outdated packets |
| Transport | UDP | Exchange game network messages |

The secure transport is integrated into the existing client and server networking implementation.

## 2. Architecture

The secure networking implementation separates cryptographic operations from application-level networking.

Its main responsibilities are:

- **Compression layer:** compress and decompress payloads.
- **Encryption layer:** encrypt and authenticate payloads.
- **Handshake layer:** exchange the information required to establish secure sessions.
- **Session layer:** manage session keys and security state.
- **Transport layer:** protect outgoing datagrams and validate incoming datagrams.
- **Network integration:** connect secure transport to the existing UDP communication flow.

This separation keeps cryptographic operations independent from gameplay commands.

## 3. Secure connection establishment

Before exchanging protected application messages, the client and server establish a secure session.

The handshake uses ephemeral X25519 key exchange and Ed25519 server authentication.

### Handshake sequence

```text
CLIENT                              SERVER
   |                                   |
   | ------ Handshake Request -------> |
   |                                   |
   | <----- Handshake Response ------- |
   |                                   |
   |    Verify server authentication   |
   |    Derive session keys            |
   |                                   |
   | ------ Handshake Confirm -------> |
   |                                   |
   | <-------- Handshake Ready -------- |
   |                                   |
   |     Secure session established    |
   |                                   |
   | <==== Encrypted UDP messages ====> |
   |                                   |
```

The handshake establishes the cryptographic context used to protect subsequent UDP messages.

The application connection and lobby workflow can then proceed using the existing networking commands.

### 3.1 Handshake request

The client initiates the handshake by sending its key-exchange information.

The client generates fresh X25519 key material for the new connection.

### 3.2 Handshake response

The server responds with the information required to establish the shared secret.

The server also provides authentication data associated with its Ed25519 identity.

The client verifies the response against its configured trusted server public key.

If server authentication fails, the secure session must not be established.

### 3.3 Session key derivation

Both peers calculate the shared secret using X25519.

HKDF derives the cryptographic keys used by the secure transport.

Separate directional keys are used for client-to-server and server-to-client traffic.

This prevents the two communication directions from sharing the same encryption key.

### 3.4 Handshake confirmation

The client confirms the negotiated session.

The server validates the confirmation before completing the secure connection.

The final handshake-ready message informs the client that the secure connection can proceed.

Application messages must not bypass the required secure-session checks.

## 4. Compression

LZ4 is used to reduce the size of suitable application payloads.

Compression is performed before encryption.

The receiver restores the original payload after successful authentication and decryption.

Conceptually:

```text
Outgoing:
Application payload
        |
    Compression
        |
     Encryption
        |
    UDP datagram

Incoming:
    UDP datagram
        |
 Authentication
   + Decryption
        |
   Decompression
        |
Application payload
```

The implementation includes checks for invalid compressed data and oversized decompressed payloads.

These checks are important because untrusted compressed input must not cause uncontrolled memory allocation.

## 5. Authenticated encryption

AES-256-GCM provides confidentiality and integrity protection.

The encryption process uses:

- A session encryption key.
- A unique nonce.
- The plaintext payload.
- Additional Authenticated Data (AAD).

The output includes encrypted data and an authentication tag.

The receiver must validate the authentication tag before accepting the plaintext.

### Additional Authenticated Data

AAD allows selected protocol information to be authenticated without encrypting it.

This protects security-relevant metadata against unauthorized modification.

If authenticated metadata is changed, decryption validation fails.

### Nonce management

AES-GCM requires that a nonce is not reused with the same encryption key.

The secure transport manages outgoing packet sequencing and nonce construction.

Packet sequence handling must remain consistent with the cryptographic session.

## 6. Secure datagrams

The `SecureDatagram` component connects packet serialization with authenticated encryption.

Its responsibility is to transform application-level network messages into protected datagrams and restore them after successful verification.

The existing binary protocol remains the application-facing format.

This avoids requiring gameplay handlers to implement encryption or decryption themselves.

The exact serialized datagram layout is defined by the implementation and should not be modified independently on one peer.

## 7. Secure transport

`SecureTransport` manages the protection and validation of messages exchanged through an established secure session.

Its responsibilities include:

- Encrypting outgoing application messages.
- Authenticating and decrypting incoming messages.
- Maintaining outgoing packet sequence numbers.
- Rejecting invalid encrypted datagrams.
- Applying replay protection.
- Keeping communication directions separated.

The client and server must use compatible protocol and cryptographic implementations.

## 8. Replay protection

UDP packets may arrive out of order, be duplicated or be replayed by an attacker.

The implementation uses a replay window to track accepted sequence numbers.

Expected behavior:

| Situation | Result |
|---|---|
| First valid packet | Accepted |
| New valid sequence number | Accepted |
| Duplicate sequence number | Rejected |
| Valid out-of-order packet within the window | Accepted |
| Packet older than the replay window | Rejected |
| Packet with invalid authentication | Rejected |

Replay tracking belongs to the secure session.

Separate client sessions must not share replay state.

## 9. Secure session management

Each connected client has its own cryptographic session.

Session management associates secure transport state with the corresponding network endpoint.

The session manager is responsible for installing authenticated sessions, retrieving existing sessions and removing sessions when required.

This allows the server to communicate securely with multiple clients independently.

### Session isolation

A packet encrypted for one session must not be accepted as a valid packet belonging to another session.

Session keys, sequence numbers and replay state remain independent.

### Handshake cleanup

The server tracks pending handshakes and removes expired pending state.

The integration uses an approximately five-second timeout for incomplete handshakes.

This prevents abandoned handshake attempts from remaining indefinitely in the pending-handshake collection.

## 10. Server authentication and key configuration

The server has a persistent Ed25519 identity.

The private key is stored on the server, while clients are configured with the corresponding trusted public key.

Default local paths:

```text
.local/keys/server-private.pem
.local/keys/server-public.pem
```

Optional environment variables:

```text
RTYPE_SERVER_PRIVATE_KEY
RTYPE_SERVER_PUBLIC_KEY
```

Environment variables override the default paths.

The default paths are relative to the process working directory.

### Security requirements

- Never commit the server private key.
- Never send the private key to clients.
- Distribute the public key through a trusted channel.
- Do not silently accept an unknown server identity.
- Do not disable authentication to resolve connection problems.

## 11. Integration with the existing network

The secure UDP layer is integrated into the existing networking implementation.

Relevant components include:

| Component | Responsibility |
|---|---|
| `network` | UDP communication and secure handshake integration |
| `packet` | Application packet representation and serialization |
| `protocol` | Network command definitions |
| `SecureHandshake` | Handshake message serialization |
| `SecureDatagram` | Protected datagram encoding and decoding |
| `SecureTransport` | Encrypted transport and replay checks |
| `SecureSessionManager` | Secure session lifecycle |
| `SecurityConfig` | Server identity key configuration |

The networking layer remains responsible for delivering validated application messages to the appropriate handlers.

Game logic should not directly manipulate encryption keys or cryptographic primitives.

## 12. Existing protocol compatibility

The secure networking integration is designed to preserve the existing application protocol.

Gameplay and lobby messages continue to use the established command and packet abstractions.

Security-related handshake messages are handled separately from normal application traffic.

When extending the protocol, developers should preserve the secure transport boundary.

New application commands should use the existing protected send and receive paths rather than introducing independent unencrypted UDP communication.

## 13. Testing

The secure networking implementation includes automated tests for:

- LZ4 compression and decompression.
- AES-256-GCM encryption and authentication.
- Secure packet serialization.
- Secure datagram validation.
- X25519 key exchange.
- HKDF key derivation.
- Ed25519 signatures and identity verification.
- Handshake message encoding and decoding.
- Secure session establishment.
- Secure session management.
- Replay protection.
- Bidirectional encrypted transport.

Run the complete test suite:

```bash id="3ml7wr"
ctest --test-dir build --output-on-failure
```

At the final integration checkpoint, all 113 registered tests passed.

Manual validation also confirmed successful local server startup and connections from two clients.

## 14. Maintenance guidelines

When modifying secure networking code:

1. Preserve the existing `.hpp` and `.cpp` separation.
2. Keep functions small and focused on a single responsibility.
3. Follow the established C++ naming and formatting conventions.
4. Keep cryptographic operations separate from gameplay logic.
5. Do not introduce plaintext application-message paths that bypass the secure transport.
6. Add or update tests when changing security-sensitive behavior.
7. Verify compatibility between client and server.
8. Run the full test suite before merging.

## 15. Known scope and limitations

The secure transport protects UDP communication but does not guarantee reliable or ordered delivery.

UDP packet loss and application-level synchronization remain separate concerns.

The security layer also does not replace:

- Gameplay input validation.
- Application authorization.
- Rate limiting and denial-of-service protection.
- Reliable delivery mechanisms where required.
- Production key provisioning and rotation.

The implementation has been validated through automated tests and local multiplayer testing. Broader network and security testing should be performed before production deployment.
