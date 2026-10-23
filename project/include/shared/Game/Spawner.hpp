/*
** EPITECH PROJECT, 2026
** T-type
** File description:
** Spawner.hpp
*/

#ifndef GAME_SPAWNER_HPP
    #define GAME_SPAWNER_HPP

    #include "Registry.hpp"

namespace r_type::game {

void registerGameplayComponents(ecs::Registry &r);
ecs::Entity spawnPlayer(ecs::Registry &r, int id, float x, float y);
ecs::Entity spawnEnemy(ecs::Registry &r, float x, float y, float firstShotDelay);
ecs::Entity spawnMissile(ecs::Registry &r, float x, float y, float speedX, int damage, bool fromPlayer);

}

#endif
