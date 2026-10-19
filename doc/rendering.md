# Rendering: graphics library isolated behind an interface

The game code never touches Raylib. It talks to two small interfaces; one class
(`RaylibBackend`) implements them, and it is the **only** code that includes
`raylib.h`. Switching to SDL or SFML later means writing one new class.

```
 game code (systems, main loop)
        │  uses only
        ▼
 IRenderer   IKeyboard          renderSystem   TextureManager   Sprite
 (window, textures, draw)  (keys)    (all graphics-library free, always built)
        ▲
        │ implemented by
 RaylibBackend  ── the only file that includes raylib.h (src/client/Render/RaylibBackend.cpp)
```

## Files

| File | Role |
|------|------|
| `include/client/Render/IRenderer.hpp` | abstract window + sprite drawing (`TextureId`, `Color`, `Rect`, `Size`) |
| `include/client/Input/IKeyboard.hpp` | abstract keyboard (`Key`, `isKeyDown`, `isKeyPressed`) |
| `include/client/Render/Sprite.hpp` | `Sprite` component |
| `include/client/Render/TextureManager.hpp` | loads each texture file once |
| `include/client/Render/RenderSystem.hpp` | draws every entity with `Position` + `Sprite` |
| `include/client/Render/RaylibBackend.hpp` | Raylib implementation (header has no Raylib include) |
| `include/shared/Components/Transform.hpp` | `Position`, `Velocity` (shared with the server) |
| `src/client/Render/*.cpp` | implementations; only `RaylibBackend.cpp` includes `raylib.h` |

`Position`, `Velocity` and `Sprite` are plain structs: no logic, no graphics type.

## Prerequisites

Raylib is fetched and built by CPM at the first configure (about a minute). On Linux it needs the X11 / OpenGL
development packages:

```bash
# Debian / Ubuntu
sudo apt install libx11-dev libxrandr-dev libxinerama-dev libxcursor-dev libxi-dev libgl1-mesa-dev
# Fedora
sudo dnf install libX11-devel libXrandr-devel libXinerama-devel libXcursor-devel libXi-devel mesa-libGL-devel
```

Windows needs nothing more. The server must not depend on Raylib: configure with `-DRTYPE_BUILD_CLIENT=OFF`
to skip it (the graphics-free render module and its tests are still built).

## Usage (client main)

```cpp
using namespace r_type;

render::RaylibBackend backend;                  // IRenderer + IKeyboard
backend.open(1280, 720, "R-Type");
backend.setTargetFps(144);
render::TextureManager textures(backend);

ecs::Registry registry;
registry.registerComponent<components::Position>();
registry.registerComponent<render::Sprite>();   // client only: the server never registers it

ecs::Entity ship = registry.spawn();
registry.add<components::Position>(ship, 100.f, 200.f);
registry.add<render::Sprite>(ship, render::Sprite{
    textures.get("assets/sprites/ship.png"),    // loaded once, cached
    render::Rect{0, 0, 33, 17},                 // part of the sprite sheet (w or h <= 0 = whole image)
    3.f,                                        // scale
    1});                                        // layer: higher = in front

while (!backend.shouldClose()) {
    // simulation: FixedTimestep (see docs/game-loop.md), NOT inside the render
    backend.beginFrame(render::Color{0, 0, 0});
    // background first, then:
    render::renderSystem(registry, backend);
    backend.endFrame();
}
```

`renderSystem` has the signature `void(Registry &, IRenderer &)`, not `void(Registry &, float)`: it is called **once per
display frame by the main loop**, not by the fixed-step `SystemManager`.

## Conventions

- `Position` is the **top-left corner** of the sprite. World units are screen pixels, y grows downwards.
- Draw order: ascending `Sprite::layer`, then ascending entity index (deterministic).
- A `Sprite` with `texture == kInvalidTexture` is skipped, not an error.
- `renderSystem` throws `std::out_of_range` if `Position` or `Sprite` is not registered.
- Everything graphical (window, textures, drawing, keyboard) must be used from the **main thread** (Raylib requirement).
- Escape does not close the window; read `Key::Escape` and decide in the game.

## Keeping Raylib out of the rest of the code

Three layers enforce the rule:

1. `RaylibBackend.hpp` uses a pimpl: no Raylib type or include in the header.
2. Raylib is linked `PRIVATE` to `r_type_raylib_backend`: its include directories are **not** given to other targets, so `#include <raylib.h>` elsewhere does not compile.
3. The ctest `NoRaylibOutsideRenderModule` scans `project/` and fails if `raylib.h`, `raymath.h`, `rlgl.h` or `raygui.h` is included anywhere except `src/client/Render/`.

## Tests

```bash
ctest --test-dir build --output-on-failure              # everything
ctest --test-dir build -LE gui                          # CI without a display
xvfb-run -a ctest --test-dir build -L gui               # the window tests on a headless Linux
```

| Test | Covers |
|------|--------|
| `RenderSystemTest.*` | an entity with a sprite is drawn with the right texture / position / scale / source; entities without `Position` or `Sprite` are not; layer order; killed entities; invalid textures; no frame management inside the system |
| `TextureManager.*` | one load per file, distinct ids, failed load not cached, `clear()` |
| `RaylibBackend.*` (label `gui`) | real window, BMP texture loading, drawing, errors, close / reopen. Skipped when there is no display |
| `NoRaylibOutsideRenderModule` | the "no Raylib include outside the render module" rule |

The render-system tests use a `FakeRenderer` (records draw calls), so they need no window and no Raylib.

## Limits and next steps

- One window at a time (Raylib is global). `open()` throws `std::logic_error` if one is already open.
- No rotation, flip, tint, sprite animation or scrolling background yet (`Animation` and `Background` are separate issues).
- Textures are not reference counted: `TextureManager::clear()` or closing the window frees them all.
- Keyboard only. Remappable keys and gamepad support belong to the accessibility work.
- Only Linux was tested (software OpenGL under Xvfb). Windows should work (Raylib supports it) but is untested.

## Design choices (for the comparative study)

| Choice | Alternative | Why |
|--------|-------------|-----|
| Abstract interface (virtual) | templates on the backend; `#ifdef` per library | One virtual call per sprite is negligible at this scale; the interface is readable and a fake can replace it in tests. |
| Two interfaces (`IRenderer`, `IKeyboard`) | one big interface | Input code depends only on the keyboard, not on drawing. The Raylib class implements both because Raylib ties them to one window. |
| Opaque `TextureId` | pointer to a texture object | No library type leaks; ids are cheap to store in a component and to compare. |
| Pimpl in `RaylibBackend` | Raylib members in the header | The header can be included anywhere without pulling `raylib.h`. |
| Raylib through CPM, `PRIVATE` link | system-installed Raylib; public link | Self-contained build (subject requirement) and include directories do not leak. |
| `FakeRenderer` in tests | testing only with a real window | Fast, deterministic tests that run on any machine and in CI. |