#include "main.hpp"

int main(int argc, char **argv) {
    signal(SIGPIPE, SIG_IGN);
    signal(SIGINT, sigint);
    try {
        r_type::ServerArgs args(argc, argv);
        r_type::Core core(static_cast<std::uint16_t>(args.GetPort()));
        core.Run();
    } catch (const IError &e) {
        if (e.code() == 84)
            std::cerr << e;
        else
            std::cout << e;
        return e.code();
    }
    return 0;
}

