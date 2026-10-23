/*
** EPITECH PROJECT, 2026
** T-type
** File description:
** Gameplay.hpp
*/

#ifndef GAME_GAMEPLAY_HPP
    #define GAME_GAMEPLAY_HPP

    #include <cstdint>

    #include "SystemManager.hpp"

namespace r_type::game {

void addGameplaySystems(ecs::SystemManager &systems, std::uint32_t seed);

}

#endif
