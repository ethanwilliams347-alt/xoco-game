# Implementation Plan — Multi-Layer Parallax Support & Scene Logic

This plan provides a step-by-step implementation guide to support arbitrary multi-layer parallax backgrounds (such as the 9-layer `Background_1` set in `art_src/Background_1/`) with custom per-layer parallax factors, depth ordering, and scene definition logic.

> [!IMPORTANT]
> **Instructions for Claude Opus 5 (Low Effort):**
> Execute the phases in strict order. Follow all exact file paths, function signatures, before/after code blocks, and static asserts. Maintain 100% determinism, 16.16 fixed-point arithmetic (`fx`), and ensure all 19 `ctest` suites pass after each phase.

---

## Architectural Overview

```
+---------------------------------------------------------------------------------------------------+
|                                 MULTI-LAYER PARALLAX RENDER ORDER                                 |
+------------------------------------+------------------------+-------------------------------------+
| Layer 9: Sky & Upper Atmosphere    | Parallax: 0.04x, 0.02x | Fully Opaque                        |
| Layer 7: Distant Mountain Peaks    | Parallax: 0.12x, 0.05x | Color-keyed Silhouette              |
| Layer 6: Far Hill Ridge            | Parallax: 0.20x, 0.08x | Color-keyed Silhouette              |
| Layer 5: Mid-Far Hill Mass         | Parallax: 0.30x, 0.12x | Color-keyed Silhouette              |
| Layer 4: Mid Hill Mass             | Parallax: 0.42x, 0.16x | Color-keyed Silhouette              |
| Layer 3: Mid-Near Hill Mass        | Parallax: 0.55x, 0.20x | Color-keyed Silhouette              |
| Layer 2: Near Hill Mass            | Parallax: 0.70x, 0.25x | Color-keyed Silhouette              |
| Layer 8: Ground / Water Plane      | Parallax: 0.28x-0.52x  | Receding Depth Ramp                 |
| Layers 3 & 4: World Props & Cells  | Parallax: 1.00x, 1.00x | Simulated World & Anchored Props    |
| Player Character                   | Parallax: 1.00x, 1.00x | Animated Sprite                     |
| Layer 1: Foreground Rocks          | Parallax: 1.15x, 1.10x | Front Silhouette (in front of body) |
| World Grade & Additive Light Pass  | Screen-space Multiply  | Light Quad & Night Tint             |
| UI / Reticle / HUD                 | Unlit Screen Overlay   | Top-level Present                   |
+------------------------------------+------------------------+-------------------------------------+
```

---

## Phased Implementation Roadmap

```
+-----------------------------------------------------------------------------------+
|                               PHASED IMPLEMENTATION                               |
+------------------------------------+----------------------------------------------+
| Phase 1: Frame Multi-Layer Structs | Add ParallaxLayer & vector to frame::Backdrop|
| Phase 2: Frame Composition Pipeline| Draw background & foreground layer arrays    |
| Phase 3: Asset Conversion Pipeline | Convert art_src/Background_1 PNGs to BMPs    |
| Phase 4: Scene Manifest & Loading  | Wire multi-layer loading into activate_scene |
| Phase 5: Verification & Tests      | Verify ctest pass rate and visual composition|
+------------------------------------+----------------------------------------------+
```

---

### Phase 1: Multi-Layer Parallax Structs (`src/render/frame.h`)

#### [MODIFY] [src/render/frame.h](file:///c:/Users/Ethan/Desktop/game/code/src/render/frame.h)
1. Add `ParallaxLayer` struct definition:
```cpp
// One layer in a custom multi-layer backdrop stack.
struct ParallaxLayer {
    SDL_Texture* texture = nullptr;
    int w = 0, h = 0;
    float parallax_x = 1.0f;
    float parallax_y = 1.0f;
    Grade grade{};
    bool is_foreground = false; // true if drawn in front of player & terrain cells
};
```

2. Update `struct Backdrop` to support custom layer vectors alongside legacy fields:
```cpp
struct Backdrop {
    // Legacy 3-layer fields (preserved for backwards compatibility & golden frame test)
    SDL_Texture* sky = nullptr;
    SDL_Texture* mountains = nullptr;
    SDL_Texture* ground = nullptr;
    int sky_w = 0, sky_h = 0;
    int mountain_w = 0, mountain_h = 0;
    int ground_w = 0, ground_h = 0;

    // Custom multi-layer stack (if non-empty, rendered in vector order)
    std::vector<ParallaxLayer> layers;
};
```

