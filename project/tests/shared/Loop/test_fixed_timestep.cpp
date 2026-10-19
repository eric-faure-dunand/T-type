/*
** EPITECH PROJECT, 2026
** T-type
** File description:
** test_fixed_timestep.cpp
*/

#include <gtest/gtest.h>

#include <chrono>
#include <cmath>
#include <cstdint>
#include <random>
#include <stdexcept>
#include <vector>

#include "include/shared/ECS/Registry.hpp"
#include "shared/Loop/FixedTimestep.hpp"

using namespace r_type::loop;
using Duration = FixedTimestep::Duration;
using std::chrono::milliseconds;
using std::chrono::seconds;

namespace {

struct Position {
    float x = 0.f;
    float y = 0.f;
};

struct Velocity {
    float dx = 0.f;
    float dy = 0.f;
};

FixedTimestep unclamped(unsigned hz = 60)
{
    return FixedTimestep(hz, seconds(1000));
}

std::vector<Duration> regularFrames(int fps, int totalSeconds)
{
    std::vector<Duration> frames;
    std::int64_t previous = 0;
    for (std::int64_t k = 1; k <= static_cast<std::int64_t>(fps) * totalSeconds; ++k) {
        const auto ts = static_cast<std::int64_t>(std::llround(static_cast<double>(k) * 1e9 / fps));
        frames.push_back(Duration(ts - previous));
        previous = ts;
    }
    return frames;
}

std::vector<Duration> jitteryFrames(std::uint32_t seed, int totalSeconds)
{
    std::mt19937 rng(seed);
    std::vector<Duration> frames;
    std::int64_t remaining = static_cast<std::int64_t>(totalSeconds) * 1'000'000'000;
    while (remaining > 0) {
        std::int64_t len = 1'000'000 + static_cast<std::int64_t>(rng() % 40'000'000u);
        if (len > remaining)
            len = remaining;
        frames.push_back(Duration(len));
        remaining -= len;
    }
    return frames;
}

struct Session {
    std::uint64_t ticks = 0;
    std::size_t renders = 0;
    std::vector<float> trajectory;
    float finalX = 0.f;
    float finalY = 0.f;
    float bystanderX = 0.f;
};

Session play(const std::vector<Duration> &frames)
{
    using namespace r_type::ecs;
    Registry reg;
    reg.registerComponent<Position>();
    reg.registerComponent<Velocity>();

    Entity moving = reg.spawn();
    reg.add<Position>(moving, 0.f, 0.f);
    reg.add<Velocity>(moving, 120.f, -30.f);
    Entity bystander = reg.spawn();
    reg.add<Position>(bystander, 5.f, 5.f);

    FixedTimestep loop = unclamped(60);
    Session s;
    for (Duration frame : frames) {
        loop.advance(frame, [&](float dt) {
            reg.each<Position, Velocity>([dt](Position &p, Velocity &v) {
                p.x += v.dx * dt;
                p.y += v.dy * dt;
                v.dx *= 0.999f;
            });
            s.trajectory.push_back(reg.get<Position>(moving).x);
        });
        ++s.renders;
    }
    s.ticks = loop.tick();
    s.finalX = reg.get<Position>(moving).x;
    s.finalY = reg.get<Position>(moving).y;
    s.bystanderX = reg.get<Position>(bystander).x;
    return s;
}

}

TEST(LoopDeterminism, SimulationIsIdenticalAt30And144Fps)
{
    const Session at30 = play(regularFrames(30, 10));
    const Session at144 = play(regularFrames(144, 10));

    EXPECT_EQ(at30.ticks, 600u);
    EXPECT_EQ(at144.ticks, 600u);
    ASSERT_EQ(at30.trajectory.size(), at144.trajectory.size());
    EXPECT_EQ(at30.trajectory, at144.trajectory);
    EXPECT_EQ(at30.finalX, at144.finalX);
    EXPECT_EQ(at30.finalY, at144.finalY);
    EXPECT_FLOAT_EQ(at30.bystanderX, 5.f);
}

