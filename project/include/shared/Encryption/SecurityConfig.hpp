/*
** EPITECH PROJECT, 2026
** T-type
** File description:
** SecurityConfig.hpp
*/

#ifndef SECURITY_CONFIG_HPP
    #define SECURITY_CONFIG_HPP

    #include "Encryption/ServerIdentity.hpp"
    #include <string>

namespace r_type {

class SecurityConfig {
public:
    static std::string serverPrivateKeyPath();
    static ServerIdentity::PublicKey trustedServerKey();
};

}

#endif
