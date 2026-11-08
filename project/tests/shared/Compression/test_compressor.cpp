/*
** EPITECH PROJECT, 2026
** T-type
** File description:
** test_compressor.cpp
*/

#include "Compression/Compressor.hpp"
#include <gtest/gtest.h>
#include <stdexcept>
#include <string>

TEST(CompressorTest, CompressAndDecompressText) {
    r_type::Compressor compressor;
    const std::string original = "Hello R-Type multiplayer!";

    const std::string compressed = compressor.compress(original);
    EXPECT_EQ(compressor.decompress(compressed), original);
}

TEST(CompressorTest, CompressAndDecompressRepeatedData) {
    r_type::Compressor compressor;
    const std::string original(1000, 'A');

    const std::string compressed = compressor.compress(original);

    EXPECT_LT(compressed.size(), original.size());
    EXPECT_EQ(compressor.decompress(compressed), original);
}

TEST(CompressorTest, CompressAndDecompressBinaryData) {
    r_type::Compressor compressor;
    const std::string original("\x00\x01\x02\xFF\x00\x80", 6);

    const std::string compressed = compressor.compress(original);
    EXPECT_EQ(compressor.decompress(compressed), original);
}

TEST(CompressorTest, CompressAndDecompressEmptyData) {
    r_type::Compressor compressor;
    const std::string original;

    const std::string compressed = compressor.compress(original);
    EXPECT_EQ(compressor.decompress(compressed), original);
}

TEST(CompressorTest, RejectInvalidCompressedData) {
    r_type::Compressor compressor;

    EXPECT_THROW(compressor.decompress("abc"), std::runtime_error);
}


TEST(CompressorTest, RejectOversizedDecompressedData) {
    r_type::Compressor compressor;
    const std::string invalidData("\x01\x00\x10\x00", 4);

    EXPECT_THROW(compressor.decompress(invalidData), std::length_error);
}


TEST(CompressorTest, RejectCorruptedCompressedData) {
    r_type::Compressor compressor;
    const std::string original(1000, 'A');
    std::string compressed = compressor.compress(original);

    compressed.resize(4);

    EXPECT_THROW(compressor.decompress(compressed), std::runtime_error);
}
