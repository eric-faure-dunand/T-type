#ifndef ARGS_HPP
    #define ARGS_HPP

    #define HELP_MESSAGE "USAGE\n\t./r-type_server [-p <PORT>/-h]\n\nDESCRIPTION\nThe server for the R-type project\n\nOPTION\n\t-p <PORT> :\t set your port, default 6767\n\t-h, --help :\t show this help\n"

    #include <iostream>
    #include <string.h>
    #include <string>

    #include "network.hpp"
    #include "Error.hpp"
    #include "Warning.hpp"

namespace r_type {

class ServerArgs {
    int port = -1;

    int TestPort(std::string port);
    void check(int argc, char **argv);
public:
    ServerArgs(int argc, char **argv) {check(argc, argv);};
    ~ServerArgs() = default;

    int GetPort() {if (port != -1) return port; return DEFAULT_PORT;}
};

}

#endif