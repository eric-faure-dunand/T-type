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
    #include "Input.hpp"
    #include "Tags.hpp"
    #include "Transform.hpp"
    #include "Weapon.hpp"
    #include "GameConfig.hpp"
    #include "Spawner.hpp"

namespace r_type::game {

inline void shootingSystem(ecs::Registry &r, float dt)
{
    struct Shot {
        float x;
        float y;
        float speedX;
        int damage;
        bool fromPlayer;
    };
    std::vector<Shot> shots;

    r.each<Weapon, Position, Hitbox>([&](ecs::Entity e, Weapon &weapon, const Position &pos, const Hitbox &box) {
        if (weapon.cooldown > 0.f)
            weapon.cooldown -= dt;
        const bool isPlayer = r.has<Player>(e);
        const Input *input = r.tryGet<Input>(e);
        const bool wantsToShoot = isPlayer ? (input != nullptr && input->shoot) : true;
        if (!wantsToShoot || weapon.cooldown > 0.f || weapon.fireRate <= 0.f)
            return;
        weapon.cooldown = 1.f / weapon.fireRate;
        const float y = pos.y + (box.height - kMissileHeight) / 2.f;
        if (isPlayer)
            shots.push_back({pos.x + box.width, y, weapon.projectileSpeed, weapon.damage, true});
        else
            shots.push_back({pos.x - kMissileWidth, y, -weapon.projectileSpeed, weapon.damage, false});
    });
    for (const Shot &s : shots)
        spawnMissile(r, s.x, s.y, s.speedX, s.damage, s.fromPlayer);
}

}

#endif
