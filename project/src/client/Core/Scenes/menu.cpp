/*
** EPITECH PROJECT, 2026
** T-type
** File description:
** menu.cpp
*/

#include "ClientCore.hpp"

namespace r_type::client {

void ClientCore::MenuFrame() {
    for (const char c : _display.TypedText())
        if ((std::isdigit(static_cast<unsigned char>(c)) || c == '.') && _ip.size() < MAX_IP_LENGTH)
            _ip += c;
    if (_display.IsKeyPressed(Key::Backspace) && !_ip.empty())
        _ip.pop_back();

    DrawMenuBackground();
    DrawTitle();

    std::string countries;
    for (const Character& character : CHARACTERS) {
        if (!countries.empty())
            countries += "  ·  ";
        for (const char *c = character.country; *c; c++)
            countries += static_cast<char>(std::toupper(static_cast<unsigned char>(*c)));
    }
    DrawCentered(countries, 297, 18, palette::DIM);
    DrawCentered("Server IP address", 353, 22, palette::DIM);

    const float boxX = static_cast<float>(WINDOW_WIDTH) / 2.f - 180.f;
    DrawTourBox(boxX, 389.f, 360.f, 56.f);
    _display.FillRect(boxX + 2.f, 391.f, 356.f, 52.f, palette::INPUT);
    const bool caret = _settings.reduceMotion || static_cast<int>(_display.Time() * 2) % 2 == 0;
    DrawCentered(_ip + (caret ? "_" : " "), 400, 30, palette::FOREGROUND);

    const bool clicked = TourButton("Connect", static_cast<float>(WINDOW_WIDTH) / 2.f - 100.f, 468.f, 200.f, 50.f);
    DrawHint("ENTER to connect  -  O for options  -  ESC to quit", 540, 18);

    if ((clicked || _display.IsKeyPressed(Key::Enter)) && !_ip.empty()) {
        _messageTime = 0.f;
        ConnectServer();
    } else if (_display.IsKeyPressed(Key::O)) {
        OpenOptions();
    } else if (_display.IsKeyPressed(Key::Escape)) {
        _quit = true;
    }
}

void ClientCore::DrawMenuBackground() {
    const double time = _settings.reduceMotion ? 0.0 : _display.Time();
    const auto slide = static_cast<std::size_t>(time / MENU_SLIDE);
    const auto phase = static_cast<float>(time - static_cast<double>(slide) * MENU_SLIDE);
    const auto width = static_cast<float>(WINDOW_WIDTH);
    const auto height = static_cast<float>(WINDOW_HEIGHT);
    _display.DrawSprite(CHARACTERS[slide % CHARACTERS.size()].country, 0.f, 0.f, width, height);
    if (phase > MENU_SLIDE - MENU_FADE) {
        const float fade = (phase - (MENU_SLIDE - MENU_FADE)) / MENU_FADE;
        _display.DrawSprite(CHARACTERS[(slide + 1) % CHARACTERS.size()].country, 0.f, 0.f, width, height,
            static_cast<std::uint8_t>(fade * 255.f));
    }
    _display.FillRect(0.f, 0.f, width, height, Shade(palette::MENU_SHADE));
    DrawStars();
}

void ClientCore::DrawTitle() {
    const std::string title = "R-TYPE";
    _display.Print(title, (WINDOW_WIDTH - _display.TextWidth(title, 104, TITLE_FONT)) / 2, 96, 104,
        palette::FOREGROUND, TITLE_FONT);

    const std::string subtitle = "WORLD TOUR";
    const int size = 50;
    const int spacing = 6;
    int total = -spacing;
    for (const char c : subtitle)
        total += _display.TextWidth(std::string(1, c), size, TITLE_FONT) + spacing;
    int x = (WINDOW_WIDTH - total) / 2;
    std::size_t letter = 0;
    for (const char c : subtitle) {
        const std::string glyph(1, c);
        if (c != ' ')
            _display.Print(glyph, x, 223, size, palette::PLAYERS[letter++ % CHARACTERS.size()], TITLE_FONT);
        x += _display.TextWidth(glyph, size, TITLE_FONT) + spacing;
    }
}

void ClientCore::DrawTourBox(float x, float y, float width, float height) {
    const std::size_t parts = CHARACTERS.size() - 1;
    for (std::size_t i = 0; i < parts; i++) {
        const auto left = static_cast<float>(static_cast<int>(x + width * static_cast<float>(i) / static_cast<float>(parts)));
        const auto right = static_cast<float>(static_cast<int>(x + width * static_cast<float>(i + 1) / static_cast<float>(parts)));
        _display.FillGradient(left, y, right - left, height, palette::PLAYERS[i], palette::PLAYERS[i + 1]);
    }
}

bool ClientCore::TourButton(const std::string& label, float x, float y, float width, float height) {
    const float mx = _display.MouseX();
    const float my = _display.MouseY();
    const bool hover = mx >= x && mx <= x + width && my >= y && my <= y + height;
    DrawTourBox(x, y, width, height);
    if (hover)
        _display.FillRect(x, y, width, height, palette::BUTTON_HOVER);
    const int textX = static_cast<int>(x + (width - static_cast<float>(_display.TextWidth(label, 22))) / 2.f);
    _display.Print(label, textX, static_cast<int>(y + (height - 22.f) / 2.f), 22, palette::BACKGROUND);
    return hover && _display.IsMouseClicked();
}

}
