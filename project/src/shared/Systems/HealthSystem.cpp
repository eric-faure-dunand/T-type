/*
** EPITECH PROJECT, 2026
** T-type
** File description:
** HealthSystem.cpp
*/

#include <vector>

#include "HealthSystem.hpp"
#include "Combat.hpp"

namespace r_type::game {

void healthSystem(ecs::Registry &r, float)
{
    std::vector<ecs::Entity> dead;
    r.each<Health>([&dead](ecs::Entity e, const Health &hp) {
        if (hp.current <= 0)
            dead.push_back(e);
    });
    for (const ecs::Entity e : dead)
        r.kill(e);
}

}
