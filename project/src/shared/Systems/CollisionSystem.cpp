/*
** EPITECH PROJECT, 2026
** T-type
** File description:
** CollisionSystem.cpp
*/

#include "CollisionSystem.hpp"

namespace r_type::game {

bool overlaps(const Position &a, const Hitbox &ha, const Position &b, const Hitbox &hb) noexcept
{
    return a.x < b.x + hb.width && b.x < a.x + ha.width
        && a.y < b.y + hb.height && b.y < a.y + ha.height;
}

void collisionSystem(ecs::Registry &r, float)
{
    std::vector<ecs::Entity> usedUp;

    r.each<Projectile, Position, Hitbox, Damage>([&](ecs::Entity missile, const Projectile &proj,
                                                    const Position &mPos, const Hitbox &mBox, const Damage &dmg) {
        bool hit = false;
        r.each<Health, Position, Hitbox>([&](ecs::Entity target, Health &hp, const Position &tPos, const Hitbox &tBox) {
            if (hit || hp.current <= 0 || proj.fromPlayer != r.has<Enemy>(target))
                return;
            if (!overlaps(mPos, mBox, tPos, tBox))
                return;
            hp.current -= dmg.amount;
            hit = true;
        });
        if (hit)
            usedUp.push_back(missile);
    });

    r.each<Enemy, Position, Hitbox, Damage>([&](ecs::Entity enemy, Enemy &, const Position &ePos,
                                               const Hitbox &eBox, const Damage &dmg) {
        bool hit = false;
        r.each<Player, Health, Position, Hitbox>([&](Player &, Health &hp, const Position &pPos, const Hitbox &pBox) {
            if (hit || hp.current <= 0 || !overlaps(ePos, eBox, pPos, pBox))
                return;
            hp.current -= dmg.amount;
            hit = true;
        });
        if (hit)
            usedUp.push_back(enemy);
    });

    for (const ecs::Entity e : usedUp)
        if (r.alive(e))
            r.kill(e);
}

}
