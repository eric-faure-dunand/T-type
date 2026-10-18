/*
** EPITECH PROJECT, 2026
** T-type
** File description:
** main.cpp
*/

#include <array>
#include <chrono>
#include <random>
#include <string>
#include <vector>

#include <raylib.h>

#include "shared/ECS/Registry.hpp"
#include "shared/ECS/SystemManager.hpp"
#include "shared/Game/Gameplay.hpp"
#include "shared/Loop/FixedTimestep.hpp"

namespace ecs = r_type::ecs;
namespace game = r_type::game;

namespace {

struct Star {
    float x;
    float y;
    float speed;
    float size;
};

const std::array<Color, 4> kPlayerColors = {SKYBLUE, PINK, LIME, GOLD};

std::vector<Star> makeStarfield(std::mt19937 &rng)
{
    std::uniform_real_distribution<float> x(0.f, game::kWorldWidth);
    std::uniform_real_distribution<float> y(0.f, game::kWorldHeight);
    std::uniform_real_distribution<float> depth(0.f, 1.f);
    std::vector<Star> stars(150);
    for (Star &s : stars) {
        const float d = depth(rng);
        s = Star{x(rng), y(rng), 20.f + d * 80.f, 1.f + d * 2.f};
    }
    return stars;
}

void scrollStarfield(std::vector<Star> &stars, float dt)
{
    for (Star &s : stars) {
        s.x -= s.speed * dt;
        if (s.x < 0.f)
            s.x += game::kWorldWidth;
    }
}

void resetGame(ecs::Registry &registry)
{
    std::vector<ecs::Entity> all;
    registry.each<game::Kind>([&all](ecs::Entity e, game::Kind &) { all.push_back(e); });
    for (const ecs::Entity e : all)
        registry.kill(e);
    game::spawnPlayer(registry, 1, 100.f, game::kWorldHeight / 3.f);
    game::spawnPlayer(registry, 2, 100.f, game::kWorldHeight * 2.f / 3.f);
}

void readKeyboard(ecs::Registry &registry)
{
    registry.each<game::Player, game::Input>([](game::Player &player, game::Input &in) {
        if (player.id == 1)
            in = game::Input{IsKeyDown(KEY_UP), IsKeyDown(KEY_DOWN), IsKeyDown(KEY_LEFT), IsKeyDown(KEY_RIGHT),
                IsKeyDown(KEY_SPACE)};
        else if (player.id == 2)
            in = game::Input{IsKeyDown(KEY_W), IsKeyDown(KEY_S), IsKeyDown(KEY_A), IsKeyDown(KEY_D),
                IsKeyDown(KEY_LEFT_SHIFT)};
    });
}

Color colorOf(ecs::Registry &registry, ecs::Entity e, game::EntityKind kind)
{
    switch (kind) {
        case game::EntityKind::Player:
            return kPlayerColors[static_cast<std::size_t>(registry.get<game::Player>(e).id - 1) % kPlayerColors.size()];
        case game::EntityKind::Enemy:
            return MAROON;
        case game::EntityKind::PlayerMissile:
            return YELLOW;
        case game::EntityKind::EnemyMissile:
            return ORANGE;
    }
    return WHITE;
}

void draw(ecs::Registry &registry, const std::vector<Star> &stars)
{
    BeginDrawing();
    ClearBackground(Color{8, 8, 20, 255});
    for (const Star &s : stars)
        DrawCircleV(Vector2{s.x, s.y}, s.size, Fade(RAYWHITE, s.size / 3.f));

    registry.each<game::Position, game::Hitbox, game::Kind>(
        [&registry](ecs::Entity e, game::Position &pos, game::Hitbox &box, game::Kind &kind) {
            DrawRectangleV(Vector2{pos.x, pos.y}, Vector2{box.width, box.height}, colorOf(registry, e, kind.value));
        });

    int line = 0;
    registry.each<game::Player, game::Health>([&line](game::Player &player, game::Health &hp) {
        const std::string text = "P" + std::to_string(player.id) + "  HP " + std::to_string(hp.current);
        DrawText(text.c_str(), 10, 10 + 24 * line++, 20, kPlayerColors[static_cast<std::size_t>(player.id - 1) % 4]);
    });
    if (line == 0)
        DrawText("GAME OVER - press ENTER", 440, 340, 30, RAYWHITE);
    DrawText("P1: arrows + space   P2: ZQSD + shift", 10, static_cast<int>(game::kWorldHeight) - 26, 18, GRAY);
    DrawFPS(static_cast<int>(game::kWorldWidth) - 90, 10);
    EndDrawing();
}

}

int main()
{
    InitWindow(static_cast<int>(game::kWorldWidth), static_cast<int>(game::kWorldHeight), "R-Type - gameplay demo");
    SetTargetFPS(144);

    ecs::Registry registry;
    ecs::SystemManager systems;
    game::registerGameplayComponents(registry);
    game::addGameplaySystems(systems, std::random_device{}());
    resetGame(registry);

    std::mt19937 rng(std::random_device{}());
    std::vector<Star> stars = makeStarfield(rng);

    r_type::loop::FixedTimestep loop(60);
    auto previous = std::chrono::steady_clock::now();

    while (!WindowShouldClose()) {
        bool anyPlayer = false;
        registry.each<game::Player>([&anyPlayer](game::Player &) { anyPlayer = true; });
        if (!anyPlayer && IsKeyPressed(KEY_ENTER))
            resetGame(registry);

        readKeyboard(registry);
        const auto now = std::chrono::steady_clock::now();
        loop.advance(std::chrono::duration_cast<r_type::loop::FixedTimestep::Duration>(now - previous),
            [&](float dt) { systems.run(registry, dt); });
        previous = now;

        scrollStarfield(stars, GetFrameTime());
        draw(registry, stars);
    }
    CloseWindow();
    return 0;
}
