/*
** EPITECH PROJECT, 2026
** T-type
** File description:
** game.cpp
*/

#include "ClientCore.hpp"

namespace r_type::client {

static float WidthOf(std::uint8_t kind) {
    switch (kind) {
        case KIND_PLAYER:
            return 48.f;
        case KIND_ENEMY:
            return 40.f;
        case KIND_PROJECTILE:
            return 16.f;
        default:
            return 20.f;
    }
}

static float HeightOf(std::uint8_t kind) {
    switch (kind) {
        case KIND_PLAYER:
            return 24.f;
        case KIND_ENEMY:
            return 40.f;
        case KIND_PROJECTILE:
            return 6.f;
        default:
            return 20.f;
    }
}

static Rgba ColorOf(std::uint32_t id, const Appearance& look) {
    switch (look.kind) {
        case KIND_PLAYER:
            return palette::PLAYERS[static_cast<std::size_t>(look.color >= 0 ? look.color : static_cast<int>(id)) % 4];
        case KIND_ENEMY:
            return palette::ENEMY;
        case KIND_PROJECTILE:
            return palette::PROJECTILE;
        default:
            return palette::UNKNOWN;
    }
}

void ClientCore::GameFrame() {
    std::uint8_t held = 0;
    held |= _display.IsKeyDown(Key::Up) ? INPUT_UP : 0;
    held |= _display.IsKeyDown(Key::Down) ? INPUT_DOWN : 0;
    held |= _display.IsKeyDown(Key::Left) ? INPUT_LEFT : 0;
    held |= _display.IsKeyDown(Key::Right) ? INPUT_RIGHT : 0;
    held |= _display.IsKeyDown(Key::Space) ? INPUT_FIRE : 0;
    if (held != _held) {
        _held = held;
        _network.Push(request::Input{held});
    }

    DrawEntities();
    _display.Print("ESC to leave the room", 10, 10, 18, palette::DIM);
    if (const auto player = _world.Player()) {
        const Rgba color = palette::PLAYERS[static_cast<std::size_t>(_world.PlayerColor() >= 0 ? _world.PlayerColor() : 0) % 4];
        _display.Print("You are player " + std::to_string(*player), 10, 34, 18, color);
    }
    if (_world.Count() == 0)
        DrawCentered("Waiting for the server...", WINDOW_HEIGHT / 2 - 15, 30, palette::TEXT);

    if (_display.IsKeyPressed(Key::Escape)) {
        _held = 0;
        _network.Push(request::Input{0});
        _network.Push(request::LeaveRoom{});
        SetScene(Scene::Leaving);
    }
}

void ClientCore::DrawEntities() {
    _world.Registry().each<NetIdentity, Transform, Appearance>(
        [this](NetIdentity& net, Transform& pos, Appearance& look) {
            _display.FillRect(pos.x, pos.y, WidthOf(look.kind), HeightOf(look.kind), ColorOf(net.id, look));
        });
}

}
