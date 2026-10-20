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

void Display::Print(const std::string& text, int x, int y, int size, Rgba color) {
    DrawText(text.c_str(), x, y, size, ToRaylib(color));
}

int Display::TextWidth(const std::string& text, int size) const {
    return MeasureText(text.c_str(), size);
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
