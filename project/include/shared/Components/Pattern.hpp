/*
** EPITECH PROJECT, 2026
** T-type
** File description:
** Pattern.hpp
*/

#ifndef COMPONENTS_PATTERN_HPP
    #define COMPONENTS_PATTERN_HPP

    #include <cstdint>

namespace r_type::game {

enum class PatternType : std::uint8_t {
    Straight,
    Sine,
    Zigzag,
    Bounce,
    Dive,
    Wall,
    Formation,
    Boss
};

struct Pattern {
    PatternType type = PatternType::Straight;
    float speed = 0.f;
    float baseY = 0.f;
    float age = 0.f;
    float direction = 1.f;
};

}

#endif
