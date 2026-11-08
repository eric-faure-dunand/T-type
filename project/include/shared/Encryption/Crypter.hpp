/*
** EPITECH PROJECT, 2026
** T-type
** File description:
** Crypter.hpp
*/

#ifndef CRYPTER_HPP
#define CRYPTER_HPP

#include <array>
#include <cstdint>
#include <string>

namespace r_type {

class Crypter {
public:
    using Key = std::array<std::uint8_t, 32>;

    explicit Crypter(const Key& key);
    ~Crypter() = default;

    std::string encrypt(const std::string& data) const;
    std::string decrypt(const std::string& data) const;

private:
    Key _key;
};

}

#endif
