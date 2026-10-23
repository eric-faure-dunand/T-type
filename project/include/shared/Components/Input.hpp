/*
** EPITECH PROJECT, 2026
** T-type
** File description:
** Input.hpp
*/

#ifndef COMPONENTS_INPUT_HPP
    #define COMPONENTS_INPUT_HPP

    #include <cstdint>

namespace r_type::game {

struct Input {
    bool up = false;
    bool down = false;
    bool left = false;
    bool right = false;
    bool shoot = false;
};

enum class Action : std::uint8_t {
    Up = 0x00,
    Down = 0x01,
    Left = 0x02,
    Right = 0x03,
    Fire = 0x04
};

void applyAction(Input &input, std::uint8_t action, bool pressed);

}

#endif
