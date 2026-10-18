/*
** EPITECH PROJECT, 2026
** T-type
** File description:
** RaylibBackend.hpp
*/

#ifndef RENDER_RAYLIB_BACKEND_HPP
    #define RENDER_RAYLIB_BACKEND_HPP

    #include <memory>

    #include "client/input/IKeyboard.hpp"

    #include "IRenderer.hpp"

namespace r_type::render {

/**
 * @brief Raylib implementation of IRenderer and IKeyboard.
 *
 * This header does not include raylib.h: Raylib is only used inside
 * src/client/Render/RaylibBackend.cpp. Raylib has a single global window, so
 * only one RaylibBackend can be open at a time (open() throws std::logic_error
 * otherwise). The destructor closes the window.
 *
 * Escape does NOT close the window by default: read Key::Escape from the
 * IKeyboard side and decide in the game.
 */
class RaylibBackend final : public IRenderer, public input::IKeyboard {
public:
    RaylibBackend();
    ~RaylibBackend() override;

    RaylibBackend(const RaylibBackend &) = delete;
    RaylibBackend &operator=(const RaylibBackend &) = delete;

    // IRenderer
    void open(int width, int height, const std::string &title) override;
    void close() override;
    bool isOpen() const override;
    bool shouldClose() const override;
    void setTargetFps(int fps) override;
    void beginFrame(Color clear) override;
    void endFrame() override;
    TextureId loadTexture(const std::string &path) override;
    void unloadTexture(TextureId texture) override;
    Size textureSize(TextureId texture) const override;
    void drawTexture(TextureId texture, Rect source, float x, float y, float scale) override;

    bool isKeyDown(input::Key key) const override;
    bool isKeyPressed(input::Key key) const override;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

}

#endif