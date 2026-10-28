/*
** EPITECH PROJECT, 2026
** T-type
** File description:
** update_projectiles.cpp
*/

#include "room.hpp"

namespace r_type {

void Room::UpdateProjectiles() {
    registre.each<Position, Projectile>([](Position& position, const Projectile& projectile) {
        position.x += projectile.speed;
    });
}

}