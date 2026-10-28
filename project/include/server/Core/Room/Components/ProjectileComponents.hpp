/*
** EPITECH PROJECT, 2026
** T-type
** File description:
** ProjectileComponents.hpp
*/

#ifndef PROJECTILE_COMPONENTS_HPP
#define PROJECTILE_COMPONENTS_HPP

#include <cstdint>

namespace r_type {

struct Projectile {
    std::uint32_t ownerId = 0;
    std::int32_t speed = 10;
};

}

#endif
