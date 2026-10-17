/*
** EPITECH PROJECT, 2026
** T-type
** File description:
** FixedTimestep.hpp
*/

#ifndef LOOP_FIXED_TIMESTEP_HPP
    #define LOOP_FIXED_TIMESTEP_HPP

    #include <chrono>
    #include <cstddef>
    #include <cstdint>
    #include <stdexcept>
    #include <utility>

namespace r_type::loop {

/**
 * @brief Fixed-timestep accumulator: the simulation always advances by the
 *        same dt, whatever the display framerate is.
 *
 * Usage (see docs/game-loop.md):
 * @code
 * FixedTimestep loop(60);
 * auto previous = std::chrono::steady_clock::now();
 * while (running) {
 *     auto now = std::chrono::steady_clock::now();
 *     loop.advance(now - previous, [&](float dt) { systems.run(registry, dt); });
 *     previous = now;
 *     render(registry, loop.alpha());   // client only
 * }
 * @endcode
 *
 * The class never reads a clock: the caller gives it the real elapsed time.
 * That keeps it independent of the platform and fully testable with a fake
 * clock.
 *
 * Why it is deterministic: the accumulator is an integer (nanoseconds x Hz),
 * a step is consumed each time it reaches 1'000'000'000, and every step calls
 * the simulation with the exact same dt. The number of steps executed after a
 * given total elapsed time therefore only depends on that total, never on how
 * it was cut into frames (30 FPS, 144 FPS, jitter...). No floating-point
 * error accumulates in the time bookkeeping.
 *
 * Long stalls: a single frame is clamped to maxFrame (250 ms by default). The
 * time above that is dropped (the game slows down instead of freezing while it
 * catches up, the so-called "spiral of death").
 *
 * Not thread-safe.
 */
class FixedTimestep {
public:
    using Duration = std::chrono::nanoseconds;

    /**
     * @param hz       Simulation frequency, in steps per second (1..1000).
     * @param maxFrame Longest frame time taken into account (must be > 0).
     * @throws std::invalid_argument on invalid arguments.
     */
    explicit FixedTimestep(unsigned hz = 60, Duration maxFrame = std::chrono::milliseconds(250))
        : hz_(hz), maxFrameNs_(maxFrame.count())
    {
        if (hz == 0 || hz > 1000)
            throw std::invalid_argument("FixedTimestep: hz must be between 1 and 1000");
        if (maxFrameNs_ <= 0)
            throw std::invalid_argument("FixedTimestep: maxFrame must be positive");
        stepSeconds_ = static_cast<float>(1.0 / static_cast<double>(hz));
    }

    /**
     * @brief Feeds the real elapsed time and runs every simulation step now due.
     *
     * @param frame Real time elapsed since the previous call (negative = 0).
     * @param step  Callable void(float dt). Called once per due step with the
     *              constant dt (stepSeconds()). Never called from render code.
     * @return The number of steps executed by this call.
     *
     * If @p step throws, the exception propagates; the step is counted as
     * consumed (time and tick()) and the object stays usable.
     */
    template <class F>
    std::size_t advance(Duration frame, F &&step)
    {
        std::int64_t ns = frame.count();
        if (ns < 0)
            ns = 0;
        if (ns > maxFrameNs_)
            ns = maxFrameNs_;
        accumulator_ += ns * static_cast<std::int64_t>(hz_);

        std::size_t steps = 0;
        while (accumulator_ >= kUnit) {
            accumulator_ -= kUnit;
            ++tick_;
            ++steps;
            step(stepSeconds_);
        }
        return steps;
    }

    /** @brief The constant dt, in seconds, given to every step. */
    float stepSeconds() const noexcept { return stepSeconds_; }

    unsigned hz() const noexcept { return hz_; }

    /** @brief Total number of simulation steps executed since construction. */
    std::uint64_t tick() const noexcept { return tick_; }

    /**
     * @brief Fraction of a step left in the accumulator, in [0, 1).
     *
     * The render can use it to interpolate between the previous and the
     * current simulation state, so that motion stays smooth at 144 FPS even
     * though the simulation runs at 60 Hz.
     */
    double alpha() const noexcept
    {
        return static_cast<double>(accumulator_) / static_cast<double>(kUnit);
    }

    /**
     * @brief Real time to wait before the next step is due (always > 0).
     *
     * For a loop without rendering (the server): sleep this long instead of
     * busy-waiting. Advancing by exactly this duration runs exactly one step.
     */
    Duration timeUntilNextStep() const noexcept
    {
        const std::int64_t missing = kUnit - accumulator_;
        const std::int64_t hz = static_cast<std::int64_t>(hz_);
        return Duration((missing + hz - 1) / hz);
    }

private:
    // One step is consumed when the accumulator (ns x Hz) reaches 1e9,
    // i.e. when (1e9 / hz) nanoseconds have been accumulated.
    static constexpr std::int64_t kUnit = 1'000'000'000;

    unsigned hz_;
    std::int64_t maxFrameNs_;
    float stepSeconds_ = 0.f;
    std::int64_t accumulator_ = 0;
    std::uint64_t tick_ = 0;
};

}

#endif