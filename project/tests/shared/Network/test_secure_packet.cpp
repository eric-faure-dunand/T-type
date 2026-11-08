/*
** EPITECH PROJECT, 2026
** T-type
** File description:
** test_secure_packet.cpp
*/

#include <gtest/gtest.h>
#include "Network/SecurePacket.hpp"

namespace {

r_type::Crypter::Key testKey() {
    r_type::Crypter::Key key{};
    key.fill(0x42);
    return key;
}

}

TEST(SecurePacketTest, RoundTripText) {
    r_type::SecurePacket packet(testKey());
    const std::string data = "Hello secure network";

    EXPECT_EQ(packet.decode(packet.encode(data)), data);
}

TEST(SecurePacketTest, RoundTripCompressedData) {
    r_type::SecurePacket packet(testKey());
    const std::string data(1000, 'A');

    EXPECT_EQ(packet.decode(packet.encode(data)), data);
}

TEST(SecurePacketTest, RoundTripBinaryData) {
    r_type::SecurePacket packet(testKey());
    const std::string data("\0\x01\x02\xFF", 4);

    EXPECT_EQ(packet.decode(packet.encode(data)), data);
}

TEST(SecurePacketTest, RejectWrongAAD) {
    r_type::SecurePacket packet(testKey());
    const std::string encrypted = packet.encode("Hello", "header");

    EXPECT_THROW(packet.decode(encrypted, "modified"), std::runtime_error);
}

TEST(SecurePacketTest, RejectTamperedData) {
    r_type::SecurePacket packet(testKey());
    std::string encrypted = packet.encode("Hello");

    encrypted[15] ^= 0x01;

    EXPECT_THROW(packet.decode(encrypted), std::runtime_error);
}

TEST(SecurePacketTest, RejectOversizedPayload) {
    r_type::SecurePacket packet(testKey());

    EXPECT_THROW(packet.encode(std::string(1160, 'A')), std::length_error);
}

TEST(SecurePacketTest, RoundTripEmptyPayload) {
    r_type::SecurePacket packet(testKey());

    EXPECT_EQ(packet.decode(packet.encode("")), "");
}