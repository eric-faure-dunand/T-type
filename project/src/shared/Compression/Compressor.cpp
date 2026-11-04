/*
** EPITECH PROJECT, 2026
** T-type
** File description:
** Compressor.cpp
*/

#include "Compression/Compressor.hpp"
#include <lz4.h>
#include <cstdint>
#include <cstring>
#include <limits>
#include <stdexcept>

namespace r_type {

std::string Compressor::compress(const std::string& data) const {
    if (data.size() > static_cast<std::size_t>(LZ4_MAX_INPUT_SIZE))
        throw std::length_error("Data too large to compress");

    const int inputSize = static_cast<int>(data.size());
    const int maxSize = LZ4_compressBound(inputSize);
    std::string result(4 + static_cast<std::size_t>(maxSize), '\0');

    const std::uint32_t originalSize = static_cast<std::uint32_t>(inputSize);
    for (int i = 0; i < 4; ++i)
        result[i] = static_cast<char>((originalSize >> (i * 8)) & 0xFF);

    const int compressedSize = LZ4_compress_default(
        data.data(), result.data() + 4, inputSize, maxSize
    );

    if (compressedSize <= 0)
        throw std::runtime_error("LZ4 compression failed");

    result.resize(4 + static_cast<std::size_t>(compressedSize));
    return result;
}

std::string Compressor::decompress(const std::string& data) const {
    if (data.size() < 4)
        throw std::runtime_error("Invalid compressed data");

    std::uint32_t originalSize = 0;
    for (int i = 0; i < 4; ++i)
        originalSize |= static_cast<std::uint32_t>(
            static_cast<unsigned char>(data[i])) << (i * 8);

    constexpr std::uint32_t maxDecompressedSize = 1024 * 1024;

    if (originalSize > maxDecompressedSize)
        throw std::length_error("Decompressed data too large");

    if (data.size() - 4 > static_cast<std::size_t>(std::numeric_limits<int>::max()))
        throw std::length_error("Compressed data too large");

    std::string result(originalSize, '\0');
    const int decodedSize = LZ4_decompress_safe(
        data.data() + 4,
        result.data(),
        static_cast<int>(data.size() - 4),
        static_cast<int>(originalSize)
    );

    if (decodedSize != static_cast<int>(originalSize))
        throw std::runtime_error("LZ4 decompression failed");

    return result;
}

}
