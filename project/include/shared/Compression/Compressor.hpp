/*
** EPITECH PROJECT, 2026
** T-type
** File description:
** Compressor.hpp
*/

#ifndef COMPRESSOR_HPP
#define COMPRESSOR_HPP

#include <string>

namespace r_type {

class Compressor {
public:
    Compressor() = default;
    ~Compressor() = default;

    std::string compress(const std::string& data) const;
    std::string decompress(const std::string& data) const;
};

}

#endif
