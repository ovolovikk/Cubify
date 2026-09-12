# Cubify : High performance voxel engine
![Language](https://img.shields.io/badge/language-C++-blue.svg)
![Language](https://img.shields.io/badge/language-HLSL-green.svg)
![Platform](https://img.shields.io/badge/platform-Windows-blue.svg)

Cubify is a high-performance voxel engine built with **C++20** and **DirectX 12**. The project focuses on technical efficiency and procedural generation, featuring unique worlds inspired by the *Interstellar* universe.

# Screenshots
**Minecraft**
![World1](screenshots/minecraft_3.png)
**Sector-R**
![World2](screenshots/sectorr_128_distance_0.png)
**Utopia**
![World3](screenshots/utopia_3.png)
**Void toggle**
![VoidToggle](screenshots/void_toggle.gif)
**Void Utopia**
![VoidWorld](screenshots/void_utopia_2.png)

## 🌌 Key Features

* **Procedural Worlds**: Advanced terrain generation using the **Factory pattern**.
* **Physics Engine**: Custom-built player physics featuring momentum, AABB collision detection, and a seamless free-cam toggle.
* **Dynamic Audio**: Implemented via \`miniaudio\`, supporting background music and state-dependent sound effects.
* **Config System**: Settings management through \`config.json\` with support for custom seeds, render distances, and UI scaling.

## Optimizations

### While developing Cubify I focused on implementing high-performance one-threaded game engine.

| Feature | Technical Impact |
| :--- | :--- |
| **Structured Buffers** | Utilizes **StructuredBuffer** SRVs for efficient chunk data management. |
| **Data Packing** | Optimized CPU-to-GPU transfer by bit-packing vertex for 300% less memory usage. |
| **Hybrid Culling** | Combined **Frustum Culling** (6-plane) and **Face Culling** to minimize draw calls. |
| **Texture Arrays** | Uses \`Texture2DArray\` to eliminate atlas bleeding and improve cache locality. |
| **PIMPL Idiom** | Enforces strict architectural separation for faster compilation and cleaner API. |

## Controls

| Key | Action |
| :--- | :--- |
| \`W, A, S, D\` | Movement |
| \`Space\` / \`L-Alt\` | Jump / Sprint |
| \`1-9\` | Choose specific block |
| \`Mouse Scroll\`| Cycle through every block |
| \`F1\` | Toggle Free-Cam / Player Mode |
| \`F3\` | Toggle Cursor visibility |
| \`F11\` | Toggle Fullscreen |
| \`ESC\` | Exit |

## Dependencies

They all built-in within external folder.

*   **GLFW**: Windowing and input.
*   **GLM**: Mathematics.
*   **FastNoiseLite**: Noise generation for terrain.
*   **nlohmann/json**: JSON parsing for configuration.
*   **miniaudio**: Audio playback.
*   **stb_image**: Texture loading.

## Build

**Requirements**: Visual Studio 2022 with C++ workload, CMake 3.21+

### Option 1: Run the build script in PowerShell
```powershell
git clone https://github.com/ovolovikk/Cubify.git
cd Cubify
scripts\build.ps1 Release
```

### Option 2: Manual build in cmd
```cmd
git clone https://github.com/ovolovikk/Cubify.git
cd Cubify
if not exist build mkdir build
cmake -S . -B build -G "Visual Studio 17 2022"
cmake --build build --config Release
```

The executable will be in `build/Release/Cubify.exe`
