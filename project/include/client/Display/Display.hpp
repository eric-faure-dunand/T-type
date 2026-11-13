/*
** EPITECH PROJECT, 2026
** T-type
** File description:
** Display.hpp
*/

#ifndef CLIENT_DISPLAY_HPP
    #define CLIENT_DISPLAY_HPP

    #include <algorithm>
    #include <cstdint>
    #include <map>
    #include <string>
    #include <vector>

namespace r_type::client {

struct Rgba {
    std::uint8_t r = 0;
    std::uint8_t g = 0;
    std::uint8_t b = 0;
    std::uint8_t a = 255;
};

struct Sprite {
    unsigned int id = 0;
    int width = 0;
    int height = 0;
    int mipmaps = 0;
    int format = 0;
};

struct Typeface {
    int baseSize = 0;
    int glyphCount = 0;
    int glyphPadding = 0;
    Sprite texture;
    void *recs = nullptr;
    void *glyphs = nullptr;
};

constexpr int FONT_RESOLUTION = 96;

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
    R,
    W,
    A,
    S,
    D,
    O,
    F
};

class Display {
    int _width;
    int _height;
    std::map<std::string, Sprite> _sprites;
    std::map<std::string, Typeface> _fonts;
    std::string _font;

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
    void FillGradient(float x, float y, float width, float height, Rgba left, Rgba right);
    void Print(const std::string& text, int x, int y, int size, Rgba color, const std::string& font = "");
    int TextWidth(const std::string& text, int size, const std::string& font = "") const;
    bool LoadFont(const std::string& name, const std::string& path);
    void UseFont(const std::string& name);
    bool LoadSprite(const std::string& name, const std::string& path);
    bool HasSprite(const std::string& name) const;
    void DrawSprite(const std::string& name, float x, float y, float width, float height, std::uint8_t alpha = 255,
        bool mirrored = false);

    bool IsKeyDown(Key key) const;
    bool IsKeyPressed(Key key) const;
    std::string TypedText() const;
    float MouseX() const;
    float MouseY() const;
    bool IsMouseClicked() const;
};

}

#endif
