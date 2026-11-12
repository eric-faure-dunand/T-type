/*
** EPITECH PROJECT, 2026
** T-type
** File description:
** Display.cpp
*/

#include <stdexcept>

#include <raylib.h>

#include "Display.hpp"

namespace r_type::client {

static ::Color ToRaylib(Rgba color) {
    return ::Color{color.r, color.g, color.b, color.a};
}

static ::Texture2D ToRaylib(const Sprite& sprite) {
    return ::Texture2D{sprite.id, sprite.width, sprite.height, sprite.mipmaps, sprite.format};
}

static ::Font ToRaylib(const Typeface& font) {
    return ::Font{font.baseSize, font.glyphCount, font.glyphPadding, ToRaylib(font.texture),
        static_cast<::Rectangle *>(font.recs), static_cast<::GlyphInfo *>(font.glyphs)};
}

static ::Font FontOf(const std::map<std::string, Typeface>& fonts, const std::string& name) {
    const auto it = fonts.find(name);
    if (it == fonts.end())
        return GetFontDefault();
    return ToRaylib(it->second);
}

static float SpacingOf(const ::Font& font, int size) {
    return font.texture.id == GetFontDefault().texture.id ? static_cast<float>(size) / 10.f : 0.f;
}

static std::vector<int> Codepoints() {
    std::vector<int> codepoints;
    for (int codepoint = 32; codepoint < 127; codepoint++)
        codepoints.push_back(codepoint);
    for (int codepoint = 160; codepoint < 384; codepoint++)
        codepoints.push_back(codepoint);
    return codepoints;
}

static int ToRaylib(Key key) {
    switch (key) {
        case Key::Up:
            return KEY_UP;
        case Key::Down:
            return KEY_DOWN;
        case Key::Left:
            return KEY_LEFT;
        case Key::Right:
            return KEY_RIGHT;
        case Key::Space:
            return KEY_SPACE;
        case Key::Enter:
            return KEY_ENTER;
        case Key::Escape:
            return KEY_ESCAPE;
        case Key::Backspace:
            return KEY_BACKSPACE;
        case Key::C:
            return KEY_C;
        case Key::R:
            return KEY_R;
    }
    return KEY_NULL;
}

Display::Display(int width, int height, const std::string& title): _width(width), _height(height) {
    SetTraceLogLevel(LOG_WARNING);
    SetConfigFlags(FLAG_VSYNC_HINT);
    InitWindow(width, height, title.c_str());
    if (!IsWindowReady())
        throw std::runtime_error("Display: cannot open the window.");
    SetExitKey(KEY_NULL);
}

Display::~Display() {
    for (const auto& [name, sprite] : _sprites)
        UnloadTexture(ToRaylib(sprite));
    for (const auto& [name, font] : _fonts)
        UnloadFont(ToRaylib(font));
    CloseWindow();
}

bool Display::ShouldClose() const {
    return WindowShouldClose();
}

float Display::FrameTime() const {
    return GetFrameTime();
}

double Display::Time() const {
    return GetTime();
}

void Display::BeginFrame(Rgba clear) {
    BeginDrawing();
    ClearBackground(ToRaylib(clear));
}

void Display::EndFrame() {
    EndDrawing();
}

void Display::FillRect(float x, float y, float width, float height, Rgba color) {
    DrawRectangleV(Vector2{x, y}, Vector2{width, height}, ToRaylib(color));
}

void Display::OutlineRect(float x, float y, float width, float height, float thickness, Rgba color) {
    DrawRectangleLinesEx(Rectangle{x, y, width, height}, thickness, ToRaylib(color));
}

void Display::FillCircle(float x, float y, float radius, Rgba color) {
    DrawCircleV(Vector2{x, y}, radius, ToRaylib(color));
}

void Display::FillGradient(float x, float y, float width, float height, Rgba left, Rgba right) {
    DrawRectangleGradientH(static_cast<int>(x), static_cast<int>(y), static_cast<int>(width), static_cast<int>(height),
        ToRaylib(left), ToRaylib(right));
}

void Display::Print(const std::string& text, int x, int y, int size, Rgba color, const std::string& font) {
    const ::Font face = FontOf(_fonts, font.empty() ? _font : font);
    const float spacing = SpacingOf(face, size);
    const float shadow = std::max(1.f, static_cast<float>(size) / 14.f);
    const ::Color dark{0, 0, 0, static_cast<unsigned char>(color.a * 3 / 4)};
    DrawTextEx(face, text.c_str(), Vector2{static_cast<float>(x) + shadow, static_cast<float>(y) + shadow},
        static_cast<float>(size), spacing, dark);
    DrawTextEx(face, text.c_str(), Vector2{static_cast<float>(x), static_cast<float>(y)}, static_cast<float>(size),
        spacing, ToRaylib(color));
}

int Display::TextWidth(const std::string& text, int size, const std::string& font) const {
    const ::Font face = FontOf(_fonts, font.empty() ? _font : font);
    return static_cast<int>(MeasureTextEx(face, text.c_str(), static_cast<float>(size), SpacingOf(face, size)).x);
}

bool Display::LoadFont(const std::string& name, const std::string& path) {
    if (!FileExists(path.c_str()))
        return false;
    std::vector<int> codepoints = Codepoints();
    ::Font font = LoadFontEx(path.c_str(), FONT_RESOLUTION, codepoints.data(), static_cast<int>(codepoints.size()));
    if (font.texture.id == 0 || font.texture.id == GetFontDefault().texture.id)
        return false;
    GenTextureMipmaps(&font.texture);
    SetTextureFilter(font.texture, TEXTURE_FILTER_TRILINEAR);
    const auto old = _fonts.find(name);
    if (old != _fonts.end())
        UnloadFont(ToRaylib(old->second));
    const ::Texture2D& texture = font.texture;
    _fonts[name] = Typeface{font.baseSize, font.glyphCount, font.glyphPadding,
        Sprite{texture.id, texture.width, texture.height, texture.mipmaps, texture.format}, font.recs, font.glyphs};
    return true;
}

void Display::UseFont(const std::string& name) {
    _font = name;
}

bool Display::LoadSprite(const std::string& name, const std::string& path) {
    if (!FileExists(path.c_str()))
        return false;
    const ::Texture2D texture = LoadTexture(path.c_str());
    if (texture.id == 0)
        return false;
    SetTextureFilter(texture, TEXTURE_FILTER_BILINEAR);
    const auto old = _sprites.find(name);
    if (old != _sprites.end())
        UnloadTexture(ToRaylib(old->second));
    _sprites[name] = Sprite{texture.id, texture.width, texture.height, texture.mipmaps, texture.format};
    return true;
}

bool Display::HasSprite(const std::string& name) const {
    return _sprites.contains(name);
}

void Display::DrawSprite(const std::string& name, float x, float y, float width, float height, std::uint8_t alpha) {
    const auto it = _sprites.find(name);
    if (it == _sprites.end())
        return;
    const ::Texture2D texture = ToRaylib(it->second);
    const Rectangle source{0.f, 0.f, static_cast<float>(texture.width), static_cast<float>(texture.height)};
    DrawTexturePro(texture, source, Rectangle{x, y, width, height}, Vector2{0.f, 0.f}, 0.f, ::Color{255, 255, 255, alpha});
}

bool Display::IsKeyDown(Key key) const {
    return ::IsKeyDown(ToRaylib(key));
}

bool Display::IsKeyPressed(Key key) const {
    return ::IsKeyPressed(ToRaylib(key)) || ::IsKeyPressedRepeat(ToRaylib(key));
}

std::string Display::TypedText() const {
    std::string text;
    for (int codepoint = GetCharPressed(); codepoint > 0; codepoint = GetCharPressed())
        if (codepoint < 128)
            text += static_cast<char>(codepoint);
    return text;
}

float Display::MouseX() const {
    return static_cast<float>(GetMouseX());
}

float Display::MouseY() const {
    return static_cast<float>(GetMouseY());
}

bool Display::IsMouseClicked() const {
    return IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
}

}
