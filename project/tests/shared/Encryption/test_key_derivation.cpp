/*
** EPITECH PROJECT, 2026
** T-type
** File description:
** test_key_derivation.cpp
*/

#include "Encryption/KeyDerivation.hpp"
#include "Encryption/KeyExchange.hpp"
#include <gtest/gtest.h>

using namespace r_type;

TEST(KeyDerivationTest, BothPeersDeriveSameKeys)
{
    KeyExchange client;
    KeyExchange server;
    const auto clientSecret = client.sharedSecret(server.publicKey());
    const auto serverSecret = server.sharedSecret(client.publicKey());
    KeyDerivation::Salt salt{};
    salt.fill(42);

    const auto clientKeys = KeyDerivation::derive(clientSecret, salt);
    const auto serverKeys = KeyDerivation::derive(serverSecret, salt);

    EXPECT_EQ(clientKeys.clientToServer, serverKeys.clientToServer);
    EXPECT_EQ(clientKeys.serverToClient, serverKeys.serverToClient);
}

TEST(KeyDerivationTest, DirectionalKeysAreDifferent)
{
    KeyDerivation::Key secret{};
    KeyDerivation::Salt salt{};
    secret.fill(12);
    salt.fill(34);

    const auto keys = KeyDerivation::derive(secret, salt);

    EXPECT_NE(keys.clientToServer, keys.serverToClient);
}

TEST(KeyDerivationTest, DifferentSaltsProduceDifferentKeys)
{
    KeyDerivation::Key secret{};
    KeyDerivation::Salt firstSalt{};
    KeyDerivation::Salt secondSalt{};
    secret.fill(12);
    firstSalt.fill(34);
    secondSalt.fill(56);

    const auto first = KeyDerivation::derive(secret, firstSalt);
    const auto second = KeyDerivation::derive(secret, secondSalt);

    EXPECT_NE(first.clientToServer, second.clientToServer);
    EXPECT_NE(first.serverToClient, second.serverToClient);
}

TEST(KeyDerivationTest, DerivationIsDeterministic)
{
    KeyDerivation::Key secret{};
    KeyDerivation::Salt salt{};
    secret.fill(12);
    salt.fill(34);

    const auto first = KeyDerivation::derive(secret, salt);
    const auto second = KeyDerivation::derive(secret, salt);

    EXPECT_EQ(first.clientToServer, second.clientToServer);
    EXPECT_EQ(first.serverToClient, second.serverToClient);
}
