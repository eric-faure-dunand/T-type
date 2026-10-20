/*
** EPITECH PROJECT, 2026
** T-type
** File description:
** menu.cpp
*/

#include <cctype>

#include "ClientCore.hpp"

namespace r_type::client {

void ClientCore::MenuFrame() {
    for (const char c : _display.TypedText())
        if ((std::isdigit(static_cast<unsigned char>(c)) || c == '.') && _ip.size() < MAX_IP_LENGTH)
            _ip += c;
    if (_display.IsKeyPressed(Key::Backspace) && !_ip.empty())
        _ip.pop_back();

    DrawCentered("R-TYPE", 140, 80, palette::ACCENT);
    DrawCentered("Server IP address", 300, 22, palette::DIM);

    const float boxX = static_cast<float>(WINDOW_WIDTH) / 2.f - 180.f;
    _display.FillRect(boxX, 335.f, 360.f, 56.f, palette::PANEL);
    _display.OutlineRect(boxX, 335.f, 360.f, 56.f, 2.f, palette::ACCENT);
    const bool caret = static_cast<int>(_display.Time() * 2) % 2 == 0;
    DrawCentered(_ip + (caret ? "_" : " "), 350, 30, palette::FOREGROUND);

    const bool clicked = Button("Connect", static_cast<float>(WINDOW_WIDTH) / 2.f - 100.f, 420.f, 200.f, 50.f);
    DrawCentered("ENTER to connect  -  ESC to quit", 500, 18, palette::DIM);

    if ((clicked || _display.IsKeyPressed(Key::Enter)) && !_ip.empty()) {
        _messageTime = 0.f;
        ConnectServer();
    } else if (_display.IsKeyPressed(Key::Escape)) {
        _quit = true;
    }
}

}
