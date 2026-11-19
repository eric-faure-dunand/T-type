/*
** EPITECH PROJECT, 2026
** T-type
** File description:
** options.cpp
*/

#include "ClientCore.hpp"

namespace r_type::client {

void ClientCore::LoadSettings() {
    std::ifstream file(SETTINGS_FILE);
    if (!file)
        return;
    try {
        const nlohmann::json json = nlohmann::json::parse(file);
        for (const OptionInfo& option : OPTIONS)
            _settings.*option.value = json.value(option.key, false);
    } catch (const nlohmann::json::exception&) {
        _settings = Settings{};
    }
}

void ClientCore::SaveSettings() {
    nlohmann::json json;
    for (const OptionInfo& option : OPTIONS)
        json[option.key] = _settings.*option.value;
    std::ofstream file(SETTINGS_FILE);
    if (file)
        file << json.dump(4) << '\n';
}

void ClientCore::OpenOptions() {
    _optionsReturn = _scene;
    _option = 0;
    SetScene(Scene::Options);
}

void ClientCore::OptionsFrame(float dt) {
    if (_optionsReturn == Scene::Lobby)
        KeepAlive(dt);

    DrawCentered("OPTIONS", 70, 44, palette::ACCENT);

    const float width = 640.f;
    const float height = 84.f;
    const float x = (static_cast<float>(WINDOW_WIDTH) - width) / 2.f;
    std::optional<std::size_t> clicked;

    for (std::size_t i = 0; i < OPTIONS.size(); i++) {
        const OptionInfo& option = OPTIONS[i];
        const float y = 160.f + static_cast<float>(i) * (height + 16.f);
        const float mx = _display.MouseX();
        const float my = _display.MouseY();
        if (mx >= x && mx <= x + width && my >= y && my <= y + height) {
            _option = i;
            if (_display.IsMouseClicked())
                clicked = i;
        }
        const bool selected = i == _option;
        const bool on = _settings.*option.value;
        const std::string state = on ? "ON" : "OFF";
        _display.FillRect(x, y, width, height, selected ? palette::PANEL_HOVER : palette::PANEL);
        _display.OutlineRect(x, y, width, height, selected ? 4.f : 2.f, selected ? palette::ACCENT : palette::PANEL);
        _display.Print(option.name, static_cast<int>(x) + 24, static_cast<int>(y) + 14, 28, palette::FOREGROUND);
        _display.Print(option.help, static_cast<int>(x) + 24, static_cast<int>(y) + 52, 18, palette::DIM);
        _display.Print(state, static_cast<int>(x + width) - 24 - _display.TextWidth(state, 28), static_cast<int>(y) + 14,
            28, on ? palette::PLAYERS[2] : palette::DIM);
    }

    DrawHint("Move: ARROWS or WASD (ZQSD on French keyboards)  -  Fire: SPACE  -  Auto-fire in game: F", 480, 18);
    DrawHint("UP / DOWN to choose  -  ENTER to switch  -  ESC to go back", 560, 18);

    const std::size_t total = OPTIONS.size();
    if (_display.IsKeyPressed(Key::Down))
        _option = (_option + 1) % total;
    if (_display.IsKeyPressed(Key::Up))
        _option = (_option + total - 1) % total;
    if (_display.IsKeyPressed(Key::Enter) || _display.IsKeyPressed(Key::Space) || _display.IsKeyPressed(Key::Left)
        || _display.IsKeyPressed(Key::Right))
        clicked = _option;

    if (clicked) {
        _option = *clicked;
        bool& value = _settings.*OPTIONS[*clicked].value;
        value = !value;
        SaveSettings();
    } else if (_display.IsKeyPressed(Key::Escape)) {
        SetScene(_optionsReturn);
    }
}

}
