/*
** EPITECH PROJECT, 2026
** T-type
** File description:
** test_secure_pipeline.cpp
*/

#include "Compression/Compressor.hpp"
#include "Encryption/Crypter.hpp"
#include <gtest/gtest.h>
#include <string>

TEST(SecurePipelineTest, RestoreOriginalData) {
    r_type::Compressor compressor;
    r_type::Crypter::Key key{};
    r_type::Crypter crypter(key);

    const std::string original(1000, 'A');

    const std::string compressed = compressor.compress(original);
    const std::string encrypted = crypter.encrypt(compressed);
    const std::string decrypted = crypter.decrypt(encrypted);
    const std::string restored = compressor.decompress(decrypted);

    EXPECT_EQ(restored, original);
}

TEST(SecurePipelineTest, RestoreBinaryData) {
    r_type::Compressor compressor;
    r_type::Crypter::Key key{};
    r_type::Crypter crypter(key);

    const std::string original("\x00\x01\xFF\x80\x00", 5);

    const std::string compressed = compressor.compress(original);
    const std::string encrypted = crypter.encrypt(compressed);
    const std::string decrypted = crypter.decrypt(encrypted);
    const std::string restored = compressor.decompress(decrypted);

    EXPECT_EQ(restored, original);
}

TEST(SecurePipelineTest, RejectTamperedPacket) {
    r_type::Compressor compressor;
    r_type::Crypter::Key key{};
    r_type::Crypter crypter(key);

    std::string encrypted = crypter.encrypt(compressor.compress("Hello"));
    encrypted.back() ^= 0x01;

    EXPECT_THROW(crypter.decrypt(encrypted), std::runtime_error);
}