TEST(LoopDeterminism, SimulationAdvancesByConstantStepsWhateverTheDisplay)
{
    const Session at30 = play(regularFrames(30, 10));
    const Session at144 = play(regularFrames(144, 10));
    EXPECT_EQ(at30.renders, 300u);
    EXPECT_EQ(at144.renders, 1440u);
    EXPECT_EQ(at30.ticks, at144.ticks);
}

TEST(LoopDeterminism, SameResultAtManyFramerates)
{
    const Session reference = play(regularFrames(60, 10));
    for (int fps : {24, 30, 75, 120, 144, 165, 240, 360}) {
        const Session s = play(regularFrames(fps, 10));
        EXPECT_EQ(s.trajectory, reference.trajectory) << "fps=" << fps;
    }
}

TEST(LoopDeterminism, SameResultWithIrregularFrameTimes)
{
    const Session reference = play(regularFrames(30, 10));
    for (std::uint32_t seed : {1u, 2u, 3u, 42u, 1234u}) {
        const Session s = play(jitteryFrames(seed, 10));
        EXPECT_EQ(s.trajectory, reference.trajectory) << "seed=" << seed;
    }
}

TEST(LoopDeterminism, StepCountDependsOnlyOnTotalTime)
{
    for (int seconds_ : {1, 3, 7}) {
        const std::uint64_t expected = static_cast<std::uint64_t>(seconds_) * 60u;
        EXPECT_EQ(play(regularFrames(30, seconds_)).ticks, expected);
        EXPECT_EQ(play(regularFrames(144, seconds_)).ticks, expected);
        EXPECT_EQ(play(jitteryFrames(7, seconds_)).ticks, expected);
    }
}


TEST(FixedTimestep, EveryStepReceivesTheSameDt)
{
    FixedTimestep loop = unclamped(60);
    std::vector<float> dts;
    loop.advance(seconds(1), [&](float dt) { dts.push_back(dt); });
    ASSERT_EQ(dts.size(), 60u);
    for (float dt : dts)
        EXPECT_EQ(dt, loop.stepSeconds());
    EXPECT_NEAR(loop.stepSeconds(), 1.0f / 60.0f, 1e-9f);
}

TEST(FixedTimestep, OneSecondGivesExactlyHzSteps)
{
    for (unsigned hz : {1u, 20u, 30u, 60u, 120u, 144u, 1000u}) {
        FixedTimestep loop = unclamped(hz);
        EXPECT_EQ(loop.advance(seconds(1), [](float) {}), hz) << "hz=" << hz;
        EXPECT_EQ(loop.tick(), hz);
    }
}

TEST(FixedTimestep, NoStepUntilEnoughTimeHasAccumulated)
{
    FixedTimestep loop;
    EXPECT_EQ(loop.advance(milliseconds(10), [](float) {}), 0u);
    EXPECT_NEAR(loop.alpha(), 0.6, 1e-9);
    EXPECT_EQ(loop.advance(milliseconds(10), [](float) {}), 1u);
    EXPECT_NEAR(loop.alpha(), 0.2, 1e-9);
    EXPECT_EQ(loop.tick(), 1u);
}

TEST(FixedTimestep, LeftoverTimeIsKeptNotLost)
{
    // 1000 frames of 1 ms = exactly 1 s: no drift, exactly 60 steps.
    FixedTimestep loop;
    std::size_t steps = 0;
    for (int i = 0; i < 1000; ++i)
        steps += loop.advance(milliseconds(1), [](float) {});
    EXPECT_EQ(steps, 60u);
    EXPECT_NEAR(loop.alpha(), 0.0, 1e-9);
}

TEST(FixedTimestep, SeveralStepsInOneFrameWhenBehind)
{
    FixedTimestep loop; // 60 Hz
    EXPECT_EQ(loop.advance(milliseconds(100), [](float) {}), 6u); // 100 ms = 6.0 steps (+ remainder)
    EXPECT_LT(loop.alpha(), 1.0);
}

