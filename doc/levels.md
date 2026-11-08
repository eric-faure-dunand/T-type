# Levels: JSON format and loader

Files: `project/include/shared/Level/` (`LevelData.hpp`, `LevelLoader.hpp`),
`project/src/shared/Level/`, level files in `assets/levels/`, tests in `project/tests/shared/Level/`.

## Who does what

| Level files and loader | Wave system |
|------------------------|-------------|
| `assets/levels/<country>.json` | the wave system that replaces the random spawn of `EnemySpawnSystem` |
| the JSON library in CMake | the movement patterns (`straight`, `sine`, `zigzag`, ...) |
| the loader: a JSON file becomes a `LevelData` | choosing the level and sending it to the client |

**`LevelData` is the border between the two sides.** The loader reads the file; the wave
system only ever sees the `LevelData` struct and never reads JSON.

## The file format

```json
{
  "name": "Istanbul",
  "country": "turquie",
  "owner": "Hilal",
  "background": "turquie",
  "waves": [
    { "time": 1,  "enemy": "simit", "pattern": "sine",     "count": 6, "spacing": 0.5 },
    { "time": 7,  "enemy": "cay",   "pattern": "straight", "count": 5, "spacing": 0.7 },
    { "time": 13, "enemy": "chat",  "pattern": "dive",     "count": 6, "spacing": 0.4 }
  ],
  "boss": { "type": "doner", "health": 60, "time": 40 }
}
```

| Field | Type | Rules |
|-------|------|-------|
| `name` | string | required, not empty |
| `country` | string | required, not empty. **Must equal the file name** (`turquie.json` -> `"turquie"`): it is the level id |
| `owner` | string | optional |
| `background` | string | required, not empty. Name of the background asset |
| `waves` | array | required, may be empty, at most 500 waves |
| `waves[].time` | number | seconds since the start of the level, 0 to 3600 |
| `waves[].enemy` | string | one of: `ballon`, `supporter`, `defenseur`, `scorpion`, `harissa`, `simit`, `cay`, `chat`, `grognard`, `sanglier` |
| `waves[].pattern` | string | one of: `straight`, `sine`, `zigzag`, `bounce`, `dive`, `wall`, `formation` |
| `waves[].count` | integer | how many enemies, 1 to 100 |
| `waves[].spacing` | number | seconds between two enemies of the wave, 0 to 3600 |
| `boss` | object | optional. Read, but **not used yet** |
| `boss.type` | string | required if there is a boss, not empty |
| `boss.health` | integer | 1 to 100000 |
| `boss.time` | number | seconds since the start of the level, 0 to 3600 |

Names are exact and lowercase (`"Simit"` is rejected).

Twists and bonuses (`pickup`) can be added later as **optional fields**: the loader ignores
fields it does not know, so existing files keep working. When a new field becomes real, add
it to `LevelData` and to `parseWave` / `parseLevel` in `LevelLoader.cpp`, with its checks.

## What the wave system receives

```cpp
struct WaveData  { float time; EnemyType enemy; Pattern pattern; std::uint32_t count; float spacing; };
struct BossData  { std::string type; int health; float time; };
struct LevelData { std::string name, country, owner, background;
                   std::vector<WaveData> waves;       // sorted by time
                   std::optional<BossData> boss; };
```

- `EnemyType` and `Pattern` are enums (1 byte), so the wave system never compares strings, and they can be sent over the network as is. `toString()` and `enemyTypeFromString()` convert from/to the JSON names.
- `waves` is **sorted by time** (stable: equal times keep the file order). The wave system can walk it with one index:

```cpp
// sketch, not compiled: enemy number k of wave w appears at  w.time + k * w.spacing
while (next < level.waves.size() && level.waves[next].time <= elapsed) { /* start wave `next` */ ++next; }
```

`elapsed` must come from the fixed-timestep simulation (sum of `dt`), never from a clock, so
that the same level plays out identically at any framerate (see `docs/game-loop.md`).

## Loading

```cpp
#include "Level/LevelLoader.hpp"          // server include dir: project/include/shared

try {
    const r_type::level::LevelData level = r_type::level::loadFromFile("assets/levels/turquie.json");
} catch (const r_type::level::LevelError &error) {
    std::clog << error.what() << '\n';    // says which file, which field, what is wrong
}
```

- `loadFromFile(path)` and `loadFromString(text, sourceName)`.
- **Any** problem throws `LevelError` (a `std::runtime_error`): file missing or not a regular file, file bigger than 1 MiB, invalid JSON, missing field, wrong type, value out of range, unknown enemy or pattern. The message points to the exact place:

```
levels/turquie.json: waves[1].enemy: unknown enemy 'dragon' (expected one of: ballon, supporter, ...)
levels/turquie.json: waves[0].count: must be between 1 and 100
levels/turquie.json: missing field 'background'
```

- A broken level file therefore cannot crash the server: catch `LevelError`, log it, refuse to start that level. Truncated and randomly corrupted files are part of the tests.
- Limits (`kMaxWaves`, `kMaxCountPerWave`, ...) are in `LevelData.hpp`. They keep a wrong file from spawning thousands of enemies (the ECS holds at most `kMaxEntities` = 5254 entities).
- Load the levels **once** (at start-up or when a `Room` is created), not every frame.

## Adding a level

1. Create `assets/levels/<country>.json` with the format above. `"country"` must equal the file name.
2. Run the tests: `ctest --test-dir build -R ShippedLevels`. Every `.json` in `assets/levels` is loaded and checked automatically; the message tells you what to fix.
3. Make sure the `background` asset exists on the client side (not checked by the loader).

## Build

- `nlohmann/json` 3.11.3 is fetched by CPM in the root `CMakeLists.txt` (release archive, much smaller than the git repository), **before** `add_subdirectory(project)`.
- The headers contain no JSON: only `LevelLoader.cpp` includes it. The client can use `LevelData` without the JSON library.
- The server compiles `src/shared/Level/*.cpp` through its existing `project/src/shared` glob and links `nlohmann_json::nlohmann_json`.
- The same files are built as the `r_type::level` library for the tests.

## Tests

```bash
ctest --test-dir build --output-on-failure -R "LevelLoader|LevelFiles|ShippedLevels"
```

| Group | Covers |
|-------|--------|
| `LevelLoader.*` | the Istanbul example, every enemy and pattern name, optional fields, unknown fields ignored, sorting, limits, one test per kind of error (with the message), truncated / corrupted / deeply nested input |
| `LevelFiles.*` | loading from disk, missing file, directory, file too large, error message naming the file |
| `ShippedLevels.*` | every file of `assets/levels` is valid and has `country` equal to its name |