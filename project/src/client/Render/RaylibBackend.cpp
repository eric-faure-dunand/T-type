/*
** EPITECH PROJECT, 2026
** T-type
** File description:
** RaylibBackend.cpp
*/

#include "RaylibBackend.hpp"

#include <optional>
#include <stdexcept>
#include <vector>
#include <raylib.h>

namespace r_type::render {

struct RaylibBackend::Impl {
    bool open = false;
    std::vector<std::optional<Texture2D>> textures;

    const Texture2D &texture(TextureId id) const
    {
        if (id == kInvalidTexture || id > textures.size() || !textures[id - 1].has_value())
            throw std::invalid_argument("RaylibBackend: unknown texture");
        return *textures[id - 1];
    }

    void unloadAll()
    {
        for (auto &slot : textures) {
            if (slot.has_value())
                UnloadTexture(*slot);
            slot.reset();
        }
        textures.clear();
    }
};

namespace {

int toRaylibKey(input::Key key)
{
    switch (key) {
        case input::Key::Up: return KEY_UP;
        case input::Key::Down: return KEY_DOWN;
        case input::Key::Left: return KEY_LEFT;
        case input::Key::Right: return KEY_RIGHT;
        case input::Key::Space: return KEY_SPACE;
        case input::Key::Enter: return KEY_ENTER;
        case input::Key::Escape: return KEY_ESCAPE;
    }
    return KEY_NULL;
}

}

RaylibBackend::RaylibBackend() : impl_(std::make_unique<Impl>()) {}

RaylibBackend::~RaylibBackend()
{
    close();
}

void RaylibBackend::open(int width, int height, const std::string &title)
{
    if (impl_->open || IsWindowReady())
        throw std::logic_error("RaylibBackend: a window is already open");
    SetTraceLogLevel(LOG_WARNING);
    InitWindow(width, height, title.c_str());
    if (!IsWindowReady())
        throw std::runtime_error("RaylibBackend: cannot create the window");
    SetExitKey(KEY_NULL);
    impl_->open = true;
}

void RaylibBackend::close()
{
    if (!impl_->open)
        return;
    impl_->unloadAll();
    CloseWindow();
    impl_->open = false;
}

bool RaylibBackend::isOpen() const
{
    return impl_->open;
}

bool RaylibBackend::shouldClose() const
{
    return impl_->open && WindowShouldClose();
}

void RaylibBackend::setTargetFps(int fps)
{
    SetTargetFPS(fps);
}

void RaylibBackend::beginFrame(Color clear)
{
    BeginDrawing();
    ClearBackground(::Color{clear.r, clear.g, clear.b, clear.a});
}

void RaylibBackend::endFrame()
{
    EndDrawing();
}

TextureId RaylibBackend::loadTexture(const std::string &path)
{
    if (!impl_->open)
        throw std::logic_error("RaylibBackend: open the window before loading textures");
    Texture2D texture = LoadTexture(path.c_str());
    if (texture.id == 0)
        throw std::runtime_error("RaylibBackend: cannot load texture '" + path + "'");
    impl_->textures.push_back(texture);
    return static_cast<TextureId>(impl_->textures.size()); // index + 1
}

void RaylibBackend::unloadTexture(TextureId texture)
{
    if (texture == kInvalidTexture || texture > impl_->textures.size())
        return;
    auto &slot = impl_->textures[texture - 1];
    if (slot.has_value()) {
        UnloadTexture(*slot);
        slot.reset();
    }
}

Size RaylibBackend::textureSize(TextureId texture) const
{
    const Texture2D &t = impl_->texture(texture);
    return Size{t.width, t.height};
}

void RaylibBackend::drawTexture(TextureId texture, Rect source, float x, float y, float scale)
{
    const Texture2D &t = impl_->texture(texture);
    if (source.w <= 0.f || source.h <= 0.f)
        source = Rect{0.f, 0.f, static_cast<float>(t.width), static_cast<float>(t.height)};
    const Rectangle src{source.x, source.y, source.w, source.h};
    const Rectangle dest{x, y, source.w * scale, source.h * scale};
    DrawTexturePro(t, src, dest, Vector2{0.f, 0.f}, 0.f, ::Color{255, 255, 255, 255}); // not the WHITE macro: it would expand to r_type::render::Color
}

bool RaylibBackend::isKeyDown(input::Key key) const
{
    return impl_->open && IsKeyDown(toRaylibKey(key));
}

bool RaylibBackend::isKeyPressed(input::Key key) const
{
    return impl_->open && IsKeyPressed(toRaylibKey(key));
}

}