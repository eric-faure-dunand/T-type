/*
** EPITECH PROJECT, 2026
** T-type
** File description:
** PlayerComponents.hpp
*/

#ifndef PLAYER_COMPONENTS_HPP
#define PLAYER_COMPONENTS_HPP

#include <cstdint>

namespace r_type {

struct Position {
    std::int32_t x = 0;
    std::int32_t y = 0;
};

struct Player {
    std::uint8_t color = 0;
};

}

#endif
