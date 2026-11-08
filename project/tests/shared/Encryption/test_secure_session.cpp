/*
** EPITECH PROJECT, 2026
** T-type
** File description:
** test_secure_session.cpp
*/

#include "Encryption/SecureSession.hpp"
#include <cstdio>
#include <filesystem>
#include <gtest/gtest.h>
#include <memory>
#include <openssl/pem.h>
#include <openssl/rand.h>
#include <stdexcept>
#include <string>

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

std::filesystem::path createKeyPath()
{
    std::array<unsigned char, 8> random{};
    if (RAND_bytes(random.data(), random.size()) != 1)
        throw std::runtime_error("Cannot generate test filename");

    std::string name = "t-type-session-";
    constexpr char digits[] = "0123456789abcdef";
    for (unsigned char byte : random) {
        name += digits[byte >> 4];
        name += digits[byte & 0x0f];
    }
    return std::filesystem::temp_directory_path() / (name + ".pem");
}

void writePrivateKey(const std::filesystem::path& path)
{
    KeyContext context(EVP_PKEY_CTX_new_id(EVP_PKEY_ED25519, nullptr),
        EVP_PKEY_CTX_free);
    if (!context || EVP_PKEY_keygen_init(context.get()) <= 0)
        throw std::runtime_error("Cannot initialize Ed25519 test key");

    EVP_PKEY* generated = nullptr;
    if (EVP_PKEY_keygen(context.get(), &generated) <= 0)
        throw std::runtime_error("Cannot generate Ed25519 test key");
    KeyHandle key(generated, EVP_PKEY_free);

    std::unique_ptr<FILE, FileCloser> file(
        std::fopen(path.string().c_str(), "wb"));
    if (!file || PEM_write_PrivateKey(file.get(), key.get(),
        nullptr, nullptr, 0, nullptr, nullptr) != 1)
        throw std::runtime_error("Cannot write Ed25519 test key");
}

class SecureSessionTest : public ::testing::Test {
protected:
    std::filesystem::path _path;

    void SetUp() override
    {
        _path = createKeyPath();
        writePrivateKey(_path);
    }

    void TearDown() override
    {
        std::error_code error;
        std::filesystem::remove(_path, error);
    }
};

}

TEST_F(SecureSessionTest, EstablishSameDirectionalKeys)
{
    ServerIdentity identity(_path.string());
    SecureSession client;
    SecureSession server;

    const auto request = client.request();
    const auto response = server.accept(request, identity);

    ASSERT_TRUE(client.complete(request, response, identity.publicKey()));
    ASSERT_TRUE(client.ready());
    ASSERT_TRUE(server.ready());

    EXPECT_EQ(client.keys().clientToServer, server.keys().clientToServer);
    EXPECT_EQ(client.keys().serverToClient, server.keys().serverToClient);
    EXPECT_NE(client.keys().clientToServer, client.keys().serverToClient);
}

TEST_F(SecureSessionTest, RejectWrongServerIdentity)
{
    ServerIdentity identity(_path.string());
    SecureSession client;
    SecureSession server;

    const auto request = client.request();
    const auto response = server.accept(request, identity);
    auto wrongKey = identity.publicKey();
    wrongKey[0] ^= 0x01;

    EXPECT_FALSE(client.complete(request, response, wrongKey));
    EXPECT_FALSE(client.ready());
}

TEST_F(SecureSessionTest, RejectModifiedServerPublicKey)
{
    ServerIdentity identity(_path.string());
    SecureSession client;
    SecureSession server;

    const auto request = client.request();
    auto response = server.accept(request, identity);
    response.publicKey[0] ^= 0x01;

    EXPECT_FALSE(client.complete(request, response, identity.publicKey()));
    EXPECT_FALSE(client.ready());
}

TEST_F(SecureSessionTest, RejectModifiedServerNonce)
{
    ServerIdentity identity(_path.string());
    SecureSession client;
    SecureSession server;

    const auto request = client.request();
    auto response = server.accept(request, identity);
    response.nonce[0] ^= 0x01;

    EXPECT_FALSE(client.complete(request, response, identity.publicKey()));
    EXPECT_FALSE(client.ready());
}

TEST_F(SecureSessionTest, RejectModifiedClientRequest)
{
    ServerIdentity identity(_path.string());
    SecureSession client;
    SecureSession server;

    const auto request = client.request();
    const auto response = server.accept(request, identity);
    auto modified = request;
    modified.nonce[0] ^= 0x01;

    EXPECT_FALSE(client.complete(modified, response, identity.publicKey()));
    EXPECT_FALSE(client.ready());
}

TEST_F(SecureSessionTest, RejectModifiedSignature)
{
    ServerIdentity identity(_path.string());
    SecureSession client;
    SecureSession server;

    const auto request = client.request();
    auto response = server.accept(request, identity);
    response.signature[0] ^= 0x01;

    EXPECT_FALSE(client.complete(request, response, identity.publicKey()));
    EXPECT_FALSE(client.ready());
}

TEST_F(SecureSessionTest, RejectRepeatedCompletion)
{
    ServerIdentity identity(_path.string());
    SecureSession client;
    SecureSession server;

    const auto request = client.request();
    const auto response = server.accept(request, identity);

    ASSERT_TRUE(client.complete(request, response, identity.publicKey()));
    EXPECT_FALSE(client.complete(request, response, identity.publicKey()));
}

TEST(SecureSessionStateTest, RejectKeysBeforeHandshake)
{
    SecureSession session;

    EXPECT_FALSE(session.ready());
    EXPECT_THROW(session.keys(), std::runtime_error);
}

TEST(SecureSessionStateTest, GenerateDifferentRequests)
{
    SecureSession first;
    SecureSession second;

    const auto firstRequest = first.request();
    const auto secondRequest = second.request();

    EXPECT_NE(firstRequest.publicKey, secondRequest.publicKey);
    EXPECT_NE(firstRequest.nonce, secondRequest.nonce);
}
