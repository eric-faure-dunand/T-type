/*
** EPITECH PROJECT, 2026
** T-type
** File description:
** MovementSystem.hpp
*/

#ifndef SYSTEMS_MOVEMENT_HPP
    #define SYSTEMS_MOVEMENT_HPP

    #include "Registry.hpp"
    #include "Transform.hpp"

namespace r_type::game {

inline void movementSystem(ecs::Registry &r, float dt)
{
    r.each<Position, Velocity>([dt](Position &pos, const Velocity &vel) {
        pos.x += vel.x * dt;
        pos.y += vel.y * dt;
    });
}

}

#endif
