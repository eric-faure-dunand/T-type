/*
** EPITECH PROJECT, 2026
** T-type
** File description:
** MusicLogic.hpp
*/

#ifndef MUSIC_LOGIC_HPP
    #define MUSIC_LOGIC_HPP

    #include <functional>
    #include <string>

namespace r_type::client {

struct AudioState {
    bool inGame = false;
    bool gameOver = false;
    bool bossActive = false;
    std::string country;
};

enum class SoundEvent {
    LevelStart,
    BossAppears,
    BossDefeated,
    GameOver
};

const char *SoundName(SoundEvent event);

std::string ChooseTrack(const AudioState& state, const std::function<bool(const std::string&)>& hasTrack);

class MusicFader {
public:
    struct Result {
        std::string track;
        float gain = 0.f;
        bool changed = false;
    };

    explicit MusicFader(float fadeSeconds = 0.6f);

    Result Step(float dt, const std::string& wanted);

private:
    float _fade;
    std::string _current;
    float _gain = 0.f;
};

}

#endif
