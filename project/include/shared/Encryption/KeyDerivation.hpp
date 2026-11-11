/*
** EPITECH PROJECT, 2026
** T-type
** File description:
** KeyDerivation.hpp
*/

#ifndef KEY_DERIVATION_HPP
    #define KEY_DERIVATION_HPP

    #include "Encryption/Crypter.hpp"
    #include <array>
    #include <cstdint>

namespace r_type {

class KeyDerivation {
public:
    using Key = Crypter::Key;
    using Salt = std::array<std::uint8_t, 32>;

    struct SessionKeys {
        Key clientToServer;
        Key serverToClient;
    };

    static SessionKeys derive(const Key& sharedSecret, const Salt& salt);
};

}

#endif
