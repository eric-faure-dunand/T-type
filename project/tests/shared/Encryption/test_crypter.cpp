/*
** EPITECH PROJECT, 2026
** T-type
** File description:
** test_crypter.cpp
*/

#include "Encryption/Crypter.hpp"
#include <gtest/gtest.h>
#include <stdexcept>
#include <string>

TEST(CrypterTest, EncryptAndDecryptText) {
    r_type::Crypter::Key key{};
    r_type::Crypter crypter(key);
    const std::string original = "Hello R-Type multiplayer!";

    EXPECT_EQ(crypter.decrypt(crypter.encrypt(original)), original);
}

TEST(CrypterTest, EncryptAndDecryptBinaryData) {
    r_type::Crypter::Key key{};
    r_type::Crypter crypter(key);
    const std::string original("\x00\x01\xFF\x80\x00", 5);

    EXPECT_EQ(crypter.decrypt(crypter.encrypt(original)), original);
}

TEST(CrypterTest, EncryptAndDecryptEmptyData) {
    r_type::Crypter::Key key{};
    r_type::Crypter crypter(key);

    EXPECT_EQ(crypter.decrypt(crypter.encrypt("")), "");
}

TEST(CrypterTest, RejectInvalidEncryptedData) {
    r_type::Crypter::Key key{};
    r_type::Crypter crypter(key);

    EXPECT_THROW(crypter.decrypt("abc"), std::runtime_error);
}

TEST(CrypterTest, RejectModifiedEncryptedData) {
    r_type::Crypter::Key key{};
    r_type::Crypter crypter(key);
    std::string encrypted = crypter.encrypt("Hello");

    encrypted[12] ^= 0x01;

    EXPECT_THROW(crypter.decrypt(encrypted), std::runtime_error);
}

TEST(CrypterTest, RejectWrongKey) {
    r_type::Crypter::Key key1{};
    r_type::Crypter::Key key2{};
    key2[0] = 1;

    r_type::Crypter sender(key1);
    r_type::Crypter receiver(key2);

    EXPECT_THROW(receiver.decrypt(sender.encrypt("Hello")), std::runtime_error);
}

TEST(CrypterTest, GenerateDifferentNonces) {
    r_type::Crypter::Key key{};
    r_type::Crypter crypter(key);

    EXPECT_NE(crypter.encrypt("Hello"), crypter.encrypt("Hello"));
}


TEST(CrypterTest, RejectModifiedNonce) {
    r_type::Crypter::Key key{};
    r_type::Crypter crypter(key);
    std::string encrypted = crypter.encrypt("Hello");

    encrypted[0] ^= 0x01;

    EXPECT_THROW(crypter.decrypt(encrypted), std::runtime_error);
}

TEST(CrypterTest, RejectModifiedTag) {
    r_type::Crypter::Key key{};
    r_type::Crypter crypter(key);
    std::string encrypted = crypter.encrypt("Hello");

    encrypted.back() ^= 0x01;

    EXPECT_THROW(crypter.decrypt(encrypted), std::runtime_error);
}

TEST(CrypterTest, RejectOversizedData) {
    r_type::Crypter::Key key{};
    r_type::Crypter crypter(key);
    const std::string oversized(1024 * 1024 + 1, 'A');

    EXPECT_THROW(crypter.encrypt(oversized), std::length_error);
}
