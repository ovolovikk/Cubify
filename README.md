# Cubify

A high-performance voxel engine written in C++17 and Modern OpenGL (4.3+).

## **Technical Highlights**

### **Rendering Pipeline**
*   **Programmable Vertex Pulling**: Uses **SSBOs** (Shader Storage Buffer Objects) to fetch vertex data directly on the GPU, bypassing traditional VBO overhead.
*   **Aggressive Data Packing**: Geometry is bit-packed into just **8 bytes per quad** (Position + Normal + Texture Layer), reducing memory bandwidth by 3x.
*   **Instanced Rendering**: Renders entire chunks in single draw calls using `glDrawArraysInstanced`.
*   **Texture Arrays**: Utilizes `GL_TEXTURE_2D_ARRAY` for efficient texture management without atlas bleeding artifacts.

### **Engine Architecture**
*   **Decoupled Design**: The Renderer is completely agnostic of game logic, following **SOLID** principles.
*   **Modular Structure**: Clean separation between `Core` (Window/Input), `Graphics` (OpenGL abstraction), and `World` (Voxel logic).

### **World Generation**
*   **Infinite Terrain**: Procedural generation powered by **FastNoiseLite**.
*   **Dynamic Streaming**: Efficient chunk loading and unloading based on player position.
*   **Greedy Meshing (Lite)**: Face culling eliminates hidden geometry between blocks and chunks.

## **Controls**
*   `W, A, S, D`: Move Camera
*   `LCTRL`: Sprint
*   `Mouse`: Look around
*   `ESC`: Exit

## **Dependencies**
*   **OpenGL 4.3+** (Core Profile)
*   **GLFW 3** (Windowing and Input)
*   **GLEW** (OpenGL Extension Wrangler)
*   **GLM** (OpenGL Mathematics)
*   **FastNoiseLite** (Noise generation)
*   **stb_image** (Image loading)

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
3.  Configure and build:
    ```bash
    cmake ..
    cmake --build .
    ```