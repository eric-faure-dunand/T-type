/*
** EPITECH PROJECT, 2026
** T-type
** File description:
** test_entity.cpp
*/

#include <gtest/gtest.h>

#include <set>
#include <stdexcept>
#include <unordered_set>

#include "shared/ECS/Registry.hpp"

using namespace r_type::ecs;

// ---------------------------------------------------------------- Entity ID

TEST(Entity, PacksIndexAndGeneration)
{
    Entity e = makeEntity(1234, 56);
    EXPECT_EQ(entityIndex(e), 1234u);
    EXPECT_EQ(entityGeneration(e), 56u);
    EXPECT_NE(e, Entity::Null);
}

TEST(Entity, GenerationIsMaskedTo12Bits)
{
    EXPECT_EQ(entityGeneration(makeEntity(1, 4096 + 7)), 7u);
}

TEST(Entity, RawValueRoundTrip)
{
    Entity e = makeEntity(5253, 4094);
    EXPECT_EQ(entityFromValue(entityValue(e)), e);
}

TEST(Entity, NullIsNeverAMaxEntitiesSlot)
{
    EXPECT_GE(entityIndex(Entity::Null), kMaxEntities);
}

TEST(Entity, IsA32BitValue)
{
    EXPECT_EQ(sizeof(Entity), 4u);
}

TEST(Entity, HashableInContainers)
{
    std::unordered_set<Entity> set{makeEntity(1, 0), makeEntity(2, 0), makeEntity(1, 0)};
    EXPECT_EQ(set.size(), 2u);
}

// ---------------------------------------------------------------- Lifecycle

TEST(Lifecycle, SpawnReturnsDistinctAliveEntities)
{
    Registry reg;
    std::set<std::uint32_t> seen;
    for (int i = 0; i < 100; ++i) {
        Entity e = reg.spawn();
        EXPECT_TRUE(reg.alive(e));
        EXPECT_TRUE(seen.insert(entityValue(e)).second);
    }
    EXPECT_EQ(reg.aliveCount(), 100u);
}

TEST(Lifecycle, KillMakesEntityDead)
{
    Registry reg;
    Entity e = reg.spawn();
    reg.kill(e);
    EXPECT_FALSE(reg.alive(e));
    EXPECT_EQ(reg.aliveCount(), 0u);
}

TEST(Lifecycle, KillThenSpawnRecyclesIndexWithNextGeneration)
{
    Registry reg;
    Entity a = reg.spawn();
    reg.kill(a);
    Entity b = reg.spawn();
    EXPECT_EQ(entityIndex(b), entityIndex(a));
    EXPECT_EQ(entityGeneration(b), entityGeneration(a) + 1);
    EXPECT_NE(a, b);
}

TEST(Lifecycle, RecyclingIsLifo)
{
    Registry reg;
    Entity a = reg.spawn();
    Entity b = reg.spawn();
    reg.kill(a);
    reg.kill(b);
    EXPECT_EQ(entityIndex(reg.spawn()), entityIndex(b));
    EXPECT_EQ(entityIndex(reg.spawn()), entityIndex(a));
}

TEST(Lifecycle, StaleHandleIsRejectedAfterRecycling)
{
    Registry reg;
    Entity old = reg.spawn();
    reg.kill(old);
    Entity fresh = reg.spawn();
    EXPECT_FALSE(reg.alive(old));
    EXPECT_TRUE(reg.alive(fresh));
}

TEST(Lifecycle, KillOnStaleHandleThrows)
{
    Registry reg;
    Entity old = reg.spawn();
    reg.kill(old);
    reg.spawn();
    EXPECT_THROW(reg.kill(old), std::invalid_argument);
    EXPECT_EQ(reg.aliveCount(), 1u);
}

TEST(Lifecycle, KillTwiceThrows)
{
    Registry reg;
    Entity e = reg.spawn();
    reg.kill(e);
    EXPECT_THROW(reg.kill(e), std::invalid_argument);
}

TEST(Lifecycle, KillNullThrows)
{
    Registry reg;
    EXPECT_THROW(reg.kill(Entity::Null), std::invalid_argument);
    EXPECT_FALSE(reg.alive(Entity::Null));
}

TEST(Lifecycle, NeverSpawnedHandleIsNotAlive)
{
    Registry reg;
    EXPECT_FALSE(reg.alive(makeEntity(42, 0)));
    EXPECT_FALSE(reg.alive(makeEntity(static_cast<std::uint32_t>(kMaxEntities + 10), 0)));
    EXPECT_THROW(reg.kill(makeEntity(42, 0)), std::invalid_argument);
}

TEST(Lifecycle, SpawnBeyondMaxEntitiesThrows)
{
    Registry reg;
    for (std::size_t i = 0; i < kMaxEntities; ++i)
        reg.spawn();
    EXPECT_EQ(reg.aliveCount(), kMaxEntities);
    EXPECT_THROW(reg.spawn(), std::length_error);
    EXPECT_EQ(reg.aliveCount(), kMaxEntities);
}

TEST(Lifecycle, SlotAvailableAgainAfterKillWhenFull)
{
    Registry reg;
    Entity last = Entity::Null;
    for (std::size_t i = 0; i < kMaxEntities; ++i)
        last = reg.spawn();
    reg.kill(last);
    EXPECT_NO_THROW(reg.spawn());
    EXPECT_THROW(reg.spawn(), std::length_error);
}

TEST(Lifecycle, ManySpawnKillCyclesDoNotLeakOrOverflow)
{
    Registry reg;
    for (int i = 0; i < 100000; ++i) {
        Entity e = reg.spawn();
        ASSERT_TRUE(reg.alive(e));
        reg.kill(e);
    }
    EXPECT_EQ(reg.aliveCount(), 0u);
}

TEST(Lifecycle, ManyEntitiesKilledInBatchesThenRespawned)
{
    Registry reg;
    std::vector<Entity> batch;
    for (int round = 0; round < 20; ++round) {
        for (int i = 0; i < 1000; ++i)
            batch.push_back(reg.spawn());
        for (Entity e : batch)
            reg.kill(e);
        batch.clear();
    }
    EXPECT_EQ(reg.aliveCount(), 0u);
}

TEST(Lifecycle, GenerationWrapsAfter4096Recycles)
{
    // Documented limitation: the 12-bit generation wraps, so a handle kept across
    // exactly 4096 recycles of its slot is considered alive again.
    Registry reg;
    Entity first = reg.spawn();
    Entity current = first;
    for (int i = 0; i < 4096; ++i) {
        reg.kill(current);
        current = reg.spawn();
        ASSERT_EQ(entityIndex(current), entityIndex(first));
    }
    EXPECT_EQ(entityGeneration(current), entityGeneration(first));
    EXPECT_TRUE(reg.alive(first));
}

TEST(Lifecycle, GenerationBelowWrapStaysDistinct)
{
    Registry reg;
    Entity first = reg.spawn();
    Entity current = first;
    for (int i = 0; i < 4095; ++i) {
        reg.kill(current);
        current = reg.spawn();
    }
    EXPECT_NE(entityGeneration(current), entityGeneration(first));
    EXPECT_FALSE(reg.alive(first));
}