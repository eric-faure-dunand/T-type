/*
** EPITECH PROJECT, 2026
** T-type
** File description:
** ClientCore.cpp
*/

#include <random>

#include "ClientCore.hpp"

namespace r_type::client {

ClientCore::ClientCore(std::string ip):
    _display(WINDOW_WIDTH, WINDOW_HEIGHT, "R-Type"), _net(Role::Client), _ip(std::move(ip)) {
    RegisterHandlers();
    std::mt19937 rng(std::random_device{}());
    std::uniform_real_distribution<float> x(0.f, static_cast<float>(WINDOW_WIDTH));
    std::uniform_real_distribution<float> y(0.f, static_cast<float>(WINDOW_HEIGHT));
    std::uniform_real_distribution<float> depth(0.f, 1.f);
    for (int i = 0; i < 150; i++) {
        const float d = depth(rng);
        _stars.push_back({x(rng), y(rng), 20.f + d * 80.f, 1.f + d * 2.f});
    }
}

void ClientCore::Run() {
    while (!_quit && !_display.ShouldClose()) {
        const float dt = _display.FrameTime();
        if (_net.IsOpen())
            _net.Poll();
        _sceneTime += dt;
        CheckTimeouts();
        ScrollStars(dt);
        _display.BeginFrame(palette::BACKGROUND);
        DrawStars();
        switch (_scene) {
            case Scene::Menu:
                MenuFrame();
                break;
            case Scene::Connecting:
                WaitingFrame("Connecting to " + _ip + "...");
                break;
            case Scene::Lobby:
                LobbyFrame(dt);
                break;
            case Scene::Joining:
                WaitingFrame("Entering the room...");
                break;
            case Scene::Game:
                GameFrame(dt);
                break;
            case Scene::Leaving:
                WaitingFrame("Going back to the rooms...");
                break;
        }
        DrawMessage(dt);
        _display.EndFrame();
    }
    SayGoodbye();
    _net.Close();
}

void ClientCore::SetScene(Scene scene) {
    _scene = scene;
    _sceneTime = 0.f;
    _refreshTimer = 0.f;
}

void ClientCore::ShowMessage(const std::string& message) {
    _message = message;
    _messageTime = MESSAGE_DURATION;
}

void ClientCore::CheckTimeouts() {
    if (_sceneTime < CONNECT_LIMIT)
        return;
    if (_scene == Scene::Connecting) {
        GoOffline("The server does not answer");
    } else if (_scene == Scene::Joining) {
        ShowMessage("The room does not answer");
        if (_roomPort != 0)
            ReturnToLobby();
        else
            SetScene(Scene::Lobby);
    } else if (_scene == Scene::Leaving) {
        GoOffline("Connection to the server lost");
    }
}

void ClientCore::WaitingFrame(const std::string& text) {
    const int dots = static_cast<int>(_display.Time() * 2) % 4;
    DrawCentered(text.substr(0, text.size() - 3) + std::string(static_cast<std::size_t>(dots), '.'),
        WINDOW_HEIGHT / 2 - 20, 30, palette::FOREGROUND);
    if (_scene == Scene::Leaving)
        return;
    DrawCentered("ESC to cancel", WINDOW_HEIGHT / 2 + 30, 20, palette::DIM);
    if (!_display.IsKeyPressed(Key::Escape))
        return;
    if (_scene == Scene::Connecting) {
        SayGoodbye();
        GoOffline("");
    } else if (_roomPort != 0) {
        Send(Command::DisconnectRoom, {{0u}});
        ReturnToLobby();
    } else {
        SetScene(Scene::Lobby);
    }
}

void ClientCore::ScrollStars(float dt) {
    for (Star& star : _stars) {
        star.x -= star.speed * dt;
        if (star.x < 0.f)
            star.x += static_cast<float>(WINDOW_WIDTH);
    }
}

void ClientCore::DrawStars() {
    for (const Star& star : _stars) {
        const auto alpha = static_cast<std::uint8_t>(80.f + star.size * 55.f);
        _display.FillCircle(star.x, star.y, star.size, {255, 255, 255, alpha});
    }
}

void ClientCore::DrawMessage(float dt) {
    if (_messageTime <= 0.f)
        return;
    _messageTime -= dt;
    const int width = _display.TextWidth(_message, 22) + 40;
    const float x = static_cast<float>((WINDOW_WIDTH - width) / 2);
    const float y = static_cast<float>(WINDOW_HEIGHT - 90);
    _display.FillRect(x, y, static_cast<float>(width), 44.f, palette::PANEL);
    _display.OutlineRect(x, y, static_cast<float>(width), 44.f, 2.f, palette::ALERT);
    DrawCentered(_message, WINDOW_HEIGHT - 79, 22, palette::FOREGROUND);
}

void ClientCore::DrawCentered(const std::string& text, int y, int size, Rgba color) {
    _display.Print(text, (WINDOW_WIDTH - _display.TextWidth(text, size)) / 2, y, size, color);
}

bool ClientCore::Button(const std::string& label, float x, float y, float width, float height) {
    const float mx = _display.MouseX();
    const float my = _display.MouseY();
    const bool hover = mx >= x && mx <= x + width && my >= y && my <= y + height;
    _display.FillRect(x, y, width, height, hover ? palette::PANEL_HOVER : palette::PANEL);
    _display.OutlineRect(x, y, width, height, 2.f, hover ? palette::ACCENT : palette::DIM);
    const int textX = static_cast<int>(x + (width - static_cast<float>(_display.TextWidth(label, 20))) / 2.f);
    _display.Print(label, textX, static_cast<int>(y + (height - 20.f) / 2.f), 20, palette::FOREGROUND);
    return hover && _display.IsMouseClicked();
}

}
