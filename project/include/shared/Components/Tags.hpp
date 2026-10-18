/*
** EPITECH PROJECT, 2026
** T-type
** File description:
** Tags.hpp
*/

#ifndef COMPONENTS_TAGS_HPP
    #define COMPONENTS_TAGS_HPP

    #include <cstdint>

namespace r_type::game {

struct Player {
    int id = 1;
};

struct Enemy {};

struct Projectile {
    bool fromPlayer = true;
};

enum class EntityKind : std::uint8_t {
    Player = 0,
    Enemy = 1,
    PlayerMissile = 2,
    EnemyMissile = 3
};

struct Kind {
    EntityKind value = EntityKind::Player;
};

}

#endif
