# Cubify

A high-performance voxel engine written in C++17 and Modern OpenGL.

## Features

*   **High Performance**: 1500+ FPS using modern rendering techniques (SSBOs, Face Culling).
*   **Infinite World**: Procedurally generated terrain using FastNoiseLite.
*   **Player Physics**: Custom physics engine with AABB collision detection, gravity, and momentum.
*   **Movement**: Smooth WASD movement, sprinting, jumping, and air control.
*   **Debug Tools**: Toggleable Free-Cam mode for inspecting the world.

## Controls

*   `W, A, S, D`: Move
*   `Space`: Jump
*   `Left Alt`: Sprint
*   `Mouse`: Look
*   `F1`: Toggle Free-Cam / Player Mode
*   `ESC`: Exit

## Dependencies

*   OpenGL 4.3+, GLFW, GLEW, GLM
*   FastNoiseLite, stb_image

## Build

```bash
mkdir build && cd build
cmake ..
cmake --build .
```