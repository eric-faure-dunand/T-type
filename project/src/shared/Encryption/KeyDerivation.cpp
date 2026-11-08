/*
** EPITECH PROJECT, 2026
** T-type
** File description:
** KeyDerivation.cpp
*/

#include "Encryption/KeyDerivation.hpp"
#include <memory>
#include <openssl/evp.h>
#include <openssl/kdf.h>
#include <stdexcept>
#include <string>

namespace r_type {

namespace {

using HkdfContext = std::unique_ptr<EVP_PKEY_CTX, decltype(&EVP_PKEY_CTX_free)>;

KeyDerivation::Key deriveKey(const KeyDerivation::Key& secret,
    const KeyDerivation::Salt& salt, const std::string& info)
{
    HkdfContext context(EVP_PKEY_CTX_new_id(EVP_PKEY_HKDF, nullptr),
        EVP_PKEY_CTX_free);
    if (!context)
        throw std::runtime_error("Cannot create HKDF context");
    if (EVP_PKEY_derive_init(context.get()) <= 0
        || EVP_PKEY_CTX_set_hkdf_md(context.get(), EVP_sha256()) <= 0
        || EVP_PKEY_CTX_set1_hkdf_salt(context.get(), salt.data(), salt.size()) <= 0
        || EVP_PKEY_CTX_set1_hkdf_key(context.get(), secret.data(), secret.size()) <= 0
        || EVP_PKEY_CTX_add1_hkdf_info(context.get(),
            reinterpret_cast<const unsigned char*>(info.data()), info.size()) <= 0)
        throw std::runtime_error("Cannot initialize HKDF");

    KeyDerivation::Key key{};
    std::size_t size = key.size();
    if (EVP_PKEY_derive(context.get(), key.data(), &size) <= 0
        || size != key.size())
        throw std::runtime_error("Cannot derive HKDF key");
    return key;
}

}

KeyDerivation::SessionKeys KeyDerivation::derive(
    const Key& sharedSecret, const Salt& salt)
{
    return {
        deriveKey(sharedSecret, salt, "T-type UDP v1 client-to-server"),
        deriveKey(sharedSecret, salt, "T-type UDP v1 server-to-client")
    };
}

}
