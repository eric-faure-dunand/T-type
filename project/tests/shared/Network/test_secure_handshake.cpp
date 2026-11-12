/*
** EPITECH PROJECT, 2026
** T-type
** File description:
** test_secure_handshake.cpp
*/

#include "Network/SecureHandshake.hpp"
#include <gtest/gtest.h>
#include <stdexcept>

using namespace r_type;

namespace {

SecureSession::Request testRequest()
{
    SecureSession::Request request{};

    request.publicKey.fill(0x42);
    request.nonce.fill(0x24);
    return request;
}

SecureSession::Response testResponse()
{
    SecureSession::Response response{};

    response.publicKey.fill(0x42);
    response.nonce.fill(0x24);
    response.signature.fill(0x13);
    return response;
}

}

TEST(SecureHandshakeTest, EncodeAndDecodeRequest)
{
    const auto original = testRequest();
    const auto encoded = SecureHandshake::encode(original);
    const auto decoded = SecureHandshake::decodeRequest(encoded);

    EXPECT_EQ(encoded.size(), 68U);
    EXPECT_EQ(SecureHandshake::type(encoded), SecureHandshake::Type::Request);
    EXPECT_EQ(decoded.publicKey, original.publicKey);
    EXPECT_EQ(decoded.nonce, original.nonce);
}

TEST(SecureHandshakeTest, EncodeAndDecodeResponse)
{
    const auto original = testResponse();
    const auto encoded = SecureHandshake::encode(original);
    const auto decoded = SecureHandshake::decodeResponse(encoded);

    EXPECT_EQ(encoded.size(), 132U);
    EXPECT_EQ(SecureHandshake::type(encoded), SecureHandshake::Type::Response);
    EXPECT_EQ(decoded.publicKey, original.publicKey);
    EXPECT_EQ(decoded.nonce, original.nonce);
    EXPECT_EQ(decoded.signature, original.signature);
}

TEST(SecureHandshakeTest, EncodeAndDecodeConfirmation)
{
    SecureSession::Confirmation original{};

    original.fill(0x42);
    const auto encoded = SecureHandshake::encode(original);
    const auto decoded = SecureHandshake::decodeConfirmation(encoded);

    EXPECT_EQ(encoded.size(), 36U);
    EXPECT_EQ(SecureHandshake::type(encoded),
        SecureHandshake::Type::Confirmation);
    EXPECT_EQ(decoded, original);
}

TEST(SecureHandshakeTest, RejectInvalidVersion)
{
    auto encoded = SecureHandshake::encode(testRequest());

    encoded[0] = 2;

    EXPECT_THROW(SecureHandshake::decodeRequest(encoded), std::runtime_error);
}

TEST(SecureHandshakeTest, RejectUnknownType)
{
    auto encoded = SecureHandshake::encode(testRequest());

    encoded[1] = 99;

    EXPECT_THROW(SecureHandshake::type(encoded), std::runtime_error);
}

TEST(SecureHandshakeTest, RejectTruncatedMessage)
{
    auto encoded = SecureHandshake::encode(testRequest());

    encoded.pop_back();

    EXPECT_THROW(SecureHandshake::decodeRequest(encoded), std::runtime_error);
}

TEST(SecureHandshakeTest, RejectOversizedMessage)
{
    auto encoded = SecureHandshake::encode(testResponse());

    encoded.push_back(0);

    EXPECT_THROW(SecureHandshake::decodeResponse(encoded), std::runtime_error);
}

TEST(SecureHandshakeTest, RejectWrongMessageType)
{
    const auto encoded = SecureHandshake::encode(testRequest());

    EXPECT_THROW(SecureHandshake::decodeResponse(encoded), std::runtime_error);
}

TEST(SecureHandshakeTest, RejectEmptyMessage)
{
    const std::vector<std::uint8_t> empty;

    EXPECT_THROW(SecureHandshake::type(empty), std::runtime_error);
}

TEST(SecureHandshakeTest, RejectHeaderWithoutType)
{
    const std::vector<std::uint8_t> header = {1};

    EXPECT_THROW(SecureHandshake::type(header), std::runtime_error);
}

TEST(SecureHandshakeTest, RejectInvalidMagic)
{
    SecureSession::Request request{};
    auto encoded = SecureHandshake::encode(request);

    encoded[0] = 0x00;
    EXPECT_THROW(SecureHandshake::type(encoded), std::runtime_error);

    encoded = SecureHandshake::encode(request);
    encoded[1] = 0x00;
    EXPECT_THROW(SecureHandshake::type(encoded), std::runtime_error);
}

TEST(SecureHandshakeTest, EncodeAndDecodeReady)
{
    const auto data = SecureHandshake::encodeReady();

    EXPECT_EQ(data.size(), 4);
    EXPECT_EQ(SecureHandshake::type(data), SecureHandshake::Type::Ready);
    EXPECT_NO_THROW(SecureHandshake::decodeReady(data));
}

TEST(SecureHandshakeTest, RejectInvalidReady)
{
    auto data = SecureHandshake::encodeReady();
    data.push_back(0x00);

    EXPECT_THROW(SecureHandshake::decodeReady(data), std::runtime_error);
}
