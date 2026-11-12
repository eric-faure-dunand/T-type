/*
** EPITECH PROJECT, 2026
** T-type
** File description:
** test_replay_window.cpp
*/

#include "Network/ReplayWindow.hpp"
#include <cstdint>
#include <gtest/gtest.h>
#include <limits>

using namespace r_type;

TEST(ReplayWindowTest, AcceptFirstPacket)
{
    ReplayWindow window;

    EXPECT_TRUE(window.accepts(1));
    EXPECT_TRUE(window.record(1));
    EXPECT_FALSE(window.accepts(1));
}

TEST(ReplayWindowTest, RejectDuplicatePacket)
{
    ReplayWindow window;

    ASSERT_TRUE(window.record(10));
    EXPECT_FALSE(window.record(10));
}

TEST(ReplayWindowTest, AcceptOutOfOrderPackets)
{
    ReplayWindow window;

    ASSERT_TRUE(window.record(10));
    ASSERT_TRUE(window.record(12));
    EXPECT_TRUE(window.record(11));
    EXPECT_FALSE(window.record(11));
}

TEST(ReplayWindowTest, RejectOldPackets)
{
    ReplayWindow window;

    ASSERT_TRUE(window.record(1));
    ASSERT_TRUE(window.record(100));
    EXPECT_FALSE(window.accepts(1));
    EXPECT_FALSE(window.record(1));
}

TEST(ReplayWindowTest, AcceptPacketAtWindowBoundary)
{
    ReplayWindow window;

    ASSERT_TRUE(window.record(100));
    EXPECT_TRUE(window.record(37));
    EXPECT_FALSE(window.record(36));
}

TEST(ReplayWindowTest, RejectZeroSequence)
{
    ReplayWindow window;

    EXPECT_FALSE(window.accepts(0));
    EXPECT_FALSE(window.record(0));
}

TEST(ReplayWindowTest, ResetWindow)
{
    ReplayWindow window;

    ASSERT_TRUE(window.record(10));
    ASSERT_TRUE(window.record(12));

    window.reset();

    EXPECT_TRUE(window.accepts(10));
    EXPECT_TRUE(window.record(10));
}

TEST(ReplayWindowTest, AcceptLargeSequenceJump)
{
    ReplayWindow window;

    ASSERT_TRUE(window.record(1));
    ASSERT_TRUE(window.record(1000));

    EXPECT_FALSE(window.record(1));
    EXPECT_TRUE(window.record(999));
    EXPECT_FALSE(window.record(999));
}

TEST(ReplayWindowTest, RejectDuplicateAfterOutOfOrderPackets)
{
    ReplayWindow window;

    ASSERT_TRUE(window.record(20));
    ASSERT_TRUE(window.record(22));
    ASSERT_TRUE(window.record(21));
    ASSERT_TRUE(window.record(19));

    EXPECT_FALSE(window.record(20));
    EXPECT_FALSE(window.record(21));
    EXPECT_FALSE(window.record(22));
    EXPECT_FALSE(window.record(19));
}

TEST(ReplayWindowTest, AcceptMaximumSequence)
{
    ReplayWindow window;
    const auto maximum = std::numeric_limits<std::uint64_t>::max();

    EXPECT_TRUE(window.record(maximum));
    EXPECT_FALSE(window.record(maximum));
    EXPECT_TRUE(window.record(maximum - 1));
}
