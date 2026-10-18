/*
** EPITECH PROJECT, 2026
** T-type
** File description:
** IKeyboard.hpp
*/

#ifndef INPUT_IKEYBOARD_HPP
    #define INPUT_IKEYBOARD_HPP

namespace r_type::input {

/** @brief Keys the game cares about. Independent from any graphics library. */
enum class Key {
    Up,
    Down,
    Left,
    Right,
    Space,
    Enter,
    Escape
};

/**
 * @brief Read-only access to the keyboard.
 *
 * The game code (InputSystem...) only depends on this interface. The
 * implementation (Raylib today) lives in the render module.
 */
class IKeyboard {
public:
    virtual ~IKeyboard() = default;

    /** @brief True while the key is held down. */
    virtual bool isKeyDown(Key key) const = 0;

    /** @brief True only on the frame the key goes from up to down. */
    virtual bool isKeyPressed(Key key) const = 0;
};

}

#endif