/*
** EPITECH PROJECT, 2026
** T-type
** File description:
** KeyExchange.hpp
*/

#ifndef KEY_EXCHANGE_HPP
    #define KEY_EXCHANGE_HPP

    #include <array>
    #include <cstdint>
    #include <memory>
    #include <openssl/evp.h>

namespace r_type {

class KeyExchange {
public:
    using Key = std::array<std::uint8_t, 32>;

    KeyExchange();

    Key publicKey() const;
    Key sharedSecret(const Key& peerPublicKey) const;

private:
    using PrivateKey = std::unique_ptr<EVP_PKEY, decltype(&EVP_PKEY_free)>;

    PrivateKey _privateKey;
};

}

#endif
