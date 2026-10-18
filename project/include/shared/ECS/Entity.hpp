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

namespace r_type::ecs {

/**
 * @brief Hard limit on the number of simultaneously alive entities.
 *
 * Every component array is allocated with this capacity when the component
 * type is registered, and never grows afterwards.
 */
constexpr std::size_t kMaxEntities = 5254;

/**
 * @brief An entity is just a 32-bit ID (no data, no methods).
 *
 * Layout: the low 20 bits are the slot index, the high 12 bits are a
 * generation counter. The generation is incremented each time a slot is
 * recycled, so a stale ID (kept after kill()) is detected instead of silently
 * pointing to the new owner of the slot. It wraps after 4096 recycles of the
 * same slot.
 *
 * It is an enum class only to stop an int or an index from being passed as an
 * entity by mistake. Use the helpers below to read it; to send it over the
 * network, send entityValue(e) and rebuild it with entityFromValue(raw).
 */
enum class Entity : std::uint32_t {
    Null = 0xFFFFFFFFu
};

constexpr std::uint32_t kIndexBits = 20;
constexpr std::uint32_t kGenerationBits = 12;
constexpr std::uint32_t kIndexMask = (1u << kIndexBits) - 1u;
constexpr std::uint32_t kGenerationMask = (1u << kGenerationBits) - 1u;

constexpr Entity makeEntity(std::uint32_t index, std::uint32_t generation) noexcept
{
    return static_cast<Entity>(((generation & kGenerationMask) << kIndexBits) | (index & kIndexMask));
}

constexpr std::uint32_t entityIndex(Entity e) noexcept
{
    return static_cast<std::uint32_t>(e) & kIndexMask;
}

constexpr std::uint32_t entityGeneration(Entity e) noexcept
{
    return static_cast<std::uint32_t>(e) >> kIndexBits;
}

constexpr std::uint32_t entityValue(Entity e) noexcept
{
    return static_cast<std::uint32_t>(e);
}

constexpr Entity entityFromValue(std::uint32_t raw) noexcept
{
    return static_cast<Entity>(raw);
}

static_assert(kMaxEntities < kIndexMask, "kMaxEntities does not fit in the index bits");
static_assert(sizeof(Entity) == sizeof(std::uint32_t), "Entity must stay a 32-bit value");

}

#endif