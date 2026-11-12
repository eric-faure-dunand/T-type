/*
** EPITECH PROJECT, 2026
** T-type
** File description:
** PatternSystem.cpp
*/

#include "PatternSystem.hpp"

namespace r_type::game {

static std::optional<Position> closestPlayer(ecs::Registry &r, const Position &from)
{
    std::optional<Position> closest;
    float best = 0.f;
    r.each<Player, Position>([&](Player &, const Position &pos) {
        const float dx = pos.x - from.x;
        const float dy = pos.y - from.y;
        const float distance = dx * dx + dy * dy;
        if (!closest || distance < best) {
            closest = pos;
            best = distance;
        }
    });
    return closest;
}

static void zigzag(Pattern &pattern, const Position &pos, Velocity &vel)
{
    if (pos.y - pattern.baseY > kZigzagRange)
        pattern.direction = -1.f;
    else if (pos.y - pattern.baseY < -kZigzagRange)
        pattern.direction = 1.f;
    vel = {-pattern.speed, pattern.direction * kZigzagSpeed};
}

static void bounce(Pattern &pattern, const Position &pos, const Hitbox &box, Velocity &vel)
{
    if (pos.y <= 0.f)
        pattern.direction = 1.f;
    else if (pos.y + box.height >= kWorldHeight)
        pattern.direction = -1.f;
    vel = {-pattern.speed, pattern.direction * kBounceSpeed};
}

static void dive(ecs::Registry &r, const Pattern &pattern, const Position &pos, Velocity &vel)
{
    vel = {-pattern.speed * kDiveBoost, 0.f};
    const std::optional<Position> target = closestPlayer(r, pos);
    if (!target || target->x >= pos.x)
        return;
    if (target->y > pos.y + 1.f)
        vel.y = kDiveSpeed;
    else if (target->y < pos.y - 1.f)
        vel.y = -kDiveSpeed;
}

static void boss(Pattern &pattern, const Position &pos, const Hitbox &box, Velocity &vel)
{
    if (pos.x > kWorldWidth - box.width - kBossMargin) {
        vel = {-pattern.speed, 0.f};
        return;
    }
    if (pos.y <= kBossMargin)
        pattern.direction = 1.f;
    else if (pos.y + box.height >= kWorldHeight - kBossMargin)
        pattern.direction = -1.f;
    vel = {0.f, pattern.direction * kBossVerticalSpeed};
}

void patternSystem(ecs::Registry &r, float dt)
{
    r.each<Pattern, Position, Velocity, Hitbox>([&](Pattern &pattern, const Position &pos, Velocity &vel,
                                                    const Hitbox &box) {
        pattern.age += dt;
        switch (pattern.type) {
            case PatternType::Straight:
                vel = {-pattern.speed, 0.f};
                break;
            case PatternType::Sine:
                vel = {-pattern.speed,
                    kSineAmplitude * kSineFrequency * std::cos(pattern.age * kSineFrequency)};
                break;
            case PatternType::Zigzag:
                zigzag(pattern, pos, vel);
                break;
            case PatternType::Bounce:
                bounce(pattern, pos, box, vel);
                break;
            case PatternType::Dive:
                dive(r, pattern, pos, vel);
                break;
            case PatternType::Wall: {
                const bool waiting = pos.x <= kWorldWidth * kWallStop && pattern.age < kWallWait;
                vel = {waiting ? 0.f : -pattern.speed, 0.f};
                break;
            }
            case PatternType::Formation: {
                const bool fast = std::sin(pattern.age * kFormationFrequency) > 0.f;
                vel = {-pattern.speed * (fast ? kFormationFast : kFormationSlow), 0.f};
                break;
            }
            case PatternType::Boss:
                boss(pattern, pos, box, vel);
                break;
        }
    });
}

}
