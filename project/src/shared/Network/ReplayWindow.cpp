/*
** EPITECH PROJECT, 2026
** T-type
** File description:
** ReplayWindow.cpp
*/

#include "Network/ReplayWindow.hpp"

namespace r_type {

bool ReplayWindow::accepts(std::uint64_t sequence) const
{
    if (sequence == 0)
        return false;
    if (sequence > _highest)
        return true;

    const std::uint64_t distance = _highest - sequence;

    if (distance >= WINDOW_SIZE)
        return false;
    return (_received & (std::uint64_t{1} << distance)) == 0;
}

bool ReplayWindow::record(std::uint64_t sequence)
{
    if (!accepts(sequence))
        return false;

    if (sequence > _highest) {
        const std::uint64_t shift = sequence - _highest;

        if (shift >= WINDOW_SIZE)
            _received = 0;
        else
            _received <<= shift;

        _received |= 1;
        _highest = sequence;
        return true;
    }

    const std::uint64_t distance = _highest - sequence;

    _received |= std::uint64_t{1} << distance;
    return true;
}

void ReplayWindow::reset()
{
    _highest = 0;
    _received = 0;
}

}
