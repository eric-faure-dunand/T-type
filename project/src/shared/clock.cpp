#include "clock.hpp"

namespace r_type {

static std::chrono::steady_clock::duration TickOf(unsigned int rate) {
    if (rate == 0)
        throw Error("Clock: rate must be greater than 0.");
    return std::chrono::duration_cast<std::chrono::steady_clock::duration>(
        std::chrono::nanoseconds(1'000'000'000 / rate));
}

Clock::Clock(unsigned int rate): _tick(TickOf(rate)), _last(std::chrono::steady_clock::now()) {}

bool Clock::Tick() {
    const std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
    if (now - _last < _tick)
        return false;
    _last = now;
    return true;
}

}
