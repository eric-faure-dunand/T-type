/*
** EPITECH PROJECT, 2026
** T-type
** File description:
** HealthSystem.hpp
*/

#ifndef SYSTEMS_HEALTH_HPP
    #define SYSTEMS_HEALTH_HPP

    #include <vector>

    #include "shared/ECS/Registry.hpp"
    #include "shared/Components/Combat.hpp"

namespace r_type::game {

inline void healthSystem(ecs::Registry &r, float)
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

#endif
