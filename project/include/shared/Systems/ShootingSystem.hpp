/*
** EPITECH PROJECT, 2026
** T-type
** File description:
** ShootingSystem.hpp
*/

#ifndef SYSTEMS_SHOOTING_HPP
    #define SYSTEMS_SHOOTING_HPP

    #include <vector>

    #include "Registry.hpp"
    #include "Combat.hpp"
    #include "GameConfig.hpp"
    #include "Input.hpp"
    #include "Spawner.hpp"
    #include "Tags.hpp"
    #include "Transform.hpp"
    #include "Weapon.hpp"

namespace r_type::game {

void shootingSystem(ecs::Registry &r, float dt);

}

#endif
