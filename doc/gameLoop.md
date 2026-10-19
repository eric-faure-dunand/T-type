# Game loop: fixed timestep

Files: `project/include/shared/Loop/FixedTimestep.hpp`, tests in `project/tests/shared/Loop/`.

## The problem

If a system moves an entity by `speed * frameTime`, the result depends on the machine:
a 30 FPS client and a 144 FPS client do not compute the same positions, the server and
the client drift apart, and bugs (tunnelling through a hitbox, a cooldown that ends one
frame too late) cannot be reproduced.

## The solution: simulate in fixed steps

The simulation always advances by the **same** `dt` (e.g. 1/60 s). The display runs as
fast as it wants, and the *accumulator* converts the real elapsed time into a number of
simulation steps.

```
 real time ──► [ accumulator ] ──► 0, 1 or n steps of exactly dt ──► systems
                     │
                     └── leftover (alpha, in [0,1)) ──► render interpolation
```

| Display | Frame | What happens per frame (60 Hz simulation) |
|---------|-------|-------------------------------------------|
| 30 FPS  | 33.3 ms | 2 steps |
| 60 FPS  | 16.7 ms | 1 step |
| 144 FPS | 6.9 ms  | 0 or 1 step (about 4 frames out of 10 run a step) |

After 10 seconds all three have executed exactly 600 steps, with the same `dt`, in the same
order: the game state is identical (this is tested, bit for bit).

## API

```cpp
FixedTimestep loop(60);                 // 60 Hz, a frame is clamped to 250 ms by default

loop.advance(elapsed, [&](float dt) {   // runs every step that is due, dt is constant
    systems.run(registry, dt);
});

loop.alpha();               // leftover fraction of a step, in [0,1)  (client render)
loop.timeUntilNextStep();   // how long to sleep (server, no rendering)
loop.tick();                // number of steps executed so far (e.g. snapshot number)
loop.stepSeconds();         // the constant dt
```

`FixedTimestep` never reads a clock: you give it the real elapsed time. This keeps it
independent of the platform and testable with a fake clock.

## Server loop (no rendering)

```cpp
r_type::loop::FixedTimestep loop(60);
auto previous = std::chrono::steady_clock::now();

while (running) {
    auto now = std::chrono::steady_clock::now();
    loop.advance(now - previous, [&](float dt) { systems.run(registry, dt); });
    previous = now;

    // send network updates here (or every N ticks, using loop.tick())
    std::this_thread::sleep_for(loop.timeUntilNextStep());   // no busy waiting
}
```

`sleep_for` may oversleep: it does not matter, the accumulator catches up on the next call.

## Client / demo loop (with rendering)

```cpp
while (!WindowShouldClose()) {
    auto now = std::chrono::steady_clock::now();
    pollInput(registry);                                   // sampled once per frame
    loop.advance(now - previous, [&](float dt) { systems.run(registry, dt); });
    previous = now;

    render(registry, loop.alpha());                        // once per frame, outside the steps
}
```

Rules:

- **Rendering is never called from inside the step callback**, and simulation code never reads the frame time.
- Input is read once per display frame and stored in the `Input` component; the steps then use the latest value.
- `alpha()` lets the render blend the previous and the current position so that motion stays smooth at 144 FPS with a 60 Hz simulation. This requires keeping the previous position of each rendered entity; it is optional and not part of this class. On the client, entities driven by the server are normally interpolated between network snapshots instead.

## Why the result is exactly the same at 30 and 144 FPS

The accumulator is an **integer** counting nanoseconds multiplied by the frequency
(`ns x Hz`). One step is consumed each time it reaches 1 000 000 000 (that is `1e9 / hz`
nanoseconds). Consequences:

- the number of steps after a given total time depends only on that total, never on how it was split into frames (30 FPS, 144 FPS, irregular frames...);
- no floating-point error accumulates in the time bookkeeping (a `double` accumulator can run one step early or late at the boundaries, and the error depends on the frame size);
- every step calls the simulation with the same `float dt`, so the same operations are done in the same order.

