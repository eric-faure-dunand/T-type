/*
** EPITECH PROJECT, 2026
** T-type
** File description:
** HealthSystem.hpp
*/

#ifndef SYSTEMS_HEALTH_HPP
    #define SYSTEMS_HEALTH_HPP

    #include <vector>

    #include "Registry.hpp"
    #include "Combat.hpp"

namespace r_type::game {

void healthSystem(ecs::Registry &r, float dt);

}

#endif
