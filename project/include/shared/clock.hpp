#ifndef CLOCK_HPP
    #define CLOCK_HPP

    #include <chrono>

    #include "Error.hpp"

namespace r_type {

constexpr unsigned int TICK_RATE = 120;

class Clock {
    std::chrono::steady_clock::duration _tick;
    std::chrono::steady_clock::time_point _last;

public:
    explicit Clock(unsigned int rate = TICK_RATE);
    ~Clock() = default;

    bool Tick();
};

}

#endif
