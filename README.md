# 3D Ecosystem & Seasonal Transition Simulation

This is a 3D computer graphics project built in **C++** and **OpenGL** that generates a dynamic natural ecosystem. The scene features continuous, real-time transitions between winter and spring alongside a full day-night cycle, centered around an animated tree inspired by the Whomping Willow.

> **Demo:** A demonstration video is available [here](https://drive.google.com/file/d/10V3lpbyFGNIPHK2wxbRvAI12hkexRb05/view).

---

## Features

* **Animated Central Tree:** Periodically shakes with realistic branch inertia to shed snow. Features procedural snow accumulation in winter and instanced foliage that sways in the wind, detaches, and settles on the ground as seasons shift.
* **Instanced Snowfall:** Renders **50,000** camera-facing billboard snowflakes using GPU instancing and modulo-based particle recycling, complete with real-time wind controls.
* **Procedural Terrain:** A Perlin-noise heightmap grid with a custom fragment shader that dynamically blends rock, grass, and slope-dependent snow accumulation with wind-driven UV warping.
* **Dynamic Day-Night Cycle:** An enclosing sky dome that interpolates between dawn, day, dusk, and night color palettes, paired with an orbiting light source transitioning between the Sun and the Moon.

## Controls & Interaction

* **Camera Movement:** Use the **keyboard** to navigate across the terrain (automatically adjusted to maintain a constant height above ground within scene bounds).
* **Environment Controls:** Use the on-screen **UI sliders** to adjust wind intensity along the X and Y axes in real time.

## Getting Started

### Prerequisites
* A C++ compiler (GCC, Clang, or MSVC)
* CMake (or your preferred build system)
* OpenGL, GLFW, GLAD/GLEW, and GLM

### Build & Run
```bash
git clone https://github.com/Arky-dev/Projet-Info-Graphique-3D.git
cd projet-info-graphique-3d
mkdir build && cd build
cmake ..
make
```

## Authors & Acknowledgments

* **Authors:** Paul Avérous & Sol Mingo
* **3D Assets:** Base tree mesh sourced from [Sketchfab](https://sketchfab.com/); snowflake sprite created in GIMP.
* **AI Assistance:** Generative AI (Gemini) was used during development for debugging, procedural noise generation, leaf orientation coefficients, and sky transition brainstorming.
