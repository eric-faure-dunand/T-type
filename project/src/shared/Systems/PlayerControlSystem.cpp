/*
** EPITECH PROJECT, 2026
** T-type
** File description:
** PlayerControlSystem.cpp
*/

#include "PlayerControlSystem.hpp"

namespace r_type::game {

void playerControlSystem(ecs::Registry &r, float)
{
    r.each<Player, Input, Velocity>([](Player &, const Input &in, Velocity &vel) {
        vel.x = (static_cast<float>(in.right) - static_cast<float>(in.left)) * kPlayerSpeed;
        vel.y = (static_cast<float>(in.down) - static_cast<float>(in.up)) * kPlayerSpeed;
    });
}

}
