/*
** EPITECH PROJECT, 2026
** T-type
** File description:
** level.cpp
*/

#include "ClientCore.hpp"

namespace r_type::client {

void ClientCore::LoadLevels() {
    for (const std::string& path : game::levelFiles()) {
        LevelInfo info{path, "?", "", ""};
        try {
            const game::LevelData data = game::loadLevel(path);
            info.name = data.name;
            info.country = data.country;
            if (!info.country.empty())
                info.country[0] = static_cast<char>(std::toupper(static_cast<unsigned char>(info.country[0])));
            info.background = std::string(BACKGROUNDS_DIR) + data.background;
            const int boss = static_cast<int>(data.boss.type) - FIRST_BOSS;
            if (data.boss.enabled && boss >= 0 && boss < static_cast<int>(BOSS_NAMES.size()))
                info.boss = BOSS_NAMES[static_cast<std::size_t>(boss)];
            LoadSprite(info.background.c_str());
        } catch (const IError& error) {
            std::cerr << error.what() << '\n';
        }
        _levels.push_back(info);
    }
}

void ClientCore::LevelFrame(float dt) {
    KeepAlive(dt);

    DrawCentered("CHOOSE THE LEVEL", 70, 44, palette::ACCENT);
    if (_levels.empty()) {
        DrawCentered("No level found in assets/levels", 300, 26, palette::ALERT);
        DrawHint("ESC to go back", 560, 18);
        if (_display.IsKeyPressed(Key::Escape))
            SetScene(Scene::Character);
        return;
    }
    if (_levelChoice >= _levels.size())
        _levelChoice = 0;

    const std::size_t first = _levelChoice / LEVEL_CARDS * LEVEL_CARDS;
    const std::size_t shown = std::min(LEVEL_CARDS, _levels.size() - first);
    const float width = 270.f;
    const float height = 300.f;
    const float gap = 20.f;
    const auto count = static_cast<float>(shown);
    const float startX = (static_cast<float>(WINDOW_WIDTH) - (count * width + (count - 1.f) * gap)) / 2.f;
    const float y = 170.f;
    const float imageWidth = width - 30.f;
    const float imageHeight = imageWidth * 9.f / 16.f;
    std::optional<std::size_t> clicked;

    for (std::size_t i = first; i < first + shown; i++) {
        const LevelInfo& level = _levels[i];
        const float x = startX + static_cast<float>(i - first) * (width + gap);
        const float mx = _display.MouseX();
        const float my = _display.MouseY();
        if (mx >= x && mx <= x + width && my >= y && my <= y + height) {
            _levelChoice = static_cast<std::uint8_t>(i);
            if (_display.IsMouseClicked())
                clicked = i;
        }
        const bool selected = i == _levelChoice;
        _display.FillRect(x, y, width, height, selected ? palette::PANEL_HOVER : palette::PANEL);
        _display.OutlineRect(x, y, width, height, selected ? 4.f : 2.f, selected ? palette::ACCENT : palette::PANEL);
        _display.DrawSprite(level.background, x + 15.f, y + 15.f, imageWidth, imageHeight);
        const std::string boss = level.boss.empty() ? "No boss" : "Boss : " + level.boss;
        const float textY = y + 30.f + imageHeight;
        _display.Print(level.country,
            static_cast<int>(x + (width - static_cast<float>(_display.TextWidth(level.country, 28))) / 2.f),
            static_cast<int>(textY), 28, palette::FOREGROUND);
        _display.Print(level.name,
            static_cast<int>(x + (width - static_cast<float>(_display.TextWidth(level.name, 20))) / 2.f),
            static_cast<int>(textY) + 40, 20, palette::DIM);
        _display.Print(boss, static_cast<int>(x + (width - static_cast<float>(_display.TextWidth(boss, 18))) / 2.f),
            static_cast<int>(textY) + 75, 18, palette::ALERT);
    }

    const std::size_t pages = (_levels.size() + LEVEL_CARDS - 1) / LEVEL_CARDS;
    if (pages > 1)
        DrawCentered("Page " + std::to_string(first / LEVEL_CARDS + 1) + " / " + std::to_string(pages), 500, 20,
            palette::DIM);
    DrawHint("LEFT / RIGHT to choose  -  ENTER to create the room  -  ESC to go back", 560, 18);

    const std::size_t total = _levels.size();
    if (_display.IsKeyPressed(Key::Right))
        _levelChoice = static_cast<std::uint8_t>((_levelChoice + 1) % total);
    if (_display.IsKeyPressed(Key::Left))
        _levelChoice = static_cast<std::uint8_t>((_levelChoice + total - 1) % total);
    if (_display.IsKeyPressed(Key::Enter))
        clicked = _levelChoice;

    if (clicked) {
        _levelChoice = static_cast<std::uint8_t>(*clicked);
        _requestedLevel = _levelChoice;
        Send(Command::CreateRoom, {{}});
        SetScene(Scene::Joining);
    } else if (_display.IsKeyPressed(Key::Escape)) {
        SetScene(Scene::Character);
    }
}

}
