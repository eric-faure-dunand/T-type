#ifndef COMBAT_HPP
    #define COMBAT_HPP

namespace r_type {

struct Health {
    int current = 1;
    int max = 1;
};

struct Damage {
    int amount = 1;
};

struct Hitbox {
    float width = 0.f;
    float height = 0.f;
};

}

#endif
