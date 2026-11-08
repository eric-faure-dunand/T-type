
/*
** EPITECH PROJECT, 2026
** T-type
** File description:
** SecureSessionManager.cpp
*/

#include "Network/SecureSessionManager.hpp"
#include <stdexcept>

namespace r_type {

SecureSessionManager::SecureSessionManager(SecureTransport::Role role):
    _role(role) {}

bool SecureSessionManager::install(const Endpoint& endpoint,
    const SecureSession& session) {
    if (!session.ready() || contains(endpoint))
        return false;
    _sessions.emplace(endpoint,
        std::make_unique<SecureTransport>(session.keys(), _role));
    return true;
}

bool SecureSessionManager::contains(const Endpoint& endpoint) const {
    return _sessions.contains(endpoint);
}

bool SecureSessionManager::remove(const Endpoint& endpoint) {
    return _sessions.erase(endpoint) != 0;
}

void SecureSessionManager::clear() {
    _sessions.clear();
}

std::size_t SecureSessionManager::size() const {
    return _sessions.size();
}

SecureTransport& SecureSessionManager::transport(const Endpoint& endpoint) {
    const auto it = _sessions.find(endpoint);
    if (it == _sessions.end())
        throw std::runtime_error("Secure session not established");
    return *it->second;
}

std::vector<std::uint8_t> SecureSessionManager::encode(
    const Endpoint& endpoint, std::span<const std::uint8_t> datagram) {
    return transport(endpoint).encode(datagram);
}

std::vector<std::uint8_t> SecureSessionManager::decode(
    const Endpoint& endpoint, std::span<const std::uint8_t> datagram) {
    return transport(endpoint).decode(datagram);
}

}
