/*
** EPITECH PROJECT, 2026
** T-type
** File description:
** test_secure_packet.cpp
*/

#include <gtest/gtest.h>
#include "Network/SecurePacket.hpp"

#include "Compression/Compressor.hpp"
#include "Encryption/Crypter.hpp"
#include "Network/protocol.hpp"

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

    EXPECT_THROW(
        packet.encode(std::string(r_type::MAX_PAYLOAD_SIZE + 1, 'A')),
        std::length_error
    );
}

TEST(SecurePacketTest, RoundTripEmptyPayload) {
    r_type::SecurePacket packet(testKey());

    EXPECT_EQ(packet.decode(packet.encode("")), "");
}

TEST(SecurePacketTest, RejectOversizedDecryptedPayload) {
    r_type::SecurePacket packet(testKey());
    r_type::Compressor compressor;
    r_type::Crypter crypter(testKey());

    const std::string compressed = compressor.compress(std::string(2000, 'A'));
    const std::string data = std::string(1, '\x01') + compressed;
    const std::string encrypted = crypter.encrypt(data);

    EXPECT_THROW(packet.decode(encrypted), std::length_error);
}

TEST(SecurePacketTest, RejectOversizedEncryptedPayload) {
    r_type::SecurePacket packet(testKey());

    EXPECT_THROW(packet.decode(std::string(1188, 'A')), std::length_error);
}

TEST(SecurePacketTest, AcceptMaximumCompressiblePayload) {
    r_type::SecurePacket packet(testKey());
    const std::string data(r_type::MAX_PAYLOAD_SIZE, 'A');

    EXPECT_EQ(packet.decode(packet.encode(data)), data);
}