TEST(FixedTimestep, TickCountsEveryStep)
{
    FixedTimestep loop = unclamped();
    std::uint64_t seenInStep = 0;
    loop.advance(seconds(1), [&](float) { seenInStep = loop.tick(); });
    EXPECT_EQ(seenInStep, 60u); // tick() already includes the running step
    EXPECT_EQ(loop.tick(), 60u);
}

TEST(FixedTimestep, AlphaStaysInZeroOneWhateverTheFrames)
{
    FixedTimestep loop;
    std::mt19937 rng(99);
    for (int i = 0; i < 10000; ++i) {
        loop.advance(Duration(static_cast<std::int64_t>(rng() % 60'000'000u)), [](float) {});
        ASSERT_GE(loop.alpha(), 0.0);
        ASSERT_LT(loop.alpha(), 1.0);
    }
}

// ------------------------------------------------ robustness

TEST(FixedTimestep, LongStallIsClampedToMaxFrame)
{
    FixedTimestep loop; // default maxFrame = 250 ms => at most 15 steps
    EXPECT_EQ(loop.advance(seconds(10), [](float) {}), 15u);
    EXPECT_LT(loop.alpha(), 1.0);
    // and the loop is healthy afterwards
    EXPECT_EQ(loop.advance(milliseconds(17), [](float) {}), 1u);
}

TEST(FixedTimestep, CustomMaxFrame)
{
    FixedTimestep loop(60, milliseconds(100));
    EXPECT_EQ(loop.advance(seconds(5), [](float) {}), 6u);
}

TEST(FixedTimestep, NegativeOrZeroElapsedTimeDoesNothing)
{
    FixedTimestep loop;
    EXPECT_EQ(loop.advance(Duration(-5'000'000), [](float) {}), 0u);
    EXPECT_EQ(loop.advance(Duration(0), [](float) {}), 0u);
    EXPECT_EQ(loop.tick(), 0u);
    EXPECT_NEAR(loop.alpha(), 0.0, 1e-12);
}

TEST(FixedTimestep, InvalidArgumentsThrow)
{
    EXPECT_THROW(FixedTimestep(0), std::invalid_argument);
    EXPECT_THROW(FixedTimestep(1001), std::invalid_argument);
    EXPECT_THROW(FixedTimestep(60, Duration(0)), std::invalid_argument);
    EXPECT_THROW(FixedTimestep(60, Duration(-1)), std::invalid_argument);
}

TEST(FixedTimestep, ExceptionInAStepPropagatesAndLoopStaysConsistent)
{
    FixedTimestep loop = unclamped();
    EXPECT_THROW(loop.advance(seconds(1), [](float) { throw std::runtime_error("boom"); }),
        std::runtime_error);
    EXPECT_EQ(loop.tick(), 1u); // the failed step was consumed

    // the 59 remaining steps are still due, nothing is lost or duplicated
    EXPECT_EQ(loop.advance(Duration(0), [](float) {}), 59u);
    EXPECT_EQ(loop.tick(), 60u);
}

// ------------------------------------------------ server-side waiting

TEST(FixedTimestep, TimeUntilNextStepOnFreshLoopIsOneStep)
{
    FixedTimestep loop;
    EXPECT_EQ(loop.timeUntilNextStep().count(), 16'666'667); // 1e9/60, rounded up
}

TEST(FixedTimestep, AdvancingByTimeUntilNextStepRunsExactlyOneStep)
{
    FixedTimestep loop;
    std::mt19937 rng(5);
    for (int i = 0; i < 500; ++i) {
        loop.advance(Duration(static_cast<std::int64_t>(rng() % 30'000'000u)), [](float) {});
        const Duration wait = loop.timeUntilNextStep();
        ASSERT_GT(wait.count(), 0);
        FixedTimestep copy = loop;
        ASSERT_EQ(copy.advance(wait - Duration(1), [](float) {}), 0u); // 1 ns too early: nothing
        ASSERT_EQ(loop.advance(wait, [](float) {}), 1u);               // exactly on time: one step
    }
}