# Cubify

This is a low-level high-optimized C++ application for deeping into Voxel Engine's. It provides **Minecraft** like view with self-painted textures.

## **Features**
*   **Camera**
    * Portable to move around in any direction using both mouse and **WASD, SHIFT, SPACE** keys.
    * Provided with flexible **aspect, FOV, speed and sensitivity**
*   **Texture Array**
    * Changed from Texture atlas to **Texture array** significally reducing artifacts.
*   **Rendering Optimization**
    * Implemented **Vertex Pulling** via **SSBOs** (Shader Storage Buffer Objects) for high-performance rendering reducing vertex count by 6 times.
    *   **Face Culling** to discard hidden faces. 
    *   **Dynamic Chunk Management** that efficiently loads and unloads chunks based on player position.

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