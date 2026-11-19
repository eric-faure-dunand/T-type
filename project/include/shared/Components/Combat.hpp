/*
** EPITECH PROJECT, 2026
** T-type
** File description:
** Combat.hpp
*/

#ifndef COMPONENTS_COMBAT_HPP
    #define COMPONENTS_COMBAT_HPP

namespace r_type::game {

struct Health {
    int current = 1;
    int max = 1;
};

struct Damage {
    int amount = 1;
};

struct Hitbox {
    float width = 0.f;
    float height = 0.f;
};

}

#endif
