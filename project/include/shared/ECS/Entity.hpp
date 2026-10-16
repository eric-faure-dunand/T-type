/*
** EPITECH PROJECT, 2026
** T-type
** File description:
** Entity.hpp
*/

#ifndef ECS_ENTITY_HPP
    #define ECS_ENTITY_HPP

    #include <cstddef>
    #include <cstdint>
    #include <functional>

namespace r_type::ecs {

/**
 * @brief Hard limit on the number of simultaneously alive entities.
 *
 * Every component array is allocated with this capacity when the component
 * type is registered, and never grows afterwards.
 */
constexpr std::size_t kMaxEntities = 5254;

/**
 * @brief Lightweight handle on an entity (index + generation).
 *
 * The handle is a single 32-bit value: the low 20 bits hold the slot index,
 * the high 12 bits hold a generation counter. The generation is incremented
 * each time a slot is recycled, so a stale handle (kept after kill()) is
 * detected instead of silently pointing to the new owner of the slot.
 *
 * The generation wraps after 4096 recycles of the same slot: a handle kept
 * across exactly 4096 kill/spawn cycles of its slot would appear alive again.
 *
 * The handle is a plain 32-bit integer on purpose: it can be sent over the
 * network as-is (see value() / fromValue()).
 */
class Entity {
public:
    static constexpr std::uint32_t kIndexBits = 20;
    static constexpr std::uint32_t kGenerationBits = 12;
    static constexpr std::uint32_t kIndexMask = (1u << kIndexBits) - 1u;
    static constexpr std::uint32_t kGenerationMask = (1u << kGenerationBits) - 1u;

    /** @brief Builds the null handle (never alive). */
    constexpr Entity() noexcept = default;

    /** @brief Builds a handle from a slot index and a generation. */
    constexpr Entity(std::uint32_t index, std::uint32_t generation) noexcept
        : value_((generation & kGenerationMask) << kIndexBits | (index & kIndexMask)) {}

    /** @brief The null handle. It is never alive. */
    static constexpr Entity null() noexcept { return Entity(); }

    /** @brief Rebuilds a handle from its raw 32-bit value (e.g. read from a packet). */
    static constexpr Entity fromValue(std::uint32_t raw) noexcept
    {
        Entity e;
        e.value_ = raw;
        return e;
    }

    constexpr std::uint32_t index() const noexcept { return value_ & kIndexMask; }
    constexpr std::uint32_t generation() const noexcept { return value_ >> kIndexBits; }
    constexpr std::uint32_t value() const noexcept { return value_; }
    constexpr bool isNull() const noexcept { return value_ == kNullValue; }

    constexpr bool operator==(const Entity &) const noexcept = default;

private:
    static constexpr std::uint32_t kNullValue = 0xFFFFFFFFu;

    std::uint32_t value_ = kNullValue;
};

static_assert(kMaxEntities < Entity::kIndexMask, "kMaxEntities does not fit in the index bits");
static_assert(sizeof(Entity) == sizeof(std::uint32_t), "Entity must stay a 32-bit handle");

}

namespace std {

template <>
struct hash<r_type::ecs::Entity> {
    std::size_t operator()(const r_type::ecs::Entity &e) const noexcept
    {
        return std::hash<std::uint32_t>{}(e.value());
    }
};

}

#endif
