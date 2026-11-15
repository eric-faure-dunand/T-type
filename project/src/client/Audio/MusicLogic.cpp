/*
** EPITECH PROJECT, 2026
** T-type
** File description:
** MusicLogic.cpp
*/

#include "MusicLogic.hpp"

#include <algorithm>

namespace r_type::client {

const char *SoundName(SoundEvent event) {
    switch (event) {
        case SoundEvent::LevelStart:
            return "level_start";
        case SoundEvent::BossAppears:
            return "boss_alert";
        case SoundEvent::BossDefeated:
            return "victory";
        case SoundEvent::GameOver:
            return "game_over";
    }
    return "";
}

std::string ChooseTrack(const AudioState& state, const std::function<bool(const std::string&)>& hasTrack) {
    if (!state.inGame)
        return hasTrack("menu") ? "menu" : "";
    if (state.gameOver)
        return "";
    if (!state.country.empty()) {
        const std::string boss = state.country + "_boss";
        if (state.bossActive && hasTrack(boss))
            return boss;
        if (hasTrack(state.country))
            return state.country;
    }
    return hasTrack("game") ? "game" : "";
}

MusicFader::MusicFader(float fadeSeconds): _fade(fadeSeconds) {}

MusicFader::Result MusicFader::Step(float dt, const std::string& wanted) {
    if (!(dt > 0.f))
        dt = 0.f;
    const float delta = _fade > 0.f ? dt / _fade : 1.f;
    bool changed = false;

    if (wanted == _current) {
        _gain = std::min(1.f, _gain + delta);
    } else if (_current.empty()) {
        _current = wanted;
        _gain = 0.f;
        changed = true;
    } else {
        _gain -= delta;
        if (_gain <= 0.f) {
            _gain = 0.f;
            _current = wanted;
            changed = true;
        }
    }
    return Result{_current, _gain, changed};
}

}