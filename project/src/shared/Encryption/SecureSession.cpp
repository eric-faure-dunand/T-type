/*
** EPITECH PROJECT, 2026
** T-type
** File description:
** SecureSession.cpp
*/

#include "Encryption/SecureSession.hpp"
#include <openssl/rand.h>
#include <openssl/sha.h>
#include <stdexcept>

namespace r_type {

namespace {

template <std::size_t Size>
void append(std::vector<std::uint8_t>& output,
    const std::array<std::uint8_t, Size>& value)
{
    output.insert(output.end(), value.begin(), value.end());
}

}

SecureSession::SecureSession():
    _nonce(generateNonce())
{
}

SecureSession::Nonce SecureSession::generateNonce()
{
    Nonce nonce{};

    if (RAND_bytes(nonce.data(), static_cast<int>(nonce.size())) != 1)
        throw std::runtime_error("Cannot generate session nonce");
    return nonce;
}

SecureSession::Request SecureSession::request() const
{
    return {_exchange.publicKey(), _nonce};
}

std::vector<std::uint8_t> SecureSession::transcript(
    const Request& request, const KeyExchange::Key& serverKey,
    const Nonce& serverNonce)
{
    const std::array<std::uint8_t, 25> context = {
        'T', '-', 't', 'y', 'p', 'e', ' ', 'U', 'D', 'P', ' ',
        'h', 'a', 'n', 'd', 's', 'h', 'a', 'k', 'e', ' ', 'v', '1', 0, 1
    };
    std::vector<std::uint8_t> data(context.begin(), context.end());

    append(data, request.publicKey);
    append(data, request.nonce);
    append(data, serverKey);
    append(data, serverNonce);
    return data;
}

KeyDerivation::Salt SecureSession::deriveSalt(
    const std::vector<std::uint8_t>& data)
{
    KeyDerivation::Salt salt{};

    if (!SHA256(data.data(), data.size(), salt.data()))
        throw std::runtime_error("Cannot hash handshake transcript");
    return salt;
}

void SecureSession::establish(const KeyExchange::Key& peerKey,
    const std::vector<std::uint8_t>& data)
{
    const auto secret = _exchange.sharedSecret(peerKey);
    const auto salt = deriveSalt(data);

    _keys = KeyDerivation::derive(secret, salt);
    _ready = true;
}

SecureSession::Response SecureSession::accept(
    const Request& clientRequest, const ServerIdentity& identity)
{
    if (_ready)
        throw std::runtime_error("Session already established");

    const auto serverKey = _exchange.publicKey();
    const auto data = transcript(clientRequest, serverKey, _nonce);
    const auto signature = identity.sign(data);

    establish(clientRequest.publicKey, data);
    return {serverKey, _nonce, signature};
}

bool SecureSession::complete(const Request& clientRequest,
    const Response& response,
    const ServerIdentity::PublicKey& trustedKey)
{
    if (_ready || clientRequest.publicKey != _exchange.publicKey()
        || clientRequest.nonce != _nonce)
        return false;

    const auto data = transcript(clientRequest, response.publicKey,
        response.nonce);

    if (!ServerIdentity::verify(data, response.signature, trustedKey))
        return false;

    try {
        establish(response.publicKey, data);
    } catch (const std::exception&) {
        return false;
    }
    return true;
}

bool SecureSession::ready() const
{
    return _ready;
}

const KeyDerivation::SessionKeys& SecureSession::keys() const
{
    if (!_ready)
        throw std::runtime_error("Secure session is not established");
    return _keys;
}

}
