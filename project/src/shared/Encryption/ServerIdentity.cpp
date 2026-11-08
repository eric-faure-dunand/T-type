/*
** EPITECH PROJECT, 2026
** T-type
** File description:
** ServerIdentity.cpp
*/

#include "Encryption/ServerIdentity.hpp"
#include <cstdio>
#include <memory>
#include <openssl/pem.h>
#include <stdexcept>

namespace r_type {

namespace {

using File = std::unique_ptr<FILE, decltype(&std::fclose)>;
using SignContext = std::unique_ptr<EVP_MD_CTX, decltype(&EVP_MD_CTX_free)>;
using PublicKeyHandle = std::unique_ptr<EVP_PKEY, decltype(&EVP_PKEY_free)>;

EVP_PKEY* loadPrivateKey(const std::string& path)
{
    File file(std::fopen(path.c_str(), "rb"), &std::fclose);
    if (!file)
        throw std::runtime_error("Cannot open server private key");
    EVP_PKEY* key = PEM_read_PrivateKey(file.get(), nullptr, nullptr, nullptr);
    if (!key)
        throw std::runtime_error("Cannot read server private key");
    if (EVP_PKEY_base_id(key) != EVP_PKEY_ED25519) {
        EVP_PKEY_free(key);
        throw std::runtime_error("Server private key must use Ed25519");
    }
    return key;
}

}

ServerIdentity::ServerIdentity(const std::string& privateKeyPath):
    _privateKey(loadPrivateKey(privateKeyPath), EVP_PKEY_free)
{
}

ServerIdentity::PublicKey ServerIdentity::publicKey() const
{
    PublicKey key{};
    std::size_t size = key.size();

    if (EVP_PKEY_get_raw_public_key(_privateKey.get(), key.data(), &size) <= 0
        || size != key.size())
        throw std::runtime_error("Cannot export server public key");
    return key;
}

ServerIdentity::Signature ServerIdentity::sign(
    std::span<const std::uint8_t> message) const
{
    SignContext context(EVP_MD_CTX_new(), EVP_MD_CTX_free);
    if (!context || EVP_DigestSignInit(context.get(), nullptr, nullptr,
        nullptr, _privateKey.get()) <= 0)
        throw std::runtime_error("Cannot initialize Ed25519 signing");

    Signature signature{};
    std::size_t size = signature.size();
    if (EVP_DigestSign(context.get(), signature.data(), &size,
        message.data(), message.size()) <= 0 || size != signature.size())
        throw std::runtime_error("Cannot sign server handshake");
    return signature;
}

bool ServerIdentity::verify(std::span<const std::uint8_t> message,
    const Signature& signature, const PublicKey& publicKey)
{
    PublicKeyHandle key(EVP_PKEY_new_raw_public_key(EVP_PKEY_ED25519,
        nullptr, publicKey.data(), publicKey.size()), EVP_PKEY_free);
    if (!key)
        return false;

    SignContext context(EVP_MD_CTX_new(), EVP_MD_CTX_free);
    if (!context || EVP_DigestVerifyInit(context.get(), nullptr, nullptr,
        nullptr, key.get()) <= 0)
        return false;
    return EVP_DigestVerify(context.get(), signature.data(), signature.size(),
        message.data(), message.size()) == 1;
}

}
