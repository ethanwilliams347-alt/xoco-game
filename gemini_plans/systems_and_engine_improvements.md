# Implementation Plan — Systems & Engine Improvements

This plan addresses the technical debt, memory allocations, and performance/safety improvements identified in the Senior Game Developer Code Review.

> [!IMPORTANT]
> **Instructions for Claude Opus 5 (Low Effort):**
> Follow the exact step-by-step instructions below. Do not deviate from the specified file paths, types, or algorithmic constraints. Maintain all compile-time assertions (`static_assert`), 16.16 fixed-point arithmetic (`fx`), integer determinism, and comment reasoning. After each phase, execute the verification suite to ensure all tests pass.

---

## User Review Required

> [!NOTE]
> All improvements preserve 100% determinism, exact 12-byte `Element` packing, and bit-identical test outputs across platforms. No breaking API changes are introduced.

---

## Proposed Changes

```
+-----------------------------------------------------------------------------------+
|                               PHASED IMPLEMENTATION                               |
+------------------------------------+----------------------------------------------+
| Phase 1: Grid Memory & Allocations | Eliminate heap churn in support/fracture.   |
| Phase 2: Piece Tag Wraparound Guard| Collision-free active piece tag allocator.   |
| Phase 3: Binary Division-Free isqrt| Branchless/division-free integer isqrt.      |
| Phase 4: Zero-Alloc Depth Map Pass | Eliminate vector allocation in depth_map.    |
| Phase 5: Architecture Documentation| Record 4-Phase Checkerboard parallel design. |
+------------------------------------+----------------------------------------------+
```

---

### Phase 1: Eliminate Heap Allocation Churn in Support & Fracture Flood Fills

