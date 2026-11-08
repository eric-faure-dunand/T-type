/*
** EPITECH PROJECT, 2026
** T-type
** File description:
** test_secure_datagram.cpp
*/

#include <gtest/gtest.h>
#include "Network/SecureDatagram.hpp"
#include "Network/packet.hpp"
#include <algorithm>
#include <stdexcept>
#include <array>

namespace {

r_type::Crypter::Key testKey() {
    r_type::Crypter::Key key{};
    key.fill(0x42);
    return key;
}

TEST(SecureDatagramTest, RoundTripPacket) {
    r_type::SecureDatagram secure(testKey());
    const auto original = r_type::EncodePacket(
        r_type::Command::PlayerInput,
        {{std::uint32_t{42}, std::uint8_t{1}, std::uint8_t{1}}},
        r_type::Role::Client
    );

    const auto encrypted = secure.encode(original);
    const auto restored = secure.decode(encrypted);
    const auto decoded = r_type::DecodePacket(restored, r_type::Role::Server);

    EXPECT_NE(encrypted, original);
    EXPECT_EQ(restored, original);
    ASSERT_TRUE(decoded.ok);
    EXPECT_EQ(decoded.command, r_type::Command::PlayerInput);
}

TEST(SecureDatagramTest, RejectModifiedHeader) {
    r_type::SecureDatagram secure(testKey());
    const auto original = r_type::EncodePacket(
        r_type::Command::PlayerInput,
        {{std::uint32_t{42}, std::uint8_t{1}, std::uint8_t{1}}},
        r_type::Role::Client
    );

    auto encrypted = secure.encode(original);
    encrypted[3] ^= 0x01;

    EXPECT_THROW(secure.decode(encrypted), std::runtime_error);
}

TEST(SecureDatagramTest, RejectModifiedPayload) {
    r_type::SecureDatagram secure(testKey());
    const auto original = r_type::EncodePacket(
        r_type::Command::PlayerInput,
        {{std::uint32_t{42}, std::uint8_t{1}, std::uint8_t{1}}},
        r_type::Role::Client
    );

    auto encrypted = secure.encode(original);
    encrypted.back() ^= 0x01;

    EXPECT_THROW(secure.decode(encrypted), std::runtime_error);
}

TEST(SecureDatagramTest, RejectPlainPacket) {
    r_type::SecureDatagram secure(testKey());
    const auto original = r_type::EncodePacket(
        r_type::Command::PlayerInput,
        {{std::uint32_t{42}, std::uint8_t{1}, std::uint8_t{1}}},
        r_type::Role::Client
    );

    EXPECT_THROW(secure.decode(original), std::runtime_error);
}

TEST(SecureDatagramTest, RejectWrongKey) {
    r_type::SecureDatagram sender(testKey());
    auto wrongKey = testKey();
    wrongKey[0] ^= 0xFF;
    r_type::SecureDatagram receiver(wrongKey);

    const auto original = r_type::EncodePacket(
        r_type::Command::PlayerInput,
        {{std::uint32_t{42}, std::uint8_t{1}, std::uint8_t{1}}},
        r_type::Role::Client
    );

    EXPECT_THROW(receiver.decode(sender.encode(original)), std::runtime_error);
}

}

TEST(SecureDatagramTest, PreserveErrorCommand) {
    r_type::SecureDatagram secure(testKey());
    const auto original = r_type::EncodePacket(
        r_type::Command::Error,
        {{std::uint8_t{1}, std::uint8_t{2}}},
        r_type::Role::Server
    );

    const auto encrypted = secure.encode(original);
    const auto restored = secure.decode(encrypted);
    const auto decoded = r_type::DecodePacket(restored, r_type::Role::Client);

    EXPECT_EQ(encrypted[2], static_cast<std::uint8_t>(r_type::Command::Error));
    EXPECT_EQ(restored, original);
    ASSERT_TRUE(decoded.ok);
    EXPECT_EQ(decoded.command, r_type::Command::Error);
}


TEST(SecureDatagramTest, RoundTripWithExtraAAD)
{
    r_type::SecureDatagram secure(testKey());
    const auto original = r_type::EncodePacket(
        r_type::Command::PlayerInput,
        {{std::uint32_t{42}, std::uint8_t{1}, std::uint8_t{1}}},
        r_type::Role::Client
    );
    const std::array<std::uint8_t, 8> aad = {0, 0, 0, 0, 0, 0, 0, 1};

    const auto encrypted = secure.encode(original, aad);
    const auto restored = secure.decode(encrypted, aad);

    EXPECT_EQ(restored, original);
}

TEST(SecureDatagramTest, RejectWrongExtraAAD)
{
    r_type::SecureDatagram secure(testKey());
    const auto original = r_type::EncodePacket(
        r_type::Command::PlayerInput,
        {{std::uint32_t{42}, std::uint8_t{1}, std::uint8_t{1}}},
        r_type::Role::Client
    );
    const std::array<std::uint8_t, 8> aad = {0, 0, 0, 0, 0, 0, 0, 1};
    const std::array<std::uint8_t, 8> wrongAAD = {0, 0, 0, 0, 0, 0, 0, 2};

    const auto encrypted = secure.encode(original, aad);

    EXPECT_THROW(secure.decode(encrypted, wrongAAD), std::runtime_error);
}

TEST(SecureDatagramTest, RejectMissingExtraAAD)
{
    r_type::SecureDatagram secure(testKey());
    const auto original = r_type::EncodePacket(
        r_type::Command::PlayerInput,
        {{std::uint32_t{42}, std::uint8_t{1}, std::uint8_t{1}}},
        r_type::Role::Client
    );
    const std::array<std::uint8_t, 8> aad = {0, 0, 0, 0, 0, 0, 0, 1};

    const auto encrypted = secure.encode(original, aad);

    EXPECT_THROW(secure.decode(encrypted), std::runtime_error);
}
