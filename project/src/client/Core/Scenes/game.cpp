/*
** EPITECH PROJECT, 2026
** T-type
** File description:
** game.cpp
*/

#include "ClientCore.hpp"

namespace r_type::client {

static bool IsBoss(const Appearance& look) {
    return look.kind == KIND_ENEMY && look.color >= FIRST_BOSS;
}

static float WidthOf(const Appearance& look) {
    if (IsBoss(look))
        return BOSS_SIZE;
    switch (look.kind) {
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

static float HeightOf(const Appearance& look) {
    if (IsBoss(look))
        return BOSS_SIZE;
    switch (look.kind) {
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

static const char *SpriteOf(const Appearance& look) {
    if (look.color < 0)
        return nullptr;
    const auto variant = static_cast<std::size_t>(look.color);
    switch (look.kind) {
        case KIND_ENEMY:
            return variant < ENEMY_SPRITES.size() ? ENEMY_SPRITES[variant] : nullptr;
        case KIND_PROJECTILE:
            return variant < MISSILE_SPRITES.size() ? MISSILE_SPRITES[variant] : nullptr;
        default:
            return nullptr;
    }
}

static Rgba ColorOf(const Appearance& look) {
    switch (look.kind) {
        case KIND_PLAYER:
            return palette::PLAYERS[look.color];
        case KIND_ENEMY:
            return palette::ENEMY;
        case KIND_PROJECTILE:
            return palette::PROJECTILE;
        default:
            return palette::OTHER;
    }
}

void ClientCore::GameFrame(float dt) {
    if (!_gameOver) {
        std::set<Action> held;
        for (const Binding& binding : BINDINGS)
            if (_display.IsKeyDown(binding.key))
                held.insert(binding.action);
        if (held != _held)
            SetHeld(held);
        ResendInputs(dt);
    }

    DrawBackground();
    DrawEntities();
    DrawBossBar();
    _display.Print("ESC to leave the room", 10, 10, 18, palette::DIM);
    const int character = _world.PlayerColor();
    if (_world.Player() && character >= 0 && character < static_cast<int>(CHARACTERS.size()))
        _display.Print(std::string("You are ") + CHARACTERS[static_cast<std::size_t>(character)].name, 10, 34, 18,
            palette::PLAYERS[character]);
    if (_world.Count() == 0)
        DrawCentered("Waiting for the server...", WINDOW_HEIGHT / 2 - 15, 30, palette::FOREGROUND);

    if (_gameOver)
        GameOverFrame(dt);
    else if (_display.IsKeyPressed(Key::Escape))
        LeaveRoom();
}

void ClientCore::GameOverFrame(float dt) {
    *_gameOver += dt;
    _display.FillRect(0.f, 0.f, static_cast<float>(WINDOW_WIDTH), static_cast<float>(WINDOW_HEIGHT),
        palette::GAME_OVER_SHADE);
    DrawCentered("GAME OVER", WINDOW_HEIGHT / 2 - 70, 80, palette::ALERT);
    DrawCentered("Your ship was destroyed", WINDOW_HEIGHT / 2 + 25, 24, palette::FOREGROUND);
    if (*_gameOver < GAME_OVER_DELAY)
        return;
    DrawHint("ENTER or ESC to go back to the rooms", WINDOW_HEIGHT / 2 + 70, 20);
    if (_display.IsKeyPressed(Key::Enter) || _display.IsKeyPressed(Key::Escape))
        LeaveRoom();
}

void ClientCore::LeaveRoom() {
    Send(Command::DisconnectRoom, {{_world.Player().value_or(0)}});
    ReturnToLobby();
}

void ClientCore::DrawBackground() {
    const int character = _world.PlayerColor();
    const char *country = nullptr;
    if (_level)
        country = LEVELS[*_level].country;
    else if (character >= 0 && character < static_cast<int>(CHARACTERS.size()))
        country = CHARACTERS[static_cast<std::size_t>(character)].country;
    if (!country)
        return;
    if (!_display.HasSprite(country))
        return;
    const auto width = static_cast<float>(WINDOW_WIDTH);
    const auto height = static_cast<float>(WINDOW_HEIGHT);
    _display.DrawSprite(country, 0.f, 0.f, width, height);
    _display.FillRect(0.f, 0.f, width, height, palette::BACKGROUND_SHADE);
}

void ClientCore::DrawBossBar() {
    if (!_boss)
        return;
    const float width = 420.f;
    const float height = 14.f;
    const float x = (static_cast<float>(WINDOW_WIDTH) - width) / 2.f;
    const float y = 42.f;
    DrawCentered(_boss->name, 14, 22, palette::FOREGROUND);
    _display.FillRect(x, y, width, height, palette::PANEL);
    _display.FillRect(x, y, width * static_cast<float>(_boss->percent) / 100.f, height, palette::BOSS_HEALTH);
    _display.OutlineRect(x, y, width, height, 2.f, palette::FOREGROUND);
}

void ClientCore::DrawEntities() {
    _world.Registry().each<NetIdentity, Transform, Appearance>(
        [this](NetIdentity&, Transform& pos, Appearance& look) {
            if (look.kind == KIND_PLAYER && look.color >= 0 && look.color < static_cast<int>(CHARACTERS.size())) {
                const float size = WidthOf(look);
                DrawFace(static_cast<std::size_t>(look.color), pos.x, pos.y + (HeightOf(look) - size) / 2.f, size);
                return;
            }
            const char *sprite = SpriteOf(look);
            if (sprite && _display.HasSprite(sprite)) {
                _display.DrawSprite(sprite, pos.x, pos.y, WidthOf(look), HeightOf(look));
                return;
            }
            _display.FillRect(pos.x, pos.y, WidthOf(look), HeightOf(look), ColorOf(look));
        });
}

}
