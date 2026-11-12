/*
** EPITECH PROJECT, 2026
** T-type
** File description:
** test_secure_transport.cpp
*/

#include "Network/SecureTransport.hpp"
#include "Network/packet.hpp"
#include <gtest/gtest.h>
#include <stdexcept>

using namespace r_type;

namespace {

KeyDerivation::SessionKeys testKeys()
{
    KeyDerivation::SessionKeys keys{};

    keys.clientToServer.fill(0x42);
    keys.serverToClient.fill(0x24);
    return keys;
}

std::vector<std::uint8_t> testPacket()
{
    return EncodeMessage(Command::PlayerInput,
        {{std::uint32_t{42}, std::uint8_t{1}, std::uint8_t{1}}},
        Role::Client, 1).front();
}

}

TEST(SecureTransportTest, ClientToServerRoundTrip)
{
    SecureTransport client(testKeys(), SecureTransport::Role::Client);
    SecureTransport server(testKeys(), SecureTransport::Role::Server);
    const auto original = testPacket();

    const auto encrypted = client.encode(original);
    const auto decrypted = server.decode(encrypted);

    EXPECT_EQ(decrypted, original);
    EXPECT_NE(encrypted, original);
}

TEST(SecureTransportTest, ServerToClientRoundTrip)
{
    SecureTransport server(testKeys(), SecureTransport::Role::Server);
    SecureTransport client(testKeys(), SecureTransport::Role::Client);
    const auto original = testPacket();

    EXPECT_EQ(client.decode(server.encode(original)), original);
}

TEST(SecureTransportTest, RejectReplayedPacket)
{
    SecureTransport client(testKeys(), SecureTransport::Role::Client);
    SecureTransport server(testKeys(), SecureTransport::Role::Server);
    const auto encrypted = client.encode(testPacket());

    ASSERT_NO_THROW(server.decode(encrypted));
    EXPECT_THROW(server.decode(encrypted), std::runtime_error);
}

TEST(SecureTransportTest, AcceptOutOfOrderPackets)
{
    SecureTransport client(testKeys(), SecureTransport::Role::Client);
    SecureTransport server(testKeys(), SecureTransport::Role::Server);
    const auto original = testPacket();

    const auto first = client.encode(original);
    const auto second = client.encode(original);
    const auto third = client.encode(original);

    EXPECT_EQ(server.decode(third), original);
    EXPECT_EQ(server.decode(first), original);
    EXPECT_EQ(server.decode(second), original);
}

TEST(SecureTransportTest, RejectModifiedSequence)
{
    SecureTransport client(testKeys(), SecureTransport::Role::Client);
    SecureTransport server(testKeys(), SecureTransport::Role::Server);
    const auto original = testPacket();
    auto encrypted = client.encode(original);

    encrypted[7] ^= 0x01;

    EXPECT_THROW(server.decode(encrypted), std::runtime_error);
}

TEST(SecureTransportTest, RejectWrongDirection)
{
    SecureTransport client(testKeys(), SecureTransport::Role::Client);
    SecureTransport anotherClient(testKeys(), SecureTransport::Role::Client);

    const auto encrypted = client.encode(testPacket());

    EXPECT_THROW(anotherClient.decode(encrypted), std::runtime_error);
}

TEST(SecureTransportTest, RejectTruncatedPacket)
{
    SecureTransport server(testKeys(), SecureTransport::Role::Server);
    std::vector<std::uint8_t> packet(8, 0);

    EXPECT_THROW(server.decode(packet), std::length_error);
}

TEST(SecureTransportTest, RejectOversizedPacket)
{
    SecureTransport server(testKeys(), SecureTransport::Role::Server);
    std::vector<std::uint8_t> packet(1201, 0);

    EXPECT_THROW(server.decode(packet), std::length_error);
}

TEST(SecureTransportTest, RejectZeroSequence)
{
    SecureTransport client(testKeys(), SecureTransport::Role::Client);
    SecureTransport server(testKeys(), SecureTransport::Role::Server);
    auto encrypted = client.encode(testPacket());

    for (std::size_t i = 0; i < SecureTransport::SEQUENCE_SIZE; ++i)
        encrypted[i] = 0;

    EXPECT_THROW(server.decode(encrypted), std::runtime_error);
}

TEST(SecureTransportTest, RejectModifiedPayload)
{
    SecureTransport client(testKeys(), SecureTransport::Role::Client);
    SecureTransport server(testKeys(), SecureTransport::Role::Server);
    auto encrypted = client.encode(testPacket());

    encrypted.back() ^= 0x01;

    EXPECT_THROW(server.decode(encrypted), std::runtime_error);
}

TEST(SecureTransportTest, InvalidPacketDoesNotConsumeSequence)
{
    SecureTransport client(testKeys(), SecureTransport::Role::Client);
    SecureTransport server(testKeys(), SecureTransport::Role::Server);
    const auto encrypted = client.encode(testPacket());
    auto corrupted = encrypted;

    corrupted.back() ^= 0x01;

    EXPECT_THROW(server.decode(corrupted), std::runtime_error);
    EXPECT_NO_THROW(server.decode(encrypted));
}

TEST(SecureTransportTest, GenerateDifferentSequences)
{
    SecureTransport client(testKeys(), SecureTransport::Role::Client);

    const auto first = client.encode(testPacket());
    const auto second = client.encode(testPacket());

    EXPECT_NE(std::vector<std::uint8_t>(first.begin(), first.begin() + 8),
        std::vector<std::uint8_t>(second.begin(), second.begin() + 8));
}

TEST(SecureTransportTest, RejectOversizedPlainDatagram)
{
    SecureTransport client(testKeys(), SecureTransport::Role::Client);
    std::vector<std::uint8_t> packet(1200, 0);

    EXPECT_THROW(client.encode(packet), std::length_error);
}
