cmake_minimum_required(VERSION 3.10)
cmake_policy(SET CMP0074 NEW) # Fixes GLEW warning
cmake_policy(SET CMP0144 NEW) # Fixes GLM warning
project(Cubify)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# -----------------------------------------------------------------------------
# WINDOWS CONFIGURATION
# -----------------------------------------------------------------------------
if(WIN32)
     set(GLEW_ROOT "C:/Users/shona/Documents/libs/glew-2.1.0") 
     set(GLFW_ROOT "C:/Users/shona/Documents/libs/glfw-3.4.bin.WIN64")
     set(GLM_ROOT  "C:/Users/shona/Documents/libs/glm-1.0.2")

    if(DEFINED GLEW_ROOT)
        list(APPEND CMAKE_PREFIX_PATH "${GLEW_ROOT}")
    endif()
    if(DEFINED GLM_ROOT)
        list(APPEND CMAKE_PREFIX_PATH "${GLM_ROOT}")
        # GLM often has the config file in a subdirectory like cmake/glm
        list(APPEND CMAKE_PREFIX_PATH "${GLM_ROOT}/cmake/glm")
    endif()
endif()

# Find packages
find_package(GLEW REQUIRED)

# Handle GLM
find_package(glm QUIET)
if(NOT glm_FOUND)
    # Fallback: GLM is header-only, so we can just point to the include directory
    message(STATUS "GLM package not found via find_package, using manual include path...")
    if(EXISTS "${GLM_ROOT}/glm")
        add_library(glm::glm INTERFACE IMPORTED)
        target_include_directories(glm::glm INTERFACE "${GLM_ROOT}")
        message(STATUS "Found GLM manually at ${GLM_ROOT}")
    else()
        message(FATAL_ERROR "Could not find GLM in ${GLM_ROOT}")
    endif()
endif()

# Handle GLFW manually since binaries don't always have config files
find_package(glfw3 3.3 QUIET)
if(NOT glfw3_FOUND AND WIN32)
    message(STATUS "GLFW3 package not found via find_package, trying manual paths...")
    if(EXISTS "${GLFW_ROOT}/lib-vc2022/glfw3.lib")
        add_library(glfw STATIC IMPORTED)
        set_target_properties(glfw PROPERTIES
            IMPORTED_LOCATION "${GLFW_ROOT}/lib-vc2022/glfw3.lib"
            INTERFACE_INCLUDE_DIRECTORIES "${GLFW_ROOT}/include"
        )
        message(STATUS "Found GLFW3 manually at ${GLFW_ROOT}")
    else()
        message(FATAL_ERROR "Could not find GLFW3 library in ${GLFW_ROOT}/lib-vc2022/glfw3.lib")
    endif()
else()
    # If found via find_package (e.g. on macOS or if config exists)
    if(NOT TARGET glfw)
        add_library(glfw ALIAS glfw3) # Create alias if needed
    endif()
endif()

add_executable(Cubify main.cpp)

# Link libraries
target_link_libraries(Cubify
    PRIVATE
    GLEW::GLEW
    glfw
    glm::glm
)

# Copy DLLs on Windows
if(WIN32)
    add_custom_command(TARGET Cubify POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${GLEW_ROOT}/bin/Release/x64/glew32.dll"
        $<TARGET_FILE_DIR:Cubify>
        COMMENT "Copying glew32.dll to output directory"
    )
endif()

# For macOS specifically, we often need to link the OpenGL framework
if(APPLE)
    find_package(OpenGL REQUIRED)
    target_link_libraries(Cubify PRIVATE OpenGL::GL)
else()
    # On Windows/Linux, GLEW/GLFW usually handle the OpenGL linking, 
    # but sometimes explicit linking is needed depending on the driver.
    target_link_libraries(Cubify PRIVATE opengl32)
endif()
