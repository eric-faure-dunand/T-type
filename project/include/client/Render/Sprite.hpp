/*
** EPITECH PROJECT, 2026
** T-type
** File description:
** Sprite.hpp
*/

#ifndef RENDER_SPRITE_HPP
    #define RENDER_SPRITE_HPP

    #include "IRenderer.hpp"

namespace r_type::render {

/**
 * @brief Component: what to draw for an entity (client side only).
 *
 * An entity is drawn when it has a Position (top-left corner) and a Sprite.
 * The server never registers this component.
 */
struct Sprite {
    TextureId texture = kInvalidTexture;
    Rect source{};
    float scale = 1.f;
    int layer = 0;
};

}

#endif