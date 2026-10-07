#include "main.hpp"

int main(int argc, char **argv) {
    try {
        r_type::ServerArgs args(argc, argv);
    } catch (const IError &e) {
        if (e.code() == 84)
            std::cerr << e;
        else
            std::cout << e;
        return e.code();
    }
    return 0;
}

