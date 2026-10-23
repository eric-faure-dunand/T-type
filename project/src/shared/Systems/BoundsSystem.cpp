/*
** EPITECH PROJECT, 2026
** T-type
** File description:
** BoundsSystem.cpp
*/

#include "BoundsSystem.hpp"

namespace r_type::game {

void boundsSystem(ecs::Registry &r, float)
{
    r.each<Player, Position, Hitbox>([](Player &, Position &pos, const Hitbox &box) {
        pos.x = std::clamp(pos.x, 0.f, kWorldWidth - box.width);
        pos.y = std::clamp(pos.y, 0.f, kWorldHeight - box.height);
    });

    std::vector<ecs::Entity> gone;
    r.each<Position, Hitbox>([&gone](ecs::Entity e, const Position &pos, const Hitbox &box) {
        if (pos.x + box.width < -kDespawnMargin || pos.x > kWorldWidth + kDespawnMargin
            || pos.y + box.height < -kDespawnMargin || pos.y > kWorldHeight + kDespawnMargin)
            gone.push_back(e);
    });
    for (const ecs::Entity e : gone)
        r.kill(e);
}

}
