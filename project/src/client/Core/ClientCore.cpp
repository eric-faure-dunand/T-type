/*
** EPITECH PROJECT, 2026
** T-type
** File description:
** ClientCore.cpp
*/

#include <random>

#include "ClientCore.hpp"

namespace r_type::client {

ClientCore::ClientCore(std::string ip): _display(WINDOW_WIDTH, WINDOW_HEIGHT, "R-Type"), _ip(std::move(ip)) {
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
    _network.Start();
    while (!_quit && !_display.ShouldClose()) {
        const float dt = _display.FrameTime();
        HandleEvents();
        ScrollStars(dt);
        _display.BeginFrame(palette::BACKGROUND);
        DrawStars();
        switch (_scene) {
            case Scene::Menu:
                MenuFrame();
                break;
            case Scene::Connecting:
                WaitingFrame("Connecting to " + _ip + "...", dt);
                break;
            case Scene::Lobby:
                LobbyFrame(dt);
                break;
            case Scene::Joining:
                WaitingFrame("Entering the room...", dt);
                break;
            case Scene::Game:
                GameFrame();
                break;
            case Scene::Leaving:
                WaitingFrame("Going back to the rooms...", dt);
                break;
        }
        DrawMessage(dt);
        _display.EndFrame();
    }
    _network.Stop();
}

void ClientCore::HandleEvents() {
    for (const NetEvent& event : _network.ConsumeEvents())
        HandleEvent(event);
}

void ClientCore::HandleEvent(const NetEvent& event) {
    std::visit(Overloaded{
        [this](const event::Connected&) {
            _rooms.clear();
            _selected = 0;
            SetScene(Scene::Lobby);
        },
        [this](const event::ConnectionFailed& e) {
            SetScene(Scene::Menu);
            ShowMessage(e.reason);
        },
        [this](const event::Disconnected& e) {
            _world.Clear();
            SetScene(Scene::Menu);
            ShowMessage(e.reason);
        },
        [this](const event::Notice& e) {
            if (_scene == Scene::Joining)
                SetScene(Scene::Lobby);
            ShowMessage(e.message);
        },
        [this](const event::Rooms& e) {
            _rooms = e.rooms;
            if (_selected >= _rooms.size())
                _selected = _rooms.empty() ? 0 : _rooms.size() - 1;
        },
        [this](const event::RoomEntered&) {
            _world.Clear();
            _held = 0;
            SetScene(Scene::Game);
        },
        [this](const event::BackToLobby&) {
            _world.Clear();
            SetScene(Scene::Lobby);
        },
        [this](const event::PlayerCreated& e) {
            _world.SetPlayer(e.id, e.x, e.y, e.color);
        },
        [this](const event::Spawn& e) {
            _world.Spawn(e.entity);
        },
        [this](const event::Update& e) {
            _world.Update(e.id, e.x, e.y);
        },
        [this](const event::Destroy& e) {
            _world.Destroy(e.id);
        },
        [this](const event::Snapshot& e) {
            _world.Replace(e.entities);
        },
    }, event);
}

void ClientCore::SetScene(Scene scene) {
    _scene = scene;
    _waitTimer = 0.f;
    _refreshTimer = 0.f;
}

void ClientCore::ShowMessage(const std::string& message) {
    _message = message;
    _messageTime = MESSAGE_DURATION;
}

void ClientCore::WaitingFrame(const std::string& text, float dt) {
    _waitTimer += dt;
    const int dots = static_cast<int>(_display.Time() * 2) % 4;
    DrawCentered(text.substr(0, text.size() - 3) + std::string(static_cast<std::size_t>(dots), '.'),
        WINDOW_HEIGHT / 2 - 20, 30, palette::TEXT);
    DrawCentered("ESC to cancel", WINDOW_HEIGHT / 2 + 30, 20, palette::DIM);
    const bool cancel = _display.IsKeyPressed(Key::Escape);
    if (_scene == Scene::Connecting && cancel) {
        _network.Push(request::Disconnect{});
        SetScene(Scene::Menu);
    } else if (_scene == Scene::Joining && (cancel || _waitTimer > WAIT_TIMEOUT)) {
        if (!cancel)
            ShowMessage("The server does not answer");
        _network.Push(request::LeaveRoom{});
        SetScene(Scene::Lobby);
    } else if (_scene == Scene::Leaving && _waitTimer > WAIT_TIMEOUT) {
        _network.Push(request::Disconnect{});
        _world.Clear();
        SetScene(Scene::Menu);
        ShowMessage("Connection to the server lost");
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
    _display.FillRect(x, static_cast<float>(WINDOW_HEIGHT - 90), static_cast<float>(width), 44.f, palette::PANEL);
    _display.OutlineRect(x, static_cast<float>(WINDOW_HEIGHT - 90), static_cast<float>(width), 44.f, 2.f,
        palette::ALERT);
    DrawCentered(_message, WINDOW_HEIGHT - 79, 22, palette::TEXT);
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
    _display.Print(label, textX, static_cast<int>(y + (height - 20.f) / 2.f), 20, palette::TEXT);
    return hover && _display.IsMouseClicked();
}

}
