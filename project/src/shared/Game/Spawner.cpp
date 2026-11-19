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
    r.registerComponent<Pattern>();
    r.registerComponent<Boss>();
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

ecs::Entity spawnEnemy(ecs::Registry &r, EnemyType type, PatternType pattern, float x, float y, float firstShotDelay)
{
    const EnemyStats &stats = kEnemyStats[static_cast<std::size_t>(type)];
    const ecs::Entity e = r.spawn();
    r.add<Position>(e, Position{x, y});
    r.add<Velocity>(e, Velocity{-stats.speed, 0.f});
    r.add<Hitbox>(e, Hitbox{stats.size, stats.size});
    r.add<Health>(e, Health{stats.health, stats.health});
    r.add<Damage>(e, Damage{1});
    r.add<Weapon>(e, Weapon{stats.fireRate, firstShotDelay, kEnemyMissileSpeed, 1});
    r.add<Enemy>(e, Enemy{type});
    r.add<Pattern>(e, Pattern{pattern, stats.speed, y, 0.f, 1.f});
    r.add<Kind>(e, Kind{EntityKind::Enemy});
    return e;
}

ecs::Entity spawnBoss(ecs::Registry &r, EnemyType type, int health)
{
    const ecs::Entity e = spawnEnemy(r, type, PatternType::Boss, kWorldWidth, (kWorldHeight - kBossSize) / 2.f, 1.f);
    if (health > 0)
        r.get<Health>(e) = Health{health, health};
    r.add<Boss>(e);
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
