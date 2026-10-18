/*
** EPITECH PROJECT, 2026
** T-type
** File description:
** RenderSystem.hpp
*/

#ifndef RENDER_RENDER_SYSTEM_HPP
    #define RENDER_RENDER_SYSTEM_HPP

    #include "shared/ECS/Registry.hpp"

    #include "IRenderer.hpp"

namespace r_type::render {

/**
 * @brief Draws every entity that has a Position and a Sprite.
 *
 * Called once per display frame, between beginFrame() and endFrame(), and NOT
 * from the fixed-step simulation (see docs/game-loop.md). Entities are drawn
 * by ascending Sprite::layer, then by ascending entity index. Sprites whose
 * texture is invalid are skipped. The registry is not modified.
 *
 * @throws std::out_of_range if Position or Sprite is not registered.
 */
void renderSystem(ecs::Registry &registry, IRenderer &renderer);

}

#endif