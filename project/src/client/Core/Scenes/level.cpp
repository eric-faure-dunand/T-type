/*
** EPITECH PROJECT, 2026
** T-type
** File description:
** level.cpp
*/

#include "ClientCore.hpp"

namespace r_type::client {

void ClientCore::LevelFrame(float dt) {
    KeepAlive(dt);

    DrawCentered("CHOOSE THE LEVEL", 70, 44, palette::ACCENT);

    const float width = 270.f;
    const float height = 300.f;
    const float gap = 20.f;
    const float count = static_cast<float>(LEVELS.size());
    const float startX = (static_cast<float>(WINDOW_WIDTH) - (count * width + (count - 1.f) * gap)) / 2.f;
    const float y = 170.f;
    const float imageWidth = width - 30.f;
    const float imageHeight = imageWidth * 9.f / 16.f;
    std::optional<std::size_t> clicked;

    for (std::size_t i = 0; i < LEVELS.size(); i++) {
        const float x = startX + static_cast<float>(i) * (width + gap);
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
        if (_display.HasSprite(LEVELS[i].country))
            _display.DrawSprite(LEVELS[i].country, x + 15.f, y + 15.f, imageWidth, imageHeight);
        const std::string country = LEVELS[i].country;
        const std::string name = LEVELS[i].name;
        const std::string boss = std::string("Boss : ") + BOSS_NAMES[i];
        const float textY = y + 30.f + imageHeight;
        _display.Print(country, static_cast<int>(x + (width - static_cast<float>(_display.TextWidth(country, 28))) / 2.f),
            static_cast<int>(textY), 28, palette::FOREGROUND);
        _display.Print(name, static_cast<int>(x + (width - static_cast<float>(_display.TextWidth(name, 20))) / 2.f),
            static_cast<int>(textY) + 40, 20, palette::DIM);
        _display.Print(boss, static_cast<int>(x + (width - static_cast<float>(_display.TextWidth(boss, 18))) / 2.f),
            static_cast<int>(textY) + 75, 18, palette::ALERT);
    }

    DrawCentered("LEFT / RIGHT to choose  -  ENTER to create the room  -  ESC to go back", 560, 18, palette::DIM);

    const auto total = static_cast<std::uint8_t>(LEVELS.size());
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
