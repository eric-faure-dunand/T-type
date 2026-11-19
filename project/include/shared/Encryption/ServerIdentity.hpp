/*
** EPITECH PROJECT, 2026
** T-type
** File description:
** ServerIdentity.hpp
*/

#ifndef SERVER_IDENTITY_HPP
    #define SERVER_IDENTITY_HPP

    #include <array>
    #include <cstdint>
    #include <memory>
    #include <openssl/evp.h>
    #include <span>
    #include <string>

namespace r_type {

class ServerIdentity {
public:
    using PublicKey = std::array<std::uint8_t, 32>;
    using Signature = std::array<std::uint8_t, 64>;

    explicit ServerIdentity(const std::string& privateKeyPath);

    PublicKey publicKey() const;
    Signature sign(std::span<const std::uint8_t> message) const;

    static bool verify(std::span<const std::uint8_t> message,
        const Signature& signature, const PublicKey& publicKey);

private:
    using PrivateKey = std::unique_ptr<EVP_PKEY, decltype(&EVP_PKEY_free)>;

    PrivateKey _privateKey;
};

}

#endif
