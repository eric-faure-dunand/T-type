/*
** EPITECH PROJECT, 2026
** T-type
** File description:
** IRenderer.hpp
*/

#ifndef RENDER_IRENDERER_HPP
    #define RENDER_IRENDERER_HPP

    #include <cstdint>
    #include <string>

namespace r_type::render {

/** @brief Opaque handle on a texture owned by the renderer. 0 is never valid. */
using TextureId = std::uint32_t;
constexpr TextureId kInvalidTexture = 0;

struct Color {
    std::uint8_t r = 0;
    std::uint8_t g = 0;
    std::uint8_t b = 0;
    std::uint8_t a = 255;
};

/** @brief Rectangle in pixels. */
struct Rect {
    float x = 0.f;
    float y = 0.f;
    float w = 0.f;
    float h = 0.f;
};

struct Size {
    int w = 0;
    int h = 0;
};

/**
 * @brief What the engine needs from a graphics library: a window and sprites.
 *
 * No graphics-library type appears here, so the rest of the code never
 * includes Raylib (or SDL, SFML...). Keyboard input is a separate interface
 * (input::IKeyboard); the Raylib implementation provides both.
 *
 * Typical frame:
 * @code
 * renderer.beginFrame(Color{0, 0, 0});
 * renderSystem(registry, renderer);   // and the background, before it
 * renderer.endFrame();
 * @endcode
 */
class IRenderer {
public:
    virtual ~IRenderer() = default;

    /**
     * @brief Opens the window. Must be called before any texture is loaded.
     * @throws std::runtime_error if the window cannot be created.
     * @throws std::logic_error   if a window is already open.
     */
    virtual void open(int width, int height, const std::string &title) = 0;

    /** @brief Closes the window and frees every texture. Safe to call twice. */
    virtual void close() = 0;

    virtual bool isOpen() const = 0;

    /** @brief True when the user asked to close the window. */
    virtual bool shouldClose() const = 0;

    /** @brief Caps the frame rate (0 = no cap). */
    virtual void setTargetFps(int fps) = 0;

    /** @brief Starts a frame and clears the screen with @p clear. */
    virtual void beginFrame(Color clear) = 0;

    /** @brief Ends the frame and presents it. */
    virtual void endFrame() = 0;

    /**
     * @brief Loads an image file into a texture.
     * @throws std::runtime_error if the file cannot be loaded.
     * @throws std::logic_error   if the window is not open.
     */
    virtual TextureId loadTexture(const std::string &path) = 0;

    /** @brief Frees a texture. Unknown or already freed ids are ignored. */
    virtual void unloadTexture(TextureId texture) = 0;

    /** @throws std::invalid_argument if the texture is unknown. */
    virtual Size textureSize(TextureId texture) const = 0;

    /**
     * @brief Draws a part of a texture with its top-left corner at (x, y).
     *
     * @param source Part of the texture, in pixels. A width or height <= 0
     *               means "the whole texture".
     * @param scale  Size multiplier (1 = original size).
     * @throws std::invalid_argument if the texture is unknown.
     */
    virtual void drawTexture(TextureId texture, Rect source, float x, float y, float scale) = 0;
};

}

#endif