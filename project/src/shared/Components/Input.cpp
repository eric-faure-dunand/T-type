/*
** EPITECH PROJECT, 2026
** T-type
** File description:
** Input.cpp
*/

#include "Input.hpp"

namespace r_type::game {

void applyAction(Input &input, std::uint8_t action, bool pressed)
{
    switch (static_cast<Action>(action)) {
        case Action::Up:
            input.up = pressed;
            break;
        case Action::Down:
            input.down = pressed;
            break;
        case Action::Left:
            input.left = pressed;
            break;
        case Action::Right:
            input.right = pressed;
            break;
        case Action::Fire:
            input.shoot = pressed;
            break;
    }
}

}