## Stalls: the "spiral of death"

If a frame takes very long (window dragged, debugger, laptop sleep), running every missed step
would take even longer and the game would never catch up. A single frame is therefore clamped to
`maxFrame` (250 ms by default, so at most 15 steps at 60 Hz). The time above that is dropped:
the game slows down once instead of freezing. Change it with `FixedTimestep(hz, maxFrame)`.

## Limits

- Bit-exact results are guaranteed for the same binary on the same platform. Different compilers or CPUs may round floating-point operations differently. This is not a problem here because the server is authoritative: clients do not need to reproduce the server bit for bit. It would matter for a rollback netcode.
- A step that throws is counted as consumed and the exception propagates to the caller; the object stays usable.
- Thread-safety: none. Use it from the game-logic thread only.

## Design choices (for the comparative study)

| Choice | Alternative | Why |
|--------|-------------|-----|
| Fixed timestep + accumulator | Variable `dt`; fully locked frame rate | Variable `dt` makes the simulation depend on the machine. Locking the frame rate wastes fast machines and breaks on slow ones. |
| Integer accumulator (`ns x Hz# Game loop: fixed timestep

Files: `project/include/shared/Loop/FixedTimestep.hpp`, tests in `project/tests/shared/Loop/`.

## The problem

If a system moves an entity by `speed * frameTime`, the result depends on the machine:
a 30 FPS client and a 144 FPS client do not compute the same positions, the server and
the client drift apart, and bugs (tunnelling through a hitbox, a cooldown that ends one
frame too late) cannot be reproduced.

## The solution: simulate in fixed steps

The simulation always advances by the **same** `dt` (e.g. 1/60 s). The display runs as
fast as it wants, and the *accumulator* converts the real elapsed time into a number of
simulation steps.

```
 real time ──► [ accumulator ] ──► 0, 1 or n steps of exactly dt ──► systems
                     │
                     └── leftover (alpha, in [0,1)) ──► render interpolation
```

| Display | Frame | What happens per frame (60 Hz simulation) |
|---------|-------|-------------------------------------------|
| 30 FPS  | 33.3 ms | 2 steps |
| 60 FPS  | 16.7 ms | 1 step |
| 144 FPS | 6.9 ms  | 0 or 1 step (about 4 frames out of 10 run a step) |

After 10 seconds all three have executed exactly 600 steps, with the same `dt`, in the same
order: the game state is identical (this is tested, bit for bit).

## API

```cpp
FixedTimestep loop(60);                 // 60 Hz, a frame is clamped to 250 ms by default

loop.advance(elapsed, [&](float dt) {   // runs every step that is due, dt is constant
    systems.run(registry, dt);
});

loop.alpha();               // leftover fraction of a step, in [0,1)  (client render)
loop.timeUntilNextStep();   // how long to sleep (server, no rendering)
loop.tick();                // number of steps executed so far (e.g. snapshot number)
loop.stepSeconds();         // the constant dt
```

`FixedTimestep` never reads a clock: you give it the real elapsed time. This keeps it
independent of the platform and testable with a fake clock.

## Server loop (no rendering)

```cpp
r_type::loop::FixedTimestep loop(60);
auto previous = std::chrono::steady_clock::now();

while (running) {
    auto now = std::chrono::steady_clock::now();
    loop.advance(now - previous, [&](float dt) { systems.run(registry, dt); });
    previous = now;

    // send network updates here (or every N ticks, using loop.tick())
    std::this_thread::sleep_for(loop.timeUntilNextStep());   // no busy waiting
}
```

`sleep_for` may oversleep: it does not matter, the accumulator catches up on the next call.

## Client / demo loop (with rendering)

```cpp
while (!WindowShouldClose()) {
    auto now = std::chrono::steady_clock::now();
    pollInput(registry);                                   // sampled once per frame
    loop.advance(now - previous, [&](float dt) { systems.run(registry, dt); });
    previous = now;

    render(registry, loop.alpha());                        // once per frame, outside the steps
}
```

Rules:

- **Rendering is never called from inside the step callback**, and simulation code never reads the frame time.
- Input is read once per display frame and stored in the `Input` component; the steps then use the latest value.
- `alpha()` lets the render blend the previous and the current position so that motion stays smooth at 144 FPS with a 60 Hz simulation. This requires keeping the previous position of each rendered entity; it is optional and not part of this class. On the client, entities driven by the server are normally interpolated between network snapshots instead.

## Why the result is exactly the same at 30 and 144 FPS

The accumulator is an **integer** counting nanoseconds multiplied by the frequency
(`ns x Hz`). One step is consumed each time it reaches 1 000 000 000 (that is `1e9 / hz`
nanoseconds). Consequences:

- the number of steps after a given total time depends only on that total, never on how it was split into frames (30 FPS, 144 FPS, irregular frames...);
- no floating-point error accumulates in the time bookkeeping (a `double` accumulator can run one step early or late at the boundaries, and the error depends on the frame size);
- every step calls the simulation with the same `float dt`, so the same operations are done in the same order.

## Stalls: the "spiral of death"

If a frame takes very long (window dragged, debugger, laptop sleep), running every missed step
would take even longer and the game would never catch up. A single frame is therefore clamped to
`maxFrame` (250 ms by default, so at most 15 steps at 60 Hz). The time above that is dropped:
the game slows down once instead of freezing. Change it with `FixedTimestep(hz, maxFrame)`.

## Limits

- Bit-exact results are guaranteed for the same binary on the same platform. Different compilers or CPUs may round floating-point operations differently. This is not a problem here because the server is authoritative: clients do not need to reproduce the server bit for bit. It would matter for a rollback netcode.
- A step that throws is counted as consumed and the exception propagates to the caller; the object stays usable.
- Thread-safety: none. Use it from the game-logic thread only.

## Design choices (for the comparative study)

| Choice | Alternative | Why |
|--------|-------------|-----|
| Fixed timestep + accumulator | Variable `dt`; fully locked frame rate | Variable `dt` makes the simulation depend on the machine. Locking the frame rate wastes fast machines and breaks on slow ones. |
| Integer accumulator (`ns x Hz`) | `double` accumulator; integer step in ns | No drift and no boundary error. An integer step in ns (1e9/60 is not an integer) would truncate and drift. |
| Clock given by the caller | `FixedTimestep` calling `steady_clock` itself | Testable with a synthetic clock (the 30 / 144 FPS tests run instantly and deterministically). |
| Clamp per frame | Cap on the number of steps | Same protection, expressed as a time. Easier to reason about. |

## Tests

```bash
ctest --test-dir build --output-on-failure -R "LoopDeterminism|FixedTimestep"
```

| Group | Covers |
|-------|--------|
| `LoopDeterminism.*` | same trajectory at 24 to 360 FPS and with irregular frame times, same step count, entities without `Velocity` untouched |
| `FixedTimestep.*` | constant dt, exact step counts for several frequencies, remainder kept, alpha range, stall clamp, invalid arguments, exception in a step, `timeUntilNextStep` |`) | `double` accumulator; integer step in ns | No drift and no boundary error. An integer step in ns (1e9/60 is not an integer) would truncate and drift. |
| Clock given by the caller | `FixedTimestep` calling `steady_clock` itself | Testable with a synthetic clock (the 30 / 144 FPS tests run instantly and deterministically). |
| Clamp per frame | Cap on the number of steps | Same protection, expressed as a time. Easier to reason about. |

## Tests

```bash
ctest --test-dir build --output-on-failure -R "LoopDeterminism|FixedTimestep"
```

| Group | Covers |
|-------|--------|
| `LoopDeterminism.*` | same trajectory at 24 to 360 FPS and with irregular frame times, same step count, entities without `Velocity` untouched |
| `FixedTimestep.*` | constant dt, exact step counts for several frequencies, remainder kept, alpha range, stall clamp, invalid arguments, exception in a step, `timeUntilNextStep` |