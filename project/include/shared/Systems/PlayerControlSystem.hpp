/*
** EPITECH PROJECT, 2026
** T-type
** File description:
** PlayerControlSystem.hpp
*/

#ifndef SYSTEMS_PLAYER_CONTROL_HPP
    #define SYSTEMS_PLAYER_CONTROL_HPP

    #include "Registry.hpp"
    #include "GameConfig.hpp"
    #include "Input.hpp"
    #include "Tags.hpp"
    #include "Transform.hpp"

namespace r_type::game {

void playerControlSystem(ecs::Registry &r, float dt);

}

#endif
