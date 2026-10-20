/*
** EPITECH PROJECT, 2026
** T-type
** File description:
** lobby.cpp
*/

#include <algorithm>
#include <optional>

#include "ClientCore.hpp"

namespace r_type::client {

void ClientCore::LobbyFrame(float dt) {
    _refreshTimer += dt;
    if (_refreshTimer >= ROOM_REFRESH_DELAY) {
        _refreshTimer = 0.f;
        Send(Command::ListRooms, {{}});
    }

    DrawCentered("ROOMS", 60, 50, palette::ACCENT);
    DrawCentered("Connected to " + _ip, 120, 18, palette::DIM);

    const float listX = static_cast<float>(WINDOW_WIDTH) / 2.f - 300.f;
    const float rowHeight = 48.f;
    const std::size_t visible = 7;
    std::optional<std::size_t> clicked;

    if (_rooms.empty())
        DrawCentered("No room available yet. Create one!", 300, 24, palette::FOREGROUND);
    for (std::size_t i = 0; i < _rooms.size() && i < visible; i++) {
        const float y = 160.f + static_cast<float>(i) * (rowHeight + 8.f);
        const float mx = _display.MouseX();
        const float my = _display.MouseY();
        if (mx >= listX && mx <= listX + 600.f && my >= y && my <= y + rowHeight) {
            _selected = i;
            if (_display.IsMouseClicked())
                clicked = i;
        }
        const bool selected = i == _selected;
        _display.FillRect(listX, y, 600.f, rowHeight, selected ? palette::PANEL_HOVER : palette::PANEL);
        _display.OutlineRect(listX, y, 600.f, rowHeight, 2.f, selected ? palette::ACCENT : palette::PANEL);
        const RoomInfo& room = _rooms[i];
        const std::string slots = std::to_string(room.freeSlots) + (room.freeSlots > 1 ? " free slots" : " free slot");
        _display.Print("Room " + std::to_string(room.id), static_cast<int>(listX) + 20, static_cast<int>(y) + 13, 22,
            palette::FOREGROUND);
        _display.Print(slots, static_cast<int>(listX) + 580 - _display.TextWidth(slots, 20), static_cast<int>(y) + 14,
            20, palette::DIM);
    }

    const float buttonsY = 570.f;
    const bool create = Button("Create room (C)", listX, buttonsY, 190.f, 50.f);
    const bool refresh = Button("Refresh (R)", listX + 205.f, buttonsY, 190.f, 50.f);
    const bool back = Button("Back (ESC)", listX + 410.f, buttonsY, 190.f, 50.f);
    DrawCentered("UP / DOWN to choose  -  ENTER to join", 640, 18, palette::DIM);

    const std::size_t count = std::min(_rooms.size(), visible);
    if (_display.IsKeyPressed(Key::Down) && count > 0)
        _selected = (_selected + 1) % count;
    if (_display.IsKeyPressed(Key::Up) && count > 0)
        _selected = (_selected + count - 1) % count;
    if (_display.IsKeyPressed(Key::Enter) && _selected < count)
        clicked = _selected;

    if (clicked) {
        Send(Command::JoinRoom, {{_rooms[*clicked].id}});
        SetScene(Scene::Joining);
    } else if (create || _display.IsKeyPressed(Key::C)) {
        Send(Command::CreateRoom, {{}});
        SetScene(Scene::Joining);
    } else if (refresh || _display.IsKeyPressed(Key::R)) {
        Send(Command::ListRooms, {{}});
    } else if (back || _display.IsKeyPressed(Key::Escape)) {
        SayGoodbye();
        GoOffline("");
    }
}

}
