# Implementation Plan — Infinite Scrolling & Large Fixed Bounded Scenes

This document provides a complete, step-by-step engineering plan to support two distinct scene and backdrop paradigms:
1. **Infinite Scrolling Scenes:** Backgrounds and parallax layers wrap/tile seamlessly across horizontal/vertical coordinates with unbounded camera movement.
2. **Large Fixed Bounded Scenes:** Large authored scenes with solid world borders, clamped camera navigation, and continuous non-repeating backdrop textures spanning the full pan range.

> [!IMPORTANT]
> **Instructions for Claude Opus 5 (Low Effort):**
> Execute the phases in sequence. Adhere strictly to the exact file paths, function signatures, mathematical formulas, and data structures. Do not break existing deterministic simulation invariants, 16.16 fixed-point math (`fx`), or the 19 headless `ctest` test suites.

---

## Architectural Overview

```
+-----------------------------------------------------------------------------------------------+
|                                      SCENE PARADIGMS                                          |
+-----------------------------------------------+-----------------------------------------------+
| 1. INFINITE SCROLLING SCENE                   | 2. LARGE FIXED BOUNDED SCENE                  |
| - Unbounded / wrapping camera coordinates     | - Solid world boundaries (W x H)              |
| - Tiled backdrop layers (wrap_axis)           | - Clamped camera: [0, W - view_w]             |
| - Continuous seamless repetition              | - Non-repeating authored art (no seams)       |
| - Modular world chunk streaming               | - Normalized pan mapping across world width   |
+-----------------------------------------------+-----------------------------------------------+
```

---

## Phased Implementation Roadmap

```
+-----------------------------------------------------------------------------------+
|                               PHASED IMPLEMENTATION                               |
+------------------------------------+----------------------------------------------+
| Phase 1: Scene Schema & Mode Parser| Add SceneMode (Fixed vs Infinite) to SceneDef|
| Phase 2: Camera Multi-Mode Nav     | Support clamped pan & unbounded wrapping     |
| Phase 3: Non-Repeating Parallax    | Normalized pan-span formula for fixed scenes |
| Phase 4: Dynamic Grid Resizing     | Allow Run & Grid to resize on scene activate |
| Phase 5: Scene Manifest & Tests    | Update scenes.txt & write test cases         |
+------------------------------------+----------------------------------------------+
```

---

### Phase 1: Scene Definition & Manifest Extension (`scene_list`)

