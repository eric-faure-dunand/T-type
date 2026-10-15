#ifndef TAGS_HPP
    #define TAGS_HPP

namespace r_type {

struct Player {
    int id = 0;
};

struct Enemy {};

struct Projectile {
    bool fromPlayer = true;
};

}

#endif
