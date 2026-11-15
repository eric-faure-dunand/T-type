/*
** EPITECH PROJECT, 2026
** T-type
** File description:
** Audio.hpp
*/

#ifndef AUDIO_HPP
    #define AUDIO_HPP

    #include <memory>
    #include <string>
    #include <vector>

    #include "MusicLogic.hpp"

namespace r_type::client {

constexpr const char *DEFAULT_SOUNDS_DIR = "assets/sounds/";

class Audio {
public:
    explicit Audio(std::string soundsDir = DEFAULT_SOUNDS_DIR);
    ~Audio();

    Audio(const Audio&) = delete;
    Audio& operator=(const Audio&) = delete;

    bool Enabled() const;
    void Load(const std::vector<std::string>& countries);
    void Update(float dt, const AudioState& state);
    void Play(SoundEvent event);
    void SetVolume(float volume);
    float Volume() const;
    void SetMuted(bool muted);
    bool Muted() const;
    void ToggleMute();
    std::string CurrentTrack() const;
    std::vector<std::string> LoadedTracks() const;

private:
    struct Impl;
    std::unique_ptr<Impl> _impl;
};

}

#endif