---

### Phase 2: Render Multi-Layer Parallax Stack (`src/render/frame.cpp`)

#### [MODIFY] [src/render/frame.cpp](file:///c:/Users/Ethan/Desktop/game/code/src/render/frame.cpp)
1. Add custom layer draw functions:
```cpp
void draw_custom_background_layers(SDL_Renderer* renderer, const Params& p, const Grade&) {
    if (p.backdrop.layers.empty()) return;
    for (const ParallaxLayer& l : p.backdrop.layers) {
        if (!l.is_foreground && l.texture) {
            const backdrop_layers::Layer layer_spec{l.parallax_x, l.parallax_y, l.w, l.h};
            draw_backdrop_layer(renderer, p, l.texture, l.w, l.h, layer_spec, l.grade);
        }
    }
}

void draw_custom_foreground_layers(SDL_Renderer* renderer, const Params& p, const Grade&) {
    if (p.backdrop.layers.empty()) return;
    for (const ParallaxLayer& l : p.backdrop.layers) {
        if (l.is_foreground && l.texture) {
            const backdrop_layers::Layer layer_spec{l.parallax_x, l.parallax_y, l.w, l.h};
            draw_backdrop_layer(renderer, p, l.texture, l.w, l.h, layer_spec, l.grade);
        }
    }
}
```

2. Update `draw_sky` and `draw_mountains` to check `p.backdrop.layers.empty()`:
```cpp
void draw_sky(SDL_Renderer* renderer, const Params& p, const Grade& g) {
    if (!p.backdrop.layers.empty()) return; // Custom layer stack handles background
    draw_backdrop_layer(renderer, p, p.backdrop.sky,
                        p.backdrop.sky_w, p.backdrop.sky_h, backdrop_layers::SKY, g);
}

void draw_mountains(SDL_Renderer* renderer, const Params& p, const Grade& g) {
    if (!p.backdrop.layers.empty()) return; // Custom layer stack handles background
    draw_backdrop_layer(renderer, p, p.backdrop.mountains,
                        p.backdrop.mountain_w, p.backdrop.mountain_h,
                        backdrop_layers::MOUNTAINS, g);
}
```

3. Update the `TABLE` in `frame.cpp` to include custom background and foreground passes:
```cpp
constexpr Layer TABLE[] = {
    {"clear",             Lighting::Lit,   PLAIN,           draw_clear},
    {"sky",               Lighting::Lit,   PLAIN,           draw_sky},
    {"mountains",         Lighting::Lit,   {153, 153, 153}, draw_mountains},
    {"custom_background", Lighting::Lit,   PLAIN,           draw_custom_background_layers},
    {"ground",            Lighting::Lit,   {135, 135, 135}, draw_ground},
    {"props",             Lighting::Lit,   PLAIN,           draw_props},
    {"cells",             Lighting::Lit,   PLAIN,           draw_cells},
    {"objective",         Lighting::Lit,   PLAIN,           draw_objective},
    {"player",            Lighting::Lit,   PLAIN,           draw_player},
    {"custom_foreground", Lighting::Lit,   PLAIN,           draw_custom_foreground_layers},
    {"grade",             Lighting::Grade, PLAIN,           draw_grade},
    {"light",             Lighting::Light, PLAIN,           draw_light},
};
```

---

### Phase 3: Asset Conversion Pipeline Script (`tools/convert_background_layers.py`)

