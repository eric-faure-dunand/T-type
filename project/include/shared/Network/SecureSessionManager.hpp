/*
** EPITECH PROJECT, 2026
** T-type
** File description:
** SecureSessionManager.hpp
*/

#ifndef SECURE_SESSION_MANAGER_HPP
    #define SECURE_SESSION_MANAGER_HPP

    #include "Encryption/SecureSession.hpp"
    #include "Network/SecureTransport.hpp"
    #include <asio/ip/udp.hpp>
    #include <cstddef>
    #include <map>
    #include <memory>
    #include <span>
    #include <vector>

namespace r_type {

class SecureSessionManager {
public:
    using Endpoint = asio::ip::udp::endpoint;

    explicit SecureSessionManager(SecureTransport::Role role);

    bool install(const Endpoint& endpoint, const SecureSession& session);
    bool contains(const Endpoint& endpoint) const;
    bool remove(const Endpoint& endpoint);
    void clear();
    std::size_t size() const;

    std::vector<std::uint8_t> encode(const Endpoint& endpoint,
        std::span<const std::uint8_t> datagram);
    std::vector<std::uint8_t> decode(const Endpoint& endpoint,
        std::span<const std::uint8_t> datagram);

private:
    SecureTransport& transport(const Endpoint& endpoint);

    SecureTransport::Role _role;
    std::map<Endpoint, std::unique_ptr<SecureTransport>> _sessions;
};

}

#endif
