/*
** EPITECH PROJECT, 2026
** T-type
** File description:
** ReplayWindow.hpp
*/

#ifndef REPLAY_WINDOW_HPP
    #define REPLAY_WINDOW_HPP

    #include <cstdint>

namespace r_type {

class ReplayWindow {
public:
    static constexpr std::uint64_t WINDOW_SIZE = 64;

    bool accepts(std::uint64_t sequence) const;
    bool record(std::uint64_t sequence);
    void reset();

private:
    std::uint64_t _highest = 0;
    std::uint64_t _received = 0;
};

}

#endif
