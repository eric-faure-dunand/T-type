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

enum InputFlag : std::uint8_t {
    InputUp = 1 << 0,
    InputDown = 1 << 1,
    InputLeft = 1 << 2,
    InputRight = 1 << 3,
    InputShoot = 1 << 4
};

inline std::uint8_t toFlags(const Input &in) noexcept
{
    return static_cast<std::uint8_t>((in.up ? InputUp : 0) | (in.down ? InputDown : 0)
        | (in.left ? InputLeft : 0) | (in.right ? InputRight : 0) | (in.shoot ? InputShoot : 0));
}

inline Input fromFlags(std::uint8_t flags) noexcept
{
    return Input{(flags & InputUp) != 0, (flags & InputDown) != 0, (flags & InputLeft) != 0,
        (flags & InputRight) != 0, (flags & InputShoot) != 0};
}

}

#endif
