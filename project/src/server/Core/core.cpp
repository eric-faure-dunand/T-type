#include "core.hpp"

namespace r_type {

Core::Core(std::uint16_t port): _socket(Role::Server) {
    _socket.Bind(port);
}

void Core::Run() {
    while (SERVER_RUN)
        _socket.Poll();
}

}
