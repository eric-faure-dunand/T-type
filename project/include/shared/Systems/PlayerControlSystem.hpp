/*
** EPITECH PROJECT, 2026
** T-type
** File description:
** PlayerControlSystem.hpp
*/

#ifndef SYSTEMS_PLAYER_CONTROL_HPP
    #define SYSTEMS_PLAYER_CONTROL_HPP

    #include "shared/ECS/Registry.hpp"
    #include "shared/Components/Input.hpp"
    #include "shared/Components/Tags.hpp"
    #include "shared/Components/Transform.hpp"
    #include "shared/Game/GameConfig.hpp"

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
