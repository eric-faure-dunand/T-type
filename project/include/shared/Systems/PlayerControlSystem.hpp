/*
** EPITECH PROJECT, 2026
** T-type
** File description:
** PlayerControlSystem.hpp
*/

#ifndef SYSTEMS_PLAYER_CONTROL_HPP
    #define SYSTEMS_PLAYER_CONTROL_HPP

    #include "Registry.hpp"
    #include "Input.hpp"
    #include "Tags.hpp"
    #include "Transform.hpp"
    #include "GameConfig.hpp"

namespace r_type::game {

inline void playerControlSystem(ecs::Registry &r, float)
{
    r.each<Player, Input, Velocity>([](Player &, const Input &in, Velocity &vel) {
        vel.x = (static_cast<float>(in.right) - static_cast<float>(in.left)) * kPlayerSpeed;
        vel.y = (static_cast<float>(in.down) - static_cast<float>(in.up)) * kPlayerSpeed;
    });
}

}

#endif
