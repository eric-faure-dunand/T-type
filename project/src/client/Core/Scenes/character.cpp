/*
** EPITECH PROJECT, 2026
** T-type
** File description:
** character.cpp
*/

#include "ClientCore.hpp"

namespace r_type::client {

void ClientCore::CharacterFrame(float dt) {
    KeepAlive(dt);

    DrawCentered("CHOOSE YOUR CHARACTER", 70, 44, palette::ACCENT);

    const float width = 240.f;
    const float height = 320.f;
    const float gap = 30.f;
    const float count = static_cast<float>(CHARACTERS.size());
    const float startX = (static_cast<float>(WINDOW_WIDTH) - (count * width + (count - 1.f) * gap)) / 2.f;
    const float y = 170.f;
    const float faceSize = 160.f;
    std::optional<std::size_t> clicked;

    for (std::size_t i = 0; i < CHARACTERS.size(); i++) {
        const float x = startX + static_cast<float>(i) * (width + gap);
        const float mx = _display.MouseX();
        const float my = _display.MouseY();
        if (mx >= x && mx <= x + width && my >= y && my <= y + height) {
            _character = static_cast<std::uint8_t>(i);
            if (_display.IsMouseClicked())
                clicked = i;
        }
        const bool selected = i == _character;
        _display.FillRect(x, y, width, height, selected ? palette::PANEL_HOVER : palette::PANEL);
        _display.OutlineRect(x, y, width, height, selected ? 4.f : 2.f, selected ? palette::PLAYERS[i] : palette::PANEL);
        DrawFace(i, x + (width - faceSize) / 2.f, y + 30.f, faceSize);
        const std::string name = CHARACTERS[i].name;
        const std::string country = CHARACTERS[i].country;
        _display.Print(name, static_cast<int>(x + (width - static_cast<float>(_display.TextWidth(name, 30))) / 2.f),
            static_cast<int>(y) + 215, 30, palette::FOREGROUND);
        _display.Print(country, static_cast<int>(x + (width - static_cast<float>(_display.TextWidth(country, 20))) / 2.f),
            static_cast<int>(y) + 260, 20, palette::DIM);
    }

    DrawHint("LEFT / RIGHT to choose  -  ENTER to play  -  ESC to go back", 560, 18);

    const auto total = static_cast<std::uint8_t>(CHARACTERS.size());
    if (_display.IsKeyPressed(Key::Right))
        _character = static_cast<std::uint8_t>((_character + 1) % total);
    if (_display.IsKeyPressed(Key::Left))
        _character = static_cast<std::uint8_t>((_character + total - 1) % total);
    if (_display.IsKeyPressed(Key::Enter))
        clicked = _character;

    if (clicked) {
        _character = static_cast<std::uint8_t>(*clicked);
        if (_joinTarget) {
            _requestedLevel.reset();
            Send(Command::JoinRoom, {{*_joinTarget}});
            SetScene(Scene::Joining);
        } else {
            SetScene(Scene::Level);
        }
    } else if (_display.IsKeyPressed(Key::Escape)) {
        SetScene(Scene::Lobby);
    }
}

void ClientCore::KeepAlive(float dt) {
    _refreshTimer += dt;
    if (_refreshTimer >= ROOM_REFRESH_DELAY) {
        _refreshTimer = 0.f;
        Send(Command::ListRooms, {{}});
    }
}

}
