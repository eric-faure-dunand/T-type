/*
** EPITECH PROJECT, 2026
** T-type
** File description:
** test_key_exchange.cpp
*/

#include "Encryption/KeyExchange.hpp"
#include <gtest/gtest.h>

TEST(KeyExchangeTest, GenerateDifferentPublicKeys) {
    r_type::KeyExchange client;
    r_type::KeyExchange server;

    EXPECT_NE(client.publicKey(), server.publicKey());
}

TEST(KeyExchangeTest, GenerateSameSharedSecret) {
    r_type::KeyExchange client;
    r_type::KeyExchange server;

    const auto clientSecret = client.sharedSecret(server.publicKey());
    const auto serverSecret = server.sharedSecret(client.publicKey());

    EXPECT_EQ(clientSecret, serverSecret);
}

TEST(KeyExchangeTest, GenerateDifferentSessionSecrets) {
    r_type::KeyExchange client;
    r_type::KeyExchange firstServer;
    r_type::KeyExchange secondServer;

    EXPECT_NE(
        client.sharedSecret(firstServer.publicKey()),
        client.sharedSecret(secondServer.publicKey())
    );
}

TEST(KeyExchangeTest, RejectInvalidPublicKey) {
    r_type::KeyExchange client;
    r_type::KeyExchange::Key invalidKey{};

    EXPECT_THROW(client.sharedSecret(invalidKey), std::runtime_error);
}
