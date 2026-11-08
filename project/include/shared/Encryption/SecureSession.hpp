/*
** EPITECH PROJECT, 2026
** T-type
** File description:
** SecureSession.hpp
*/

#ifndef SECURE_SESSION_HPP
    #define SECURE_SESSION_HPP

    #include "Encryption/KeyDerivation.hpp"
    #include "Encryption/KeyExchange.hpp"
    #include "Encryption/ServerIdentity.hpp"
    #include <array>
    #include <cstdint>
    #include <vector>

namespace r_type {

class SecureSession {
public:
    using Nonce = std::array<std::uint8_t, 32>;

    struct Request {
        KeyExchange::Key publicKey;
        Nonce nonce;
    };

    struct Response {
        KeyExchange::Key publicKey;
        Nonce nonce;
        ServerIdentity::Signature signature;
    };

    SecureSession();

    Request request() const;
    Response accept(const Request& request, const ServerIdentity& identity);
    bool complete(const Request& request, const Response& response,
        const ServerIdentity::PublicKey& trustedKey);

    bool ready() const;
    const KeyDerivation::SessionKeys& keys() const;

private:
    static Nonce generateNonce();
    static std::vector<std::uint8_t> transcript(
        const Request& request, const KeyExchange::Key& serverKey,
        const Nonce& serverNonce);
    static KeyDerivation::Salt deriveSalt(
        const std::vector<std::uint8_t>& transcript);

    void establish(const KeyExchange::Key& peerKey,
        const std::vector<std::uint8_t>& transcript);

    KeyExchange _exchange;
    Nonce _nonce;
    KeyDerivation::SessionKeys _keys{};
    bool _ready = false;
};

}

#endif
