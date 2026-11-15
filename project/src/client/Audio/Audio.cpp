/*
** EPITECH PROJECT, 2026
** T-type
** File description:
** Audio.cpp
*/

#include <algorithm>
#include <filesystem>
#include <map>

#include <raylib.h>

#include "Audio.hpp"

namespace r_type::client {

namespace {

constexpr float DEFAULT_VOLUME = 0.6f;

std::string FindFile(const std::string& base) {
    for (const char *extension : {".ogg", ".mp3", ".wav"}) {
        const std::string path = base + extension;
        std::error_code error;
        if (std::filesystem::is_regular_file(path, error))
            return path;
    }
    return "";
}

}

struct Audio::Impl {
    std::string directory;
    bool ready = false;
    std::map<std::string, ::Music> music;
    std::map<std::string, ::Sound> sounds;
    MusicFader fader;
    std::string playing;
    float volume = DEFAULT_VOLUME;
    bool muted = false;

    void UnloadAll() {
        for (auto& [name, music] : this->music)
            UnloadMusicStream(music);
        for (auto& [name, sound] : sounds)
            UnloadSound(sound);
        music.clear();
        sounds.clear();
        playing.clear();
    }
};

Audio::Audio(std::string soundsDir): _impl(std::make_unique<Impl>()) {
    _impl->directory = std::move(soundsDir);
    InitAudioDevice();
    _impl->ready = IsAudioDeviceReady();
}

Audio::~Audio() {
    if (!_impl->ready)
        return;
    _impl->UnloadAll();
    CloseAudioDevice();
}

bool Audio::Enabled() const {
    return _impl->ready;
}

void Audio::Load(const std::vector<std::string>& countries) {
    if (!_impl->ready)
        return;
    _impl->UnloadAll();

    std::vector<std::string> names{"menu", "game"};
    for (const std::string& country : countries) {
        names.push_back(country);
        names.push_back(country + "_boss");
    }
    for (const std::string& name : names) {
        const std::string path = FindFile(_impl->directory + name);
        if (path.empty())
            continue;
        ::Music music = LoadMusicStream(path.c_str());
        if (!IsMusicValid(music))
            continue;
        music.looping = true;
        _impl->music[name] = music;
    }
    for (SoundEvent event : {SoundEvent::LevelStart, SoundEvent::BossAppears, SoundEvent::BossDefeated,
             SoundEvent::GameOver}) {
        const std::string path = FindFile(_impl->directory + SoundName(event));
        if (path.empty())
            continue;
        ::Sound sound = LoadSound(path.c_str());
        if (IsSoundValid(sound))
            _impl->sounds[SoundName(event)] = sound;
    }
}

void Audio::Update(float dt, const AudioState& state) {
    if (!_impl->ready)
        return;

    const std::string wanted = ChooseTrack(state, [this](const std::string& name) {
        return _impl->music.contains(name);
    });
    const MusicFader::Result step = _impl->fader.Step(dt, wanted);

    if (step.changed) {
        const auto old = _impl->music.find(_impl->playing);
        if (old != _impl->music.end())
            StopMusicStream(old->second);
        _impl->playing = step.track;
        const auto next = _impl->music.find(_impl->playing);
        if (next != _impl->music.end()) {
            SetMusicVolume(next->second, 0.f);
            PlayMusicStream(next->second);
        }
    }

    const auto current = _impl->music.find(_impl->playing);
    if (current == _impl->music.end())
        return;
    SetMusicVolume(current->second, _impl->muted ? 0.f : _impl->volume * step.gain);
    UpdateMusicStream(current->second);
}

void Audio::Play(SoundEvent event) {
    if (!_impl->ready || _impl->muted)
        return;
    const auto it = _impl->sounds.find(SoundName(event));
    if (it == _impl->sounds.end())
        return;
    SetSoundVolume(it->second, _impl->volume);
    PlaySound(it->second);
}

void Audio::SetVolume(float volume) {
    _impl->volume = std::clamp(volume, 0.f, 1.f);
}

float Audio::Volume() const {
    return _impl->volume;
}

void Audio::SetMuted(bool muted) {
    _impl->muted = muted;
}

bool Audio::Muted() const {
    return _impl->muted;
}

void Audio::ToggleMute() {
    _impl->muted = !_impl->muted;
}

std::string Audio::CurrentTrack() const {
    return _impl->playing;
}

std::vector<std::string> Audio::LoadedTracks() const {
    std::vector<std::string> names;
    for (const auto& [name, music] : _impl->music)
        names.push_back(name);
    return names;
}

}