Extend `SceneDef` in [`src/scene/scene_list.h`](file:///c:/Users/Ethan/Desktop/game/code/src/scene/scene_list.h) to distinguish between fixed and infinite scenes, and store world dimensions.

#### [MODIFY] [src/scene/scene_list.h](file:///c:/Users/Ethan/Desktop/game/code/src/scene/scene_list.h)
1. Add `SceneMode` enum:
```cpp
enum class SceneMode : uint8_t {
    Fixed,     // Bounded world with solid borders; non-repeating backdrops
    Infinite,  // Unbounded / wrapping world; tiling backdrop layers
};
```
2. Extend `SceneDef` struct:
```cpp
struct SceneDef {
    std::string name;
    std::string material;
    std::string albedo;
    std::string props;
    Spawn spawn = Spawn::Terrain;
    SceneMode mode = SceneMode::Fixed;
    int custom_width = 0;   // 0 = derive from material BMP or default (1920)
    int custom_height = 0;  // 0 = derive from material BMP or default (1080)
    int line = 0;

    bool declared_empty() const { return material.empty() && albedo.empty(); }
    bool is_infinite() const { return mode == SceneMode::Infinite; }
};
```

#### [MODIFY] [src/scene/scene_list.cpp](file:///c:/Users/Ethan/Desktop/game/code/src/scene/scene_list.cpp)
1. Update `load_scene_list` to parse optional 6th (`mode`) and 7th/8th (`width height`) tokens:
   - If token is `"fixed"`, `def.mode = SceneMode::Fixed`.
   - If token is `"infinite"`, `def.mode = SceneMode::Infinite`.
   - Backward compatibility: If omitted on a line, default `mode` is `SceneMode::Fixed`.
   - If optional `width` and `height` integers are present, populate `custom_width` and `custom_height`.

---

### Phase 2: Multi-Mode Camera Navigation (`camera.h`)

Update [`src/game/camera.h`](file:///c:/Users/Ethan/Desktop/game/code/src/game/camera.h) to support both clamped boundary tracking and infinite/wrapping tracking.

#### [MODIFY] [src/game/camera.h](file:///c:/Users/Ethan/Desktop/game/code/src/game/camera.h)
1. Add `view_fx()` and `view_fy()` accessors if not present:
```cpp
    float view_fx() const { return view_fx_; }
    float view_fy() const { return view_fy_; }
```
2. Add `follow_mode` method:
```cpp
    void follow_mode(float center_x, float center_y,
                     int viewport_w, int viewport_h,
                     int world_w, int world_h,
                     bool is_infinite) {
        if (is_infinite) {
            // Infinite horizontal scrolling: unbounded / wrapped tracking
            view_fx_ = center_x - static_cast<float>(viewport_w) / 2.0f;
            // Vertical remains clamped to world height ceiling and floor
            view_fy_ = clamp_view(center_y - static_cast<float>(viewport_h) * VERTICAL_ANCHOR,
                                  viewport_h, world_h);
        } else {
            // Large fixed scene: strictly clamped within world borders
            view_fx_ = clamp_view(center_x - static_cast<float>(viewport_w) / 2.0f, viewport_w, world_w);
            view_fy_ = clamp_view(center_y - static_cast<float>(viewport_h) * VERTICAL_ANCHOR, viewport_h, world_h);
        }
    }
```
3. Keep existing `follow(...)` delegating to `follow_mode(..., false)`.

---

### Phase 3: Non-Repeating Fixed Backdrop Pan vs. Infinite Tiling (`frame.cpp`)

In a large fixed scene, a backdrop layer (e.g. mountains or panoramic skyline) must span from $X=0$ (when the player is at the left world border) to $X = -(\text{texture\_w} - \text{window\_w})$ (when the player reaches the right world border) **without repeating or running out of image**.

#### Parallax Mathematics:
1. **Fixed Bounded Scene (Normalized Pan-Span):**
   $$\text{max\_cam\_x} = \text{world\_w} - \text{viewport\_w}$$
   $$u_x = \frac{\text{view\_fx}}{\text{max\_cam\_x}} \quad (0 \le u_x \le 1)$$
   $$\text{dst\_x} = -u_x \cdot (\text{texture\_w} - \text{window\_w})$$
   *Result:* At the left border ($u_x = 0$), the layer's left edge is at $X=0$. At the right border ($u_x = 1$), the layer's right edge meets the window's right edge. **No repeating textures, no black seams, no cutoffs.**

2. **Infinite Scene (Continuous Wrapping):**
   $$\text{origin\_x} = -(\text{view\_x} + \text{frac\_x}) \cdot \text{SCALE} \cdot \text{factor}$$
   Uses `backdrop_wrap::wrap_axis(origin_x, tile_w, window_w)` for seamless tiling.

#### [MODIFY] [src/render/frame.h](file:///c:/Users/Ethan/Desktop/game/code/src/render/frame.h)
1. In `frame::Params`, add:
```cpp
    bool is_infinite = false;
    int world_w = 1920;
    int world_h = 1080;
```

#### [MODIFY] [src/render/frame.cpp](file:///c:/Users/Ethan/Desktop/game/code/src/render/frame.cpp)
1. Update `draw_backdrop_layer`:
```cpp
void draw_backdrop_layer(SDL_Renderer* renderer, const Params& p,
                         SDL_Texture* tex, int w, int h,
                         const backdrop_layers::Layer& layer, const Grade& g) {
    if (!tex) return;
    apply_grade(tex, g);

    const Camera& camera = *p.camera;
    const int window_w = p.padded_w * Camera::SCALE;
    const int window_h = p.padded_h * Camera::SCALE;

    if (p.is_infinite) {
        // Infinite scrolling: wrap using tiling arithmetic
        const backdrop_wrap::Tiling t = backdrop_wrap::wrap_axis(
            camera.parallax_origin_x(layer.parallax_x), w, window_w);
        for (int c = 0; c < t.count; ++c) {
            const SDL_FRect dst{
                t.first + static_cast<float>(c * w),
                camera.parallax_origin_y(layer.parallax_y),
                static_cast<float>(w), static_cast<float>(h)
            };
            SDL_RenderCopyF(renderer, tex, nullptr, &dst);
        }
    } else {
        // Fixed scene: normalized pan across borders (no tiling, no repeats)
        const float max_cam_x = static_cast<float>(std::max(1, p.world_w - p.padded_w));
        const float max_cam_y = static_cast<float>(std::max(1, p.world_h - p.padded_h));

        const float u_x = std::clamp(camera.view_fx() / max_cam_x, 0.0f, 1.0f);
        const float u_y = std::clamp(camera.view_fy() / max_cam_y, 0.0f, 1.0f);

        const float span_x = static_cast<float>(std::max(0, w - window_w));
        const float span_y = static_cast<float>(std::max(0, h - window_h));

        const SDL_FRect dst{
            -u_x * span_x,
            -u_y * span_y,
            static_cast<float>(w),
            static_cast<float>(h)
        };
        SDL_RenderCopyF(renderer, tex, nullptr, &dst);
    }
}
```

---

### Phase 4: Dynamic Grid Resizing on Scene Activation (`main.cpp` & `run.h`)

Allow `Run` and `Grid` to support scenes of different dimensions without restarting the engine.

#### [MODIFY] [src/game/run.h](file:///c:/Users/Ethan/Desktop/game/code/src/game/run.h)
1. Extend `Run::reset` to support optional resizing:
```cpp
    void reset(uint64_t seed, int new_width = 0, int new_height = 0);
```

#### [MODIFY] [src/game/run.cpp](file:///c:/Users/Ethan/Desktop/game/code/src/game/run.cpp)
```cpp
void Run::reset(uint64_t seed, int new_width, int new_height) {
    if (new_width > 0 && new_height > 0 && 
        (new_width != grid.get_width() || new_height != grid.get_height())) {
        grid = Grid(new_width, new_height, seed);
    } else {
        grid.reset(seed);
    }
    player = Player(grid.get_width() / 2, grid.get_height() / 4);
    dig_tool = DigTool();
    run_outcome = Outcome::Playing;
}
```

#### [MODIFY] [src/main.cpp](file:///c:/Users/Ethan/Desktop/game/code/src/main.cpp)
1. In `activate_scene(int index)`:
   - Determine target scene width and height (from `def.custom_width`/`custom_height` or loaded BMP header).
   - Pass dimensions to `run.reset(world_seed, target_w, target_h)`.
   - Pass `def.is_infinite()`, `target_w`, and `target_h` into `frame::Params`.

---

### Phase 5: Manifest Configurations & Verification

#### [MODIFY] [assets/scenes.txt](file:///c:/Users/Ethan/Desktop/game/code/assets/scenes.txt)
Configure entries for both infinite and fixed scenes:
```text
# name        material           albedo           props           spawn    mode      width  height
empty         -                  -                -               floor    infinite  1920   1080
large_canyon  test_material.bmp  test_albedo.bmp  test_props.txt  terrain  fixed     3840   1440
```

---

## Verification Suite

### Automated Verification
```powershell
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
```

### Specific Test Cases:
1. **Fixed Bounded Scene Test:**
   - Walk player to $X=0$: Camera clamps at 0; backdrop displays at left edge ($X=0$).
   - Walk player to $X = \text{world\_w}$: Camera clamps at right border; backdrop aligns flush with right window edge with **no wrapping** and **no repetition**.
2. **Infinite Scrolling Scene Test:**
   - Walk player continuously in one direction: camera and backdrop wrap seamlessly with `wrap_axis`.
3. **100% Determinism:**
   - All 19 existing headless `ctest` suites continue to pass with 0 errors.
