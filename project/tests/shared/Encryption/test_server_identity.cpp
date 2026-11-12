/*
** EPITECH PROJECT, 2026
** T-type
** File description:
** test_server_identity.cpp
*/

#include "Encryption/ServerIdentity.hpp"
#include <filesystem>
#include <gtest/gtest.h>
#include <memory>
#include <openssl/pem.h>
#include <stdexcept>
#include <string>
#include <vector>

using namespace r_type;

namespace {

struct FileCloser {
    void operator()(FILE* file) const
    {
        if (file)
            std::fclose(file);
    }
};

using KeyHandle = std::unique_ptr<EVP_PKEY, decltype(&EVP_PKEY_free)>;
using KeyContext = std::unique_ptr<EVP_PKEY_CTX, decltype(&EVP_PKEY_CTX_free)>;

class ServerIdentityTest : public ::testing::Test {
protected:
    std::filesystem::path _path;

    void SetUp() override
    {
        KeyContext context(EVP_PKEY_CTX_new_id(EVP_PKEY_ED25519, nullptr),
            EVP_PKEY_CTX_free);
        if (!context || EVP_PKEY_keygen_init(context.get()) <= 0)
            throw std::runtime_error("Cannot initialize test key");

        EVP_PKEY* generated = nullptr;
        if (EVP_PKEY_keygen(context.get(), &generated) <= 0)
            throw std::runtime_error("Cannot generate test key");
        KeyHandle key(generated, EVP_PKEY_free);

        _path = std::filesystem::temp_directory_path()
            / ("t-type-ed25519-" + std::to_string(
                reinterpret_cast<std::uintptr_t>(this)) + ".pem");

        std::unique_ptr<FILE, FileCloser> file(
            std::fopen(_path.string().c_str(), "wb"));
        if (!file || PEM_write_PrivateKey(file.get(), key.get(),
            nullptr, nullptr, 0, nullptr, nullptr) != 1)
            throw std::runtime_error("Cannot write test key");
    }

    void TearDown() override
    {
        std::error_code error;
        std::filesystem::remove(_path, error);
    }
};

}

TEST_F(ServerIdentityTest, VerifyValidSignature)
{
    ServerIdentity identity(_path.string());
    const std::vector<std::uint8_t> message = {1, 2, 3, 4};
    const auto signature = identity.sign(message);

    EXPECT_TRUE(ServerIdentity::verify(message, signature,
        identity.publicKey()));
}

TEST_F(ServerIdentityTest, RejectModifiedMessage)
{
    ServerIdentity identity(_path.string());
    const std::vector<std::uint8_t> message = {1, 2, 3, 4};
    const std::vector<std::uint8_t> modified = {1, 2, 3, 5};
    const auto signature = identity.sign(message);

    EXPECT_FALSE(ServerIdentity::verify(modified, signature,
        identity.publicKey()));
}

TEST_F(ServerIdentityTest, RejectModifiedSignature)
{
    ServerIdentity identity(_path.string());
    const std::vector<std::uint8_t> message = {1, 2, 3, 4};
    auto signature = identity.sign(message);
    signature[0] ^= 0x01;

    EXPECT_FALSE(ServerIdentity::verify(message, signature,
        identity.publicKey()));
}

TEST_F(ServerIdentityTest, RejectWrongPublicKey)
{
    ServerIdentity identity(_path.string());
    const std::vector<std::uint8_t> message = {1, 2, 3, 4};
    const auto signature = identity.sign(message);
    auto wrongKey = identity.publicKey();
    wrongKey[0] ^= 0x01;

    EXPECT_FALSE(ServerIdentity::verify(message, signature, wrongKey));
}

TEST(ServerIdentityInvalidKeyTest, RejectMissingPrivateKey)
{
    EXPECT_THROW(ServerIdentity("/nonexistent/t-type-key.pem"),
        std::runtime_error);
}
