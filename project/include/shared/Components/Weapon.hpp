/*
** EPITECH PROJECT, 2026
** T-type
** File description:
** Weapon.hpp
*/

#ifndef COMPONENTS_WEAPON_HPP
    #define COMPONENTS_WEAPON_HPP

namespace r_type::game {

struct Weapon {
    float fireRate = 5.f;
    float cooldown = 0.f;
    float projectileSpeed = 600.f;
    int damage = 1;
};

}

#endif
