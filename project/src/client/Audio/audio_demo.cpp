/*
** EPITECH PROJECT, 2026
** T-type
** File description:
** audio_demo.cpp
*/

// Plays the sounds of the game without opening the game: menu, then each level
// with its boss, then game over.
//   ./build/demo/audio_demo [sounds_dir] [country...]
// Example: ./build/demo/audio_demo assets/sounds/ turquie corse
// No window is needed: it only uses the audio device.

#include <chrono>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

#include "Audio.hpp"

using namespace r_type::client;

namespace {

constexpr float STEP = 1.f / 60.f;

void Play(Audio& audio, const AudioState& state, float seconds, const std::string& label) {
    std::cout << "== " << label << '\n';
    std::string shown = "<none>";
    for (float t = 0.f; t < seconds; t += STEP) {
        audio.Update(STEP, state);
        if (audio.CurrentTrack() != shown) {
            shown = audio.CurrentTrack();
            std::cout << "   playing: " << (shown.empty() ? "<silence>" : shown) << '\n';
        }
        std::this_thread::sleep_for(std::chrono::duration<float>(STEP));
    }
}

}

int main(int argc, char **argv) {
    Audio audio(argc > 1 ? argv[1] : DEFAULT_SOUNDS_DIR);
    std::vector<std::string> countries;
    for (int i = 2; i < argc; i++)
        countries.push_back(argv[i]);
    if (countries.empty())
        countries = {"argentine", "tunisie", "turquie", "corse"};

    if (!audio.Enabled()) {
        std::cout << "No audio device: the game would be silent.\n";
        return 0;
    }
    audio.Load(countries);
    std::cout << "Tracks found:";
    for (const std::string& name : audio.LoadedTracks())
        std::cout << ' ' << name;
    std::cout << '\n';

    AudioState state;
    Play(audio, state, 3.f, "menu");
    for (const std::string& country : countries) {
        state.inGame = true;
        state.country = country;
        state.bossActive = false;
        audio.Play(SoundEvent::LevelStart);
        Play(audio, state, 4.f, "level " + country);
        state.bossActive = true;
        audio.Play(SoundEvent::BossAppears);
        Play(audio, state, 4.f, "boss of " + country);
        audio.Play(SoundEvent::BossDefeated);
        state.bossActive = false;
        Play(audio, state, 1.f, "boss defeated");
    }
    state.gameOver = true;
    audio.Play(SoundEvent::GameOver);
    Play(audio, state, 3.f, "game over");
    state = AudioState{};
    Play(audio, state, 2.f, "back to the menu");
    return 0;
}