/*
** EPITECH PROJECT, 2026
** T-type
** File description:
** MovementSystem.cpp
*/

#include "MovementSystem.hpp"

namespace r_type::game {

void movementSystem(ecs::Registry &r, float dt)
{
    r.each<Position, Velocity>([dt](Position &pos, const Velocity &vel) {
        pos.x += vel.x * dt;
        pos.y += vel.y * dt;
    });
}

}
