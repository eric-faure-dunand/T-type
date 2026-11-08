/*
** EPITECH PROJECT, 2026
** T-type
** File description:
** test_secure_session_manager.cpp
*/

#include "Network/SecureSessionManager.hpp"
#include <cstdio>
#include <filesystem>
#include <gtest/gtest.h>
#include <memory>
#include <openssl/pem.h>
#include <stdexcept>

using namespace r_type;

namespace {

using Endpoint = SecureSessionManager::Endpoint;
using KeyHandle = std::unique_ptr<EVP_PKEY, decltype(&EVP_PKEY_free)>;
using KeyContext = std::unique_ptr<EVP_PKEY_CTX, decltype(&EVP_PKEY_CTX_free)>;

struct FileCloser {
    void operator()(FILE* file) const {
        if (file)
            std::fclose(file);
    }
};

class SecureSessionManagerTest : public ::testing::Test {
protected:
    std::filesystem::path _path;

    void SetUp() override {
        KeyContext context(EVP_PKEY_CTX_new_id(EVP_PKEY_ED25519, nullptr),
            EVP_PKEY_CTX_free);
        if (!context || EVP_PKEY_keygen_init(context.get()) <= 0)
            throw std::runtime_error("Cannot initialize test key");

        EVP_PKEY* generated = nullptr;
        if (EVP_PKEY_keygen(context.get(), &generated) <= 0)
            throw std::runtime_error("Cannot generate test key");
        KeyHandle key(generated, EVP_PKEY_free);

        _path = std::filesystem::temp_directory_path()
            / ("t-type-manager-" + std::to_string(
                reinterpret_cast<std::uintptr_t>(this)) + ".pem");

        std::unique_ptr<FILE, FileCloser> file(
            std::fopen(_path.string().c_str(), "wb"));
        if (!file || PEM_write_PrivateKey(file.get(), key.get(),
            nullptr, nullptr, 0, nullptr, nullptr) != 1)
            throw std::runtime_error("Cannot write test key");
    }

    void TearDown() override {
        std::error_code error;
        std::filesystem::remove(_path, error);
    }

    void establish(SecureSession& client, SecureSession& server) {
        ServerIdentity identity(_path.string());
        const auto request = client.request();
        const auto response = server.accept(request, identity);

        ASSERT_TRUE(client.complete(request, response, identity.publicKey()));
        ASSERT_TRUE(server.confirm(client.confirmation()));
    }

    Endpoint endpoint(unsigned short port) {
        return Endpoint(asio::ip::make_address("127.0.0.1"), port);
    }
};

}

TEST_F(SecureSessionManagerTest, RejectUnconfirmedSession) {
    SecureSessionManager manager(SecureTransport::Role::Server);
    SecureSession session;

    EXPECT_FALSE(manager.install(endpoint(7001), session));
    EXPECT_EQ(manager.size(), 0);
}

TEST_F(SecureSessionManagerTest, InstallAuthenticatedSession) {
    SecureSession client;
    SecureSession server;
    establish(client, server);

    SecureSessionManager manager(SecureTransport::Role::Server);

    EXPECT_TRUE(manager.install(endpoint(7001), server));
    EXPECT_TRUE(manager.contains(endpoint(7001)));
    EXPECT_EQ(manager.size(), 1);
}

TEST_F(SecureSessionManagerTest, RejectDuplicateEndpoint) {
    SecureSession client;
    SecureSession server;
    establish(client, server);

    SecureSessionManager manager(SecureTransport::Role::Server);

    ASSERT_TRUE(manager.install(endpoint(7001), server));
    EXPECT_FALSE(manager.install(endpoint(7001), server));
    EXPECT_EQ(manager.size(), 1);
}

TEST_F(SecureSessionManagerTest, KeepEndpointsIndependent) {
    SecureSession firstClient;
    SecureSession firstServer;
    SecureSession secondClient;
    SecureSession secondServer;
    establish(firstClient, firstServer);
    establish(secondClient, secondServer);

    SecureSessionManager manager(SecureTransport::Role::Server);

    ASSERT_TRUE(manager.install(endpoint(7001), firstServer));
    ASSERT_TRUE(manager.install(endpoint(7002), secondServer));

    EXPECT_EQ(manager.size(), 2);
    EXPECT_TRUE(manager.contains(endpoint(7001)));
    EXPECT_TRUE(manager.contains(endpoint(7002)));
}

TEST_F(SecureSessionManagerTest, RemoveSession) {
    SecureSession client;
    SecureSession server;
    establish(client, server);

    SecureSessionManager manager(SecureTransport::Role::Server);
    ASSERT_TRUE(manager.install(endpoint(7001), server));

    EXPECT_TRUE(manager.remove(endpoint(7001)));
    EXPECT_FALSE(manager.contains(endpoint(7001)));
    EXPECT_FALSE(manager.remove(endpoint(7001)));
}

TEST_F(SecureSessionManagerTest, ClearSessions) {
    SecureSession firstClient;
    SecureSession firstServer;
    SecureSession secondClient;
    SecureSession secondServer;
    establish(firstClient, firstServer);
    establish(secondClient, secondServer);

    SecureSessionManager manager(SecureTransport::Role::Server);
    ASSERT_TRUE(manager.install(endpoint(7001), firstServer));
    ASSERT_TRUE(manager.install(endpoint(7002), secondServer));

    manager.clear();

    EXPECT_EQ(manager.size(), 0);
    EXPECT_FALSE(manager.contains(endpoint(7001)));
    EXPECT_FALSE(manager.contains(endpoint(7002)));
}

TEST_F(SecureSessionManagerTest, RejectUnknownEndpoint) {
    SecureSessionManager manager(SecureTransport::Role::Server);
    const std::vector<std::uint8_t> data = {1, 2, 3};

    EXPECT_THROW(manager.encode(endpoint(7001), data), std::runtime_error);
    EXPECT_THROW(manager.decode(endpoint(7001), data), std::runtime_error);
}