**Problem:** [`src/physics/grid.cpp`](file:///c:/Users/Ethan/Desktop/game/code/src/physics/grid.cpp) dynamically instantiates `std::vector<int> seeds`, `std::vector<int> deferred` inside `resolve_support()`, and `std::vector<int> lowest` inside `fracture_landing()`. During structural cascades, these trigger repetitive heap allocations on the critical simulation path.

#### [MODIFY] [grid.h](file:///c:/Users/Ethan/Desktop/game/code/src/physics/grid.h)
- In the private section of `Grid`, declare persistent scratch buffers:
```cpp
    // Scratch vectors for support resolution & fracture (reused across passes)
    std::vector<int> support_seeds;
    std::vector<int> support_deferred;
    std::vector<int> fracture_lowest;
```

#### [MODIFY] [grid.cpp](file:///c:/Users/Ethan/Desktop/game/code/src/physics/grid.cpp)
- In `Grid::Grid(int width, int height, uint64_t seed)` and `Grid::reset(uint64_t seed)`:
  Clear and reserve sensible capacities for the scratch vectors:
  ```cpp
  support_seeds.clear();
  support_deferred.clear();
  fracture_lowest.clear();
  ```
- In `Grid::resolve_support()`:
  Replace the local vectors:
  ```cpp
  // Replace:
  // std::vector<int> seeds;
  // std::vector<int> deferred;
  // With:
  support_seeds.clear();
  support_deferred.clear();
  ```
  Update loop references from `seeds` to `support_seeds` and `deferred` to `support_deferred`.
- In `Grid::fracture_landing(int x, int y)`:
  Replace:
  ```cpp
  std::vector<int> lowest(span, -1);
  ```
  With:
  ```cpp
  fracture_lowest.assign(static_cast<size_t>(span), -1);
  ```
  And update references from `lowest` to `fracture_lowest`.

---

### Phase 2: Guard `piece_tag` Against Wraparound & Active Tag Collisions

**Problem:** In [`src/physics/grid.cpp`](file:///c:/Users/Ethan/Desktop/game/code/src/physics/grid.cpp), `next_piece_tag` increments linearly from 1 to 255. If 255 fractures occur while an existing falling piece is still in flight (`ticks > 0`), the new tag can collide with the in-flight piece, merging distinct bodies.

#### [MODIFY] [grid.h](file:///c:/Users/Ethan/Desktop/game/code/src/physics/grid.h)
- Add a private helper method declaration:
```cpp
    uint8_t alloc_piece_tag();
```

#### [MODIFY] [grid.cpp](file:///c:/Users/Ethan/Desktop/game/code/src/physics/grid.cpp)
- Implement `alloc_piece_tag()` to scan in-flight pieces in `pending_support` and guarantee tag uniqueness:
```cpp
uint8_t Grid::alloc_piece_tag() {
    for (int attempt = 0; attempt < 255; ++attempt) {
        uint8_t candidate = next_piece_tag;
        next_piece_tag = static_cast<uint8_t>(next_piece_tag + 1);
        if (next_piece_tag == 0) next_piece_tag = 1;

        bool in_use = false;
        for (int idx : pending_support) {
            if (cells[idx].ticks > 0 && cells[idx].piece_tag == candidate) {
                in_use = true;
                break;
            }
        }
        if (!in_use) return candidate;
    }
    return next_piece_tag; // Fallback if all 255 tags are simultaneously in flight
}
```
- In `Grid::fracture_landing()`, replace:
```cpp
    const uint8_t fresh = next_piece_tag;
    next_piece_tag = static_cast<uint8_t>(next_piece_tag + 1);
    if (next_piece_tag == 0) next_piece_tag = 1;
```
With:
```cpp
    const uint8_t fresh = alloc_piece_tag();
```

---

### Phase 3: Binary Division-Free `isqrt` for Tool Raymarching

**Problem:** In [`src/physics/tool.cpp`](file:///c:/Users/Ethan/Desktop/game/code/src/physics/tool.cpp), Newton-Raphson integer square root executes multiple `idiv` integer division instructions per call (`(x + v / x) / 2`).

#### [MODIFY] [tool.cpp](file:///c:/Users/Ethan/Desktop/game/code/src/physics/tool.cpp)
- Replace `isqrt` with a digit-by-digit binary square root algorithm using bitwise shifts and zero division instructions:
```cpp
// floor(sqrt(v)) for v >= 0, via digit-by-digit binary square root.
// Exact, deterministic, and division-free across all toolchains and architectures.
long long isqrt(long long v) {
    if (v <= 0) return 0;
    unsigned long long n = static_cast<unsigned long long>(v);
    unsigned long long root = 0;
    unsigned long long bit = 1ULL << 62; // The second-to-top bit of 64-bit int

    while (bit > n) bit >>= 2;
    while (bit != 0) {
        if (n >= root + bit) {
            n -= root + bit;
            root = (root >> 1) + bit;
        } else {
            root >>= 1;
        }
        bit >>= 2;
    }
    return static_cast<long long>(root);
}
```

---

### Phase 4: Zero-Allocation `depth_map` in `surface_plane`

**Problem:** In [`src/render/surface_plane.cpp`](file:///c:/Users/Ethan/Desktop/game/code/src/render/surface_plane.cpp), `depth_map` executes `std::vector<int> run(static_cast<size_t>(v.w), -1);` on every frame upload.

#### [MODIFY] [surface_plane.h](file:///c:/Users/Ethan/Desktop/game/code/src/render/surface_plane.h)
- Update `depth_map` declaration to accept an optional run buffer or document scratch reuse:
```cpp
void depth_map(const uint32_t* grid_pixels, int grid_w, int grid_h,
               const View& v, int* depth, int* run_scratch = nullptr);
```

#### [MODIFY] [surface_plane.cpp](file:///c:/Users/Ethan/Desktop/game/code/src/render/surface_plane.cpp)
- Update `depth_map` implementation to use `run_scratch` or a local stack buffer if within threshold:
```cpp
void depth_map(const uint32_t* grid_pixels, int grid_w, int grid_h,
               const View& v, int* depth, int* run_scratch) {
    if (!grid_pixels || !depth || v.w <= 0 || v.h <= 0) return;

    int start = v.view_y - DEPTH_END;
    if (start < 0) start = 0;

    std::vector<int> local_run;
    int* run = run_scratch;
    if (!run) {
        local_run.assign(static_cast<size_t>(v.w), -1);
        run = local_run.data();
    } else {
        std::fill(run, run + v.w, -1);
    }

    for (int y = start; y < v.view_y + v.h; ++y) {
        if (y < 0 || y >= grid_h) {
            if (y >= v.view_y) {
                int* row = depth + static_cast<size_t>(y - v.view_y) * v.w;
                for (int x = 0; x < v.w; ++x) row[x] = -1;
            }
            continue;
        }
        const uint32_t* src = grid_pixels + static_cast<size_t>(y) * grid_w;
        for (int x = 0; x < v.w; ++x) {
            const int gx = v.view_x + x;
            const bool matter = gx >= 0 && gx < grid_w && is_matter(src[gx]);
            int& r = run[x];
            if (!matter) r = -1;
            else if (y == start) r = DEPTH_END;
            else if (r < 0) r = 0;
            else if (r < DEPTH_END) r = r + 1;
        }
        if (y >= v.view_y) {
            int* row = depth + static_cast<size_t>(y - v.view_y) * v.w;
            for (int x = 0; x < v.w; ++x) row[x] = run[x];
        }
    }
}
```
- In `surface_plane::apply`:
  When calling `depth_map`, pass the last row of `scratch` (or a dedicated slice) when `v.h > 1`:
  ```cpp
  // scratch has v.w * v.h elements. The last row scratch + (v.h - 1) * v.w is only written at the end of depth_map.
  depth_map(grid_pixels, grid_w, grid_h, v, scratch);
  ```

---

### Phase 5: Architecture Documentation (Multi-Threaded Chunk Sweep)

#### [MODIFY] [ENGINEERING_NOTES.md](file:///c:/Users/Ethan/Desktop/game/code/ENGINEERING_NOTES.md)
- Under the **Performance and Scaling** section, document the 4-Phase Checkerboard (Red-Black) Chunk Sweep pattern for deterministic multi-threaded cellular simulation.

---

## Verification Plan

### Automated Tests
Run the entire headless test suite from the repository root:
```powershell
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
```

### Invariant Checks to Verify:
1. `sizeof(Element) <= 12` static assertion compiles cleanly.
2. `grid_test` passes all structural collapse, support resolution, and thermal checks.
3. `tool_test` passes all integer raymarching and dig range checks with the new binary `isqrt`.
4. `surface_plane_test` and `golden_frame_test` produce matching checksums.
5. All 19 tests in `ctest` report `Passed`.
