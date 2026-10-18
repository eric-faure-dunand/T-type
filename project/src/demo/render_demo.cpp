/*
** EPITECH PROJECT, 2026
** T-type
** File description:
** render_demo.cpp
*/

#include <chrono>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <exception>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#include "client/Render/RaylibBackend.hpp"
#include "client/Render/RenderSystem.hpp"
#include "client/Render/Sprite.hpp"
#include "client/Render/TextureManager.hpp"
#include "shared/Components/Transform.hpp"
#include "shared/ECS/Registry.hpp"
#include "shared/ECS/SystemManager.hpp"
#include "shared/Loop/FixedTimestep.hpp"

using namespace r_type;

namespace {

constexpr int kWindowW = 800;
constexpr int kWindowH = 450;
constexpr int kImageSize = 32;
constexpr float kScale = 3.f;
constexpr float kSpriteSize = kImageSize * kScale;
constexpr float kPlayerSpeed = 300.f;

std::string makeTestImage()
{
    const int w = kImageSize;
    const int h = kImageSize;
    const int rowSize = (w * 3 + 3) / 4 * 4;
    const std::uint32_t dataSize = static_cast<std::uint32_t>(rowSize * h);
    std::vector<std::uint8_t> bytes(54 + dataSize, 0);
    auto put32 = [&](std::size_t at, std::uint32_t v) {
        for (int i = 0; i < 4; ++i)
            bytes[at + static_cast<std::size_t>(i)] = static_cast<std::uint8_t>((v >> (8 * i)) & 0xFFu);
    };
    bytes[0] = 'B';
    bytes[1] = 'M';
    put32(2, static_cast<std::uint32_t>(bytes.size()));
    put32(10, 54);
    put32(14, 40);
    put32(18, static_cast<std::uint32_t>(w));
    put32(22, static_cast<std::uint32_t>(h));
    bytes[26] = 1;
    bytes[28] = 24;
    put32(34, dataSize);
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            std::uint8_t r = ((x / 4 + y / 4) % 2 == 0) ? 220 : 255;
            std::uint8_t g = ((x / 4 + y / 4) % 2 == 0) ? 40 : 255;
            std::uint8_t b = ((x / 4 + y / 4) % 2 == 0) ? 40 : 255;
            if (y < 6) { r = 0; g = 200; b = 0; }
            if (x < 2 || y >= h - 2 || x >= w - 2) { r = 20; g = 60; b = 255; }
            const std::size_t file_row = static_cast<std::size_t>(h - 1 - y);
            const std::size_t at = 54 + file_row * static_cast<std::size_t>(rowSize) + static_cast<std::size_t>(x) * 3;
            bytes[at] = b;
            bytes[at + 1] = g;
            bytes[at + 2] = r;
        }
    }
    const std::string path = (std::filesystem::temp_directory_path() / "rtype_demo_sprite.bmp").string();
    std::ofstream out(path, std::ios::binary);
    out.write(reinterpret_cast<const char *>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
    return path;
}

}

int main(int argc, char **argv)
{
    std::setvbuf(stdout, nullptr, _IOLBF, 0);
    const int displayFps = argc > 1 ? std::atoi(argv[1]) : 60;
    try {
        const std::string image = argc > 2 ? argv[2] : makeTestImage();

        render::RaylibBackend backend;
        backend.open(kWindowW, kWindowH, "R-Type render demo (arrows, Esc)");
        backend.setTargetFps(displayFps);
        render::TextureManager textures(backend);
        const render::TextureId texture = textures.get(image);

        ecs::Registry registry;
        registry.registerComponent<components::Position>();
        registry.registerComponent<components::Velocity>();
        registry.registerComponent<render::Sprite>();

        const ecs::Entity player = registry.spawn();
        registry.add<components::Position>(player, 100.f, 200.f);
        registry.add<components::Velocity>(player);
        registry.add<render::Sprite>(player, render::Sprite{texture, render::Rect{}, kScale, 1});

        const ecs::Entity bouncer = registry.spawn();
        registry.add<components::Position>(bouncer, 300.f, 200.f);
        registry.add<components::Velocity>(bouncer, 240.f, 0.f);
        registry.add<render::Sprite>(bouncer, render::Sprite{texture, render::Rect{}, kScale, 0});

        const ecs::Entity ghost = registry.spawn();
        registry.add<components::Position>(ghost, 600.f, 100.f);

        ecs::SystemManager systems;
        systems.add("movement", [](ecs::Registry &r, float dt) {
            r.each<components::Position, components::Velocity>(
                [dt](components::Position &p, const components::Velocity &v) {
                    p.x += v.dx * dt;
                    p.y += v.dy * dt;
                });
        });
        systems.add("bounce_and_clamp", [bouncer](ecs::Registry &r, float) {
            r.each<components::Position, components::Velocity>(
                [bouncer](ecs::Entity e, components::Position &p, components::Velocity &v) {
                    const float maxX = kWindowW - kSpriteSize;
                    const float maxY = kWindowH - kSpriteSize;
                    if (p.x < 0.f) { p.x = 0.f; if (e == bouncer) v.dx = -v.dx; }
                    if (p.x > maxX) { p.x = maxX; if (e == bouncer) v.dx = -v.dx; }
                    if (p.y < 0.f) p.y = 0.f;
                    if (p.y > maxY) p.y = maxY;
                });
        });

        loop::FixedTimestep loop(60);
        auto previous = std::chrono::steady_clock::now();
        std::printf("display cap: %d fps, simulation: 60 Hz fixed steps\n", displayFps);

        while (!backend.shouldClose() && !backend.isKeyPressed(input::Key::Escape)) {
            const auto now = std::chrono::steady_clock::now();

            components::Velocity &v = registry.get<components::Velocity>(player);
            v.dx = ((backend.isKeyDown(input::Key::Right) ? 1.f : 0.f) - (backend.isKeyDown(input::Key::Left) ? 1.f : 0.f)) * kPlayerSpeed;
            v.dy = ((backend.isKeyDown(input::Key::Down) ? 1.f : 0.f) - (backend.isKeyDown(input::Key::Up) ? 1.f : 0.f)) * kPlayerSpeed;

            loop.advance(now - previous, [&](float dt) {
                systems.run(registry, dt);
                if (loop.tick() % 60 == 0)
                    std::printf("tick=%llu bouncer.x=%.3f\n", static_cast<unsigned long long>(loop.tick()),
                        static_cast<double>(registry.get<components::Position>(bouncer).x));
            });
            previous = now;

            backend.beginFrame(render::Color{15, 15, 30, 255});
            render::renderSystem(registry, backend);
            backend.endFrame();
        }
        std::fflush(stdout);
    } catch (const std::exception &e) {
        std::fprintf(stderr, "render_demo: %s\n", e.what());
        return 1;
    }
    return 0;
}