#### [NEW] [tools/convert_background_layers.py](file:///c:/Users/Ethan/Desktop/game/code/tools/convert_background_layers.py)
Create conversion script to process `art_src/Background_1/` into `assets/bg1/`:
```python
"""Converts Background_1 layer PNGs into 24-bit BMPs in assets/bg1/."""
import os
import sys

from pixel_art import COLOR_KEY, write_bmp
from gemini_to_player_frame import read_png
from png_to_bmp import convert

LAYERS = [
    ("01_foreground_rocks.png", "bg1_01_fg_rocks.bmp", True),
    ("02_hills_near.png", "bg1_02_hills_near.bmp", True),
    ("03_hills_midnear.png", "bg1_03_hills_midnear.bmp", True),
    ("04_hills_mid.png", "bg1_04_hills_mid.bmp", True),
    ("05_hills_midfar.png", "bg1_05_hills_midfar.bmp", True),
    ("06_hills_far.png", "bg1_06_hills_far.bmp", True),
    ("07_distant_mountains.png", "bg1_07_mountains.bmp", True),
    ("08_ground_plane.png", "bg1_08_ground.bmp", False),
    ("09_sky.png", "bg1_09_sky.bmp", False),
]

def main():
    src_dir = "art_src/Background_1"
    dst_dir = "assets/bg1"
    os.makedirs(dst_dir, exist_ok=True)

    for src_name, dst_name, has_alpha in LAYERS:
        src_path = os.path.join(src_dir, src_name)
        dst_path = os.path.join(dst_dir, dst_name)
        if not os.path.exists(src_path):
            print(f"Warning: {src_path} not found")
            continue
        w, h, pixels, alpha = read_png(src_path)
        out_pixels = convert(w, h, pixels, alpha if has_alpha else None)
        write_bmp(dst_path, w, h, out_pixels)
        print(f"Converted {src_name} -> {dst_path} ({w}x{h})")

if __name__ == "__main__":
    main()
```

---

### Phase 4: Scene Manifest & Texture Loading (`main.cpp`)

#### [MODIFY] [assets/scenes.txt](file:///c:/Users/Ethan/Desktop/game/code/assets/scenes.txt)
Add `bg1` multi-layer scene definition:
```text
# name     material           albedo           props           spawn    mode      width  height
empty      -                  -                -               floor    infinite  1920   1080
bg1        -                  -                -               floor    fixed     344    144
```

#### [MODIFY] [src/main.cpp](file:///c:/Users/Ethan/Desktop/game/code/src/main.cpp)
In `activate_scene`:
If scene name is `"bg1"`:
Populate `backdrop.layers` with the 9 layers and their respective parallax factors:
```cpp
if (def.name == "bg1") {
    backdrop.layers.clear();
    // Sky (farthest back)
    backdrop.layers.push_back({load_art_texture(renderer, "assets/bg1/bg1_09_sky.bmp", false), 344, 144, 0.04f, 0.02f, {255, 255, 255}, false});
    // Mountains
    backdrop.layers.push_back({load_art_texture(renderer, "assets/bg1/bg1_07_mountains.bmp", true), 344, 144, 0.12f, 0.05f, {220, 220, 220}, false});
    // Hills Far -> Near
    backdrop.layers.push_back({load_art_texture(renderer, "assets/bg1/bg1_06_hills_far.bmp", true), 344, 144, 0.20f, 0.08f, {200, 200, 200}, false});
    backdrop.layers.push_back({load_art_texture(renderer, "assets/bg1/bg1_05_hills_midfar.bmp", true), 344, 144, 0.30f, 0.12f, {190, 190, 190}, false});
    backdrop.layers.push_back({load_art_texture(renderer, "assets/bg1/bg1_04_hills_mid.bmp", true), 344, 144, 0.42f, 0.16f, {180, 180, 180}, false});
    backdrop.layers.push_back({load_art_texture(renderer, "assets/bg1/bg1_03_hills_midnear.bmp", true), 344, 144, 0.55f, 0.20f, {170, 170, 170}, false});
    backdrop.layers.push_back({load_art_texture(renderer, "assets/bg1/bg1_02_hills_near.bmp", true), 344, 144, 0.70f, 0.25f, {160, 160, 160}, false});
    // Foreground rocks (drawn in front of player)
    backdrop.layers.push_back({load_art_texture(renderer, "assets/bg1/bg1_01_fg_rocks.bmp", true), 344, 144, 1.15f, 1.10f, {255, 255, 255}, true});
} else {
    backdrop.layers.clear();
}
```

---

## Verification Plan

### Automated Verification
```powershell
# 1. Convert background layer assets
python tools/convert_background_layers.py

# 2. Stage assets and build
python tools/load_sprite.py --stage
cmake --build build --config Release

# 3. Run headless test suite
ctest --test-dir build -C Release --output-on-failure
```

### Invariant Checks:
1. `sizeof(Element) <= 12` is unaffected.
2. `golden_frame_test` passes (proving standard 3-layer scenes produce identical checksums).
3. All 19 tests in `ctest` report `Passed`.
4. Pressing `F7` in-game switches to `bg1` and smoothly displays all 9 parallax layers at different scroll speeds.
