/*
** EPITECH PROJECT, 2026
** T-type
** File description:
** BoundsSystem.hpp
*/

#ifndef SYSTEMS_BOUNDS_HPP
    #define SYSTEMS_BOUNDS_HPP

    #include <algorithm>
    #include <vector>

    #include "Registry.hpp"
    #include "Combat.hpp"
    #include "GameConfig.hpp"
    #include "Tags.hpp"
    #include "Transform.hpp"

namespace r_type::game {

void boundsSystem(ecs::Registry &r, float dt);

}

#endif
