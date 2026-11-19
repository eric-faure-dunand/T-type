/*
** EPITECH PROJECT, 2026
** T-type
** File description:
** PatternSystem.hpp
*/

#ifndef SYSTEMS_PATTERN_HPP
    #define SYSTEMS_PATTERN_HPP

    #include <cmath>
    #include <optional>

    #include "Registry.hpp"
    #include "Combat.hpp"
    #include "GameConfig.hpp"
    #include "Pattern.hpp"
    #include "Tags.hpp"
    #include "Transform.hpp"

namespace r_type::game {

void patternSystem(ecs::Registry &r, float dt);

}

#endif
