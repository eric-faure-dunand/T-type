/*
** EPITECH PROJECT, 2026
** T-type
** File description:
** CollisionSystem.hpp
*/

#ifndef SYSTEMS_COLLISION_HPP
    #define SYSTEMS_COLLISION_HPP

    #include "Registry.hpp"
    #include "Combat.hpp"
    #include "Transform.hpp"

namespace r_type::game {

bool overlaps(const Position &a, const Hitbox &ha, const Position &b, const Hitbox &hb) noexcept;
void collisionSystem(ecs::Registry &r, float dt);

}

#endif
