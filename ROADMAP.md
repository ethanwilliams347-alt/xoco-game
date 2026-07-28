# SLOP / Xoco Game - Roadmap

This document tracks the immediate next steps and the long-term vision for the project. For deeper lore and feature brainstorming, see the `notes/` directory.

## 🎯 Project Goals
- **Core:** A production-level, barebones application.
- **Engine:** Custom pixel art destructible environment running on its own cellular automata physics engine.
- **Performance:** Strict no-bloat philosophy. Must run on low-end PCs (Windows/macOS/Linux).
- **Architecture:** Keep design choices minimal, but architected to easily allow massive expansion later.

## 🟢 Current Status
- [x] Initial GitHub Repo and CMake build system setup.
- [x] Barebones C++/SDL2 pixel physics engine prototype built (Sand, Water, Wall interactions).

## 🔴 Short Term (Next Up)
*The immediate technical steps to make the prototype a "game".*
- [ ] **Player Character:** Implement a basic controllable player entity.
- [ ] **Player Physics:** Make the player fall with gravity and collide properly with the solid pixel elements.
- [ ] **Basic Interaction:** Allow the player to shoot or mine the destructible terrain.
- [ ] **More Elements:** Add more physics elements (e.g., Fire, Steam, Wood) to test the engine's extensibility.

## 🟡 Medium Term (Core Gameplay Loop)
*Tying the physics engine into the lore.*
- [ ] **Quantum Worlds:** Implement a portal/level generation system where the player enters a new "trial".
- [ ] **The Pet ML Agent:** Create the UI/system for the pet agent that "observes" the player.
- [ ] **Proof-of-Work Economy:** Build the core extraction loop (complete a task in the trial -> extract -> agent earns a coin).

## 🟣 Long Term (The "Ideal Systems" Vision)
*Massive scope expansions to be tackled only after the core loop is incredibly fun and stable.*
- [ ] Base building and criminal/black market activities.
- [ ] Complex simulated stock/crypto market.
- [ ] Factories, Mining, and Business Ownership.
- [ ] Passive developing society and reputation mechanics.
