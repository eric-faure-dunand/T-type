/*
** EPITECH PROJECT, 2026
** T-type
** File description:
** SecurityConfig.cpp
*/

#include "Encryption/SecurityConfig.hpp"
#include <cstdlib>
#include <fstream>
#include <iterator>
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <stdexcept>
#include <vector>

namespace r_type {

namespace {

std::string trustedKeyPath()
{
    const char* path = std::getenv("RTYPE_SERVER_PUBLIC_KEY");
    if (path && *path)
        return path;
    return ".local/keys/server-public.pem";
}

}

std::string SecurityConfig::serverPrivateKeyPath()
{
    const char* path = std::getenv("RTYPE_SERVER_PRIVATE_KEY");
    if (path && *path)
        return path;
    return ".local/keys/server-private.pem";
}

ServerIdentity::PublicKey SecurityConfig::trustedServerKey()
{
    const std::string path = trustedKeyPath();
    std::ifstream file(path);
    if (!file)
        throw std::runtime_error("Cannot open trusted server public key");

    const std::string pem((std::istreambuf_iterator<char>(file)),
        std::istreambuf_iterator<char>());

    BIO* bio = BIO_new_mem_buf(pem.data(), static_cast<int>(pem.size()));
    if (!bio)
        throw std::runtime_error("Cannot read trusted server public key");

    EVP_PKEY* key = PEM_read_bio_PUBKEY(bio, nullptr, nullptr, nullptr);
    BIO_free(bio);
    if (!key)
        throw std::runtime_error("Invalid trusted server public key");

    ServerIdentity::PublicKey publicKey{};
    std::size_t size = publicKey.size();
    const bool valid = EVP_PKEY_base_id(key) == EVP_PKEY_ED25519
        && EVP_PKEY_get_raw_public_key(key, publicKey.data(), &size) > 0
        && size == publicKey.size();

    EVP_PKEY_free(key);
    if (!valid)
        throw std::runtime_error("Trusted server key must use Ed25519");
    return publicKey;
}

}
