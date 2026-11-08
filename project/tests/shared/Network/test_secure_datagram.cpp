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
