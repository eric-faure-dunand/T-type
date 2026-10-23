/*
** EPITECH PROJECT, 2026
** T-type
** File description:
** Spawner.cpp
*/

#include "Spawner.hpp"

namespace r_type::game {

void registerGameplayComponents(ecs::Registry &r)
{
    r.registerComponent<Position>();
    r.registerComponent<Velocity>();
    r.registerComponent<Health>();
    r.registerComponent<Damage>();
    r.registerComponent<Hitbox>();
    r.registerComponent<Weapon>();
    r.registerComponent<Input>();
    r.registerComponent<Player>();
    r.registerComponent<Enemy>();
    r.registerComponent<Projectile>();
    r.registerComponent<Kind>();
}

ecs::Entity spawnPlayer(ecs::Registry &r, int id, float x, float y)
{
    const ecs::Entity e = r.spawn();
    r.add<Position>(e, Position{x, y});
    r.add<Velocity>(e, Velocity{});
    r.add<Hitbox>(e, Hitbox{kPlayerWidth, kPlayerHeight});
    r.add<Health>(e, Health{kPlayerHealth, kPlayerHealth});
    r.add<Weapon>(e, Weapon{kPlayerFireRate, 0.f, kPlayerMissileSpeed, 1});
    r.add<Input>(e, Input{});
    r.add<Player>(e, Player{id});
    r.add<Kind>(e, Kind{EntityKind::Player});
    return e;
}

ecs::Entity spawnEnemy(ecs::Registry &r, float x, float y, float firstShotDelay)
{
    const ecs::Entity e = r.spawn();
    r.add<Position>(e, Position{x, y});
    r.add<Velocity>(e, Velocity{-kEnemySpeed, 0.f});
    r.add<Hitbox>(e, Hitbox{kEnemySize, kEnemySize});
    r.add<Health>(e, Health{kEnemyHealth, kEnemyHealth});
    r.add<Damage>(e, Damage{1});
    r.add<Weapon>(e, Weapon{kEnemyFireRate, firstShotDelay, kEnemyMissileSpeed, 1});
    r.add<Enemy>(e);
    r.add<Kind>(e, Kind{EntityKind::Enemy});
    return e;
}

ecs::Entity spawnMissile(ecs::Registry &r, float x, float y, float speedX, int damage, bool fromPlayer)
{
    const ecs::Entity e = r.spawn();
    r.add<Position>(e, Position{x, y});
    r.add<Velocity>(e, Velocity{speedX, 0.f});
    r.add<Hitbox>(e, Hitbox{kMissileWidth, kMissileHeight});
    r.add<Damage>(e, Damage{damage});
    r.add<Projectile>(e, Projectile{fromPlayer});
    r.add<Kind>(e, Kind{fromPlayer ? EntityKind::PlayerMissile : EntityKind::EnemyMissile});
    return e;
}

}
