/*
** EPITECH PROJECT, 2026
** T-type
** File description:
** KeyExchange.cpp
*/

#include "Encryption/KeyExchange.hpp"
#include <stdexcept>

namespace r_type {

namespace {
using KeyContext = std::unique_ptr<EVP_PKEY_CTX, decltype(&EVP_PKEY_CTX_free)>;
using PublicKey = std::unique_ptr<EVP_PKEY, decltype(&EVP_PKEY_free)>;

KeyContext createContext(EVP_PKEY* key) {
    KeyContext context(EVP_PKEY_CTX_new(key, nullptr), EVP_PKEY_CTX_free);
    if (!context)
        throw std::runtime_error("Failed to create key context");
    return context;
}

PublicKey createPublicKey(const KeyExchange::Key& key) {
    PublicKey publicKey(EVP_PKEY_new_raw_public_key(
        EVP_PKEY_X25519, nullptr, key.data(), key.size()), EVP_PKEY_free);
    if (!publicKey)
        throw std::runtime_error("Invalid public key");
    return publicKey;
}
}

KeyExchange::KeyExchange() : _privateKey(nullptr, EVP_PKEY_free) {
    KeyContext context(EVP_PKEY_CTX_new_id(EVP_PKEY_X25519, nullptr),
        EVP_PKEY_CTX_free);

    if (!context || EVP_PKEY_keygen_init(context.get()) != 1)
        throw std::runtime_error("Failed to initialize key generation");

    EVP_PKEY* key = nullptr;
    if (EVP_PKEY_keygen(context.get(), &key) != 1)
        throw std::runtime_error("Failed to generate key pair");

    _privateKey.reset(key);
}

KeyExchange::Key KeyExchange::publicKey() const {
    Key result{};
    std::size_t size = result.size();

    if (EVP_PKEY_get_raw_public_key(_privateKey.get(),
        result.data(), &size) != 1 || size != result.size())
        throw std::runtime_error("Failed to export public key");

    return result;
}

KeyExchange::Key KeyExchange::sharedSecret(const Key& peerPublicKey) const {
    auto peer = createPublicKey(peerPublicKey);
    auto context = createContext(_privateKey.get());

    if (EVP_PKEY_derive_init(context.get()) != 1 ||
        EVP_PKEY_derive_set_peer(context.get(), peer.get()) != 1)
        throw std::runtime_error("Failed to initialize key exchange");

    Key result{};
    std::size_t size = result.size();

    if (EVP_PKEY_derive(context.get(), result.data(), &size) != 1 ||
        size != result.size())
        throw std::runtime_error("Failed to derive shared secret");

    return result;
}

}
