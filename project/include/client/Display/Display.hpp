/*
** EPITECH PROJECT, 2026
** T-type
** File description:
** Display.hpp
*/

#ifndef CLIENT_DISPLAY_HPP
    #define CLIENT_DISPLAY_HPP

    #include <cstdint>
    #include <string>

namespace r_type::client {

struct Rgba {
    std::uint8_t r = 0;
    std::uint8_t g = 0;
    std::uint8_t b = 0;
    std::uint8_t a = 255;
};

enum class Key {
    Up,
    Down,
    Left,
    Right,
    Space,
    Enter,
    Escape,
    Backspace,
    C,
    R
};

class Display {
    int _width;
    int _height;

public:
    Display(int width, int height, const std::string& title);
    ~Display();

    Display(const Display&) = delete;
    Display& operator=(const Display&) = delete;

    bool ShouldClose() const;
    float FrameTime() const;
    double Time() const;
    int Width() const {return _width;};
    int Height() const {return _height;};

    void BeginFrame(Rgba clear);
    void EndFrame();
    void FillRect(float x, float y, float width, float height, Rgba color);
    void OutlineRect(float x, float y, float width, float height, float thickness, Rgba color);
    void FillCircle(float x, float y, float radius, Rgba color);
    void Print(const std::string& text, int x, int y, int size, Rgba color);
    int TextWidth(const std::string& text, int size) const;

    bool IsKeyDown(Key key) const;
    bool IsKeyPressed(Key key) const;
    std::string TypedText() const;
    float MouseX() const;
    float MouseY() const;
    bool IsMouseClicked() const;
};

}

#endif
