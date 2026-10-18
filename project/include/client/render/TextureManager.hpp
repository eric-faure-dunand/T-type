/*
** EPITECH PROJECT, 2026
** T-type
** File description:
** TextureManager.hpp
*/

#ifndef RENDER_TEXTURE_MANAGER_HPP
    #define RENDER_TEXTURE_MANAGER_HPP

    #include <cstddef>
    #include <string>
    #include <unordered_map>

    #include "IRenderer.hpp"

namespace r_type::render {

/**
 * @brief Loads each texture file only once.
 *
 * get("assets/sprites/ship.png") loads the file the first time and returns
 * the same TextureId afterwards. The manager must not outlive the renderer.
 */
class TextureManager {
public:
    explicit TextureManager(IRenderer &renderer);

    /**
     * @brief Texture of @p path, loaded on first use.
     * @throws whatever IRenderer::loadTexture throws (nothing is cached then).
     */
    TextureId get(const std::string &path);

    /** @brief Number of textures currently loaded. */
    std::size_t size() const noexcept;

    /** @brief Unloads every texture. Ids returned before become invalid. */
    void clear();

private:
    IRenderer &renderer_;
    std::unordered_map<std::string, TextureId> cache_;
};

}

#endif