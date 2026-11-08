
/*
** EPITECH PROJECT, 2026
** T-type
** File description:
** Crypter.cpp
*/

#include "Encryption/Crypter.hpp"
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <memory>
#include <stdexcept>

namespace r_type {

namespace {
constexpr int nonceSize = 12;
constexpr int tagSize = 16;

using CipherContext = std::unique_ptr<EVP_CIPHER_CTX, decltype(&EVP_CIPHER_CTX_free)>;

CipherContext createContext() {
    CipherContext context(EVP_CIPHER_CTX_new(), EVP_CIPHER_CTX_free);
    if (!context)
        throw std::runtime_error("Failed to create cipher context");
    return context;
}
}

Crypter::Crypter(const Key& key) : _key(key) {}

std::string Crypter::encrypt(const std::string& data) const {
    if (data.size() > 1024 * 1024)
        throw std::length_error("Data too large to encrypt");

    std::string result(nonceSize + data.size() + tagSize, '\0');
    auto context = createContext();

    if (RAND_bytes(reinterpret_cast<unsigned char*>(result.data()), nonceSize) != 1)
        throw std::runtime_error("Failed to generate nonce");

    if (EVP_EncryptInit_ex(context.get(), EVP_aes_256_gcm(), nullptr,
        _key.data(), reinterpret_cast<const unsigned char*>(result.data())) != 1)
        throw std::runtime_error("Encryption initialization failed");

    int written = 0;
    if (EVP_EncryptUpdate(context.get(),
        reinterpret_cast<unsigned char*>(result.data() + nonceSize),
        &written, reinterpret_cast<const unsigned char*>(data.data()),
        static_cast<int>(data.size())) != 1)
        throw std::runtime_error("Encryption failed");

    int finalSize = 0;
    if (EVP_EncryptFinal_ex(context.get(),
        reinterpret_cast<unsigned char*>(result.data() + nonceSize + written),
        &finalSize) != 1)
        throw std::runtime_error("Encryption finalization failed");

    if (EVP_CIPHER_CTX_ctrl(context.get(), EVP_CTRL_GCM_GET_TAG, tagSize,
        result.data() + nonceSize + written + finalSize) != 1)
        throw std::runtime_error("Failed to get authentication tag");

    result.resize(nonceSize + written + finalSize + tagSize);
    return result;
}

std::string Crypter::decrypt(const std::string& data) const {
    if (data.size() < nonceSize + tagSize)
        throw std::runtime_error("Invalid encrypted data");
    if (data.size() > 1024 * 1024 + nonceSize + tagSize)
        throw std::length_error("Encrypted data too large");

    const int cipherSize = static_cast<int>(data.size() - nonceSize - tagSize);
    std::string result(cipherSize, '\0');
    auto context = createContext();

    if (EVP_DecryptInit_ex(context.get(), EVP_aes_256_gcm(), nullptr,
        _key.data(), reinterpret_cast<const unsigned char*>(data.data())) != 1)
        throw std::runtime_error("Decryption initialization failed");

    int written = 0;
    if (EVP_DecryptUpdate(context.get(),
        reinterpret_cast<unsigned char*>(result.data()), &written,
        reinterpret_cast<const unsigned char*>(data.data() + nonceSize),
        cipherSize) != 1)
        throw std::runtime_error("Decryption failed");

    if (EVP_CIPHER_CTX_ctrl(context.get(), EVP_CTRL_GCM_SET_TAG, tagSize,
        const_cast<char*>(data.data() + nonceSize + cipherSize)) != 1)
        throw std::runtime_error("Failed to set authentication tag");

    int finalSize = 0;
    if (EVP_DecryptFinal_ex(context.get(),
        reinterpret_cast<unsigned char*>(result.data() + written),
        &finalSize) != 1)
        throw std::runtime_error("Authentication failed");

    result.resize(written + finalSize);
    return result;
}

}
