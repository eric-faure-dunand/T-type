#include "args.hpp"

namespace r_type {


int ServerArgs::TestPort(std::string port) {
    int nb = 0;

    try {
        nb = std::stoi(port);
    } catch (const std::exception&) {
        throw Error(port + " : Not a valid port.");
    }
    if (nb == 0 || std::to_string(std::abs(nb)).size() != port.size() || nb > 65535)
        throw Error(port + " : Not a valid port.");
    return nb;
}

void ServerArgs::check(int argc, char **argv) {
    if (argc == 1)
        return;

    std::string flag = argv[1];

    if (flag ==  "-h" || flag == "--help")
        throw Warning(HELP_MESSAGE);
    else if (flag == "-p") {
        if (argc < 3)
            throw Error("PORT missing after -p.");
        port = TestPort(argv[2]);
    } else
        throw Error("Invalide arguments, try ./r-type_server --help.");
}

}
