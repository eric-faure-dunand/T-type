#ifndef CORE_HPP
    #define CORE_HPP

    #include <cstdint>

    #include "network.hpp"

namespace r_type {

class Core {
    UdpSocket _socket;

public:
    explicit Core(std::uint16_t port = DEFAULT_PORT);
    ~Core() = default;

    void Run();
};

}

#endif
