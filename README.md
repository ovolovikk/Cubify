# Cubify

This is a low-level high-optimized C++ application for deeping into Voxel Engine's. It provides **Minecraft** like view with self-painted textures.

## **Key Features**
*   **High-Performance Core**: Built from scratch using C++17 and Modern OpenGL (4.3+).
*   **Procedural Terrain**: Infinite world generation powered by **FastNoiseLite**.
*   **Free-Roaming Camera**: Smooth 6-DOF movement with adjustable FOV, speed, and sensitivity.

## **Advanced Optimizations**
*   **Vertex Pulling Architecture**
    *   Replaced traditional VBOs with **Programmable Vertex Pulling** using **SSBOs** (Shader Storage Buffer Objects).
    *   Allows the GPU to fetch vertex data directly, bypassing fixed-function overhead.
*   **Aggressive Data Packing**
    *   Implemented bit-level compression for mesh data.
    *   Position (X, Y, Z), Texture Layer, and Normal Index are packed into just **8 bytes per face** (down from 24 bytes).
    *   Achieves a **3x reduction in memory bandwidth**, significantly boosting performance on memory-constrained GPUs.
*   **Smart Face Culling**
    *   **Inter-Chunk Culling**: Intelligently hides faces between adjacent chunks to ensure zero overdraw for internal geometry.
    *   **Back-Face Culling**: Discards faces pointing away from the camera.
*   **Instanced Rendering**
    *   Utilizes `glDrawArraysInstanced` to render entire chunks in a single draw call, drastically reducing CPU-GPU communication overhead.
*   **Texture Arrays**
    *   Utilizes `GL_TEXTURE_2D_ARRAY` to eliminate texture bleeding artifacts common in texture atlases and simplify shader logic.
*   **Dynamic Chunk Management**
    *   Efficiently loads and unloads chunks based on player position to manage memory usage.

## **Dependencies**
*   **OpenGL 4.3+** (Core Profile)
*   **GLFW 3** (Windowing and Input)
*   **GLEW** (OpenGL Extension Wrangler)
*   **GLM** (OpenGL Mathematics)
*   **FastNoiseLite** (Noise generation for terrain)
*   **stb_image** (Image loading)

## **Building**
The project uses **CMake** for building.

### **Prerequisites**
*   C++17 compatible compiler (MSVC, GCC, Clang)
*   CMake 3.10+

### **Instructions**
1.  Clone the repository:
    ```bash
    git clone https://github.com/ovolovikk/Cubify.git
    cd Cubify
    ```
2.  Create a build directory:
    ```bash
    mkdir build
    cd build
    ```
3.  Configure the project:
    *   **Note:** You may need to adjust the library paths in `CMakeLists.txt` if your libraries are not in standard locations or the hardcoded paths.
    ```bash
    cmake ..
    ```
4.  Build:
    ```bash
    cmake --build .
    ```
5.  Run the executable:
    ```bash
    ./Cubify
    ```

## **Controls**
| Key | Action |
| :--- | :--- |
| **W, A, S, D** | Move Camera |
| **Space** | Fly Up |
| **Shift** | Fly Down |
| **Mouse** | Look around |
| **ESC** | Close application |

## **License**
This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.