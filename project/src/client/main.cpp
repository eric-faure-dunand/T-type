/*
** EPITECH PROJECT, 2026
** T-type
** File description:
** main.cpp
*/

#include <exception>
#include <iostream>

#include "ClientCore.hpp"
#include "Error.hpp"

int main(int argc, char **argv) {
    try {
        r_type::client::ClientCore core(argc > 1 ? argv[1] : "127.0.0.1");
        core.Run();
    } catch (const IError &e) {
        std::cerr << e;
        return 84;
    } catch (const std::exception &e) {
        std::cerr << "Error : " << e.what() << std::endl;
        return 84;
    }
    return 0;
}
