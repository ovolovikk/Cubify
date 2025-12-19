cmake_minimum_required(VERSION 3.20)

project(Cubify VERSION 1.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(GLEW_ROOT "C:/Users/shona/Documents/libs/glew-2.1.0" CACHE PATH "Path to GLEW root")
set(GLFW_ROOT "C:/Users/shona/Documents/libs/glfw-3.4.bin.WIN64" CACHE PATH "Path to GLFW root")
set(GLM_ROOT "C:/Users/shona/Documents/libs/glm-1.0.2" CACHE PATH "Path to GLM root")

list(APPEND CMAKE_PREFIX_PATH "${GLEW_ROOT}" "${GLFW_ROOT}" "${GLM_ROOT}")

find_package(GLEW REQUIRED)

find_package(glm QUIET)
if(NOT glm_FOUND)
    add_library(glm::glm INTERFACE IMPORTED)
    target_include_directories(glm::glm INTERFACE "${GLM_ROOT}")
endif()

find_package(glfw3 3.3 QUIET)
if(NOT glfw3_FOUND)
    add_library(glfw STATIC IMPORTED)
    set_target_properties(glfw PROPERTIES
        IMPORTED_LOCATION "${GLFW_ROOT}/lib-vc2022/glfw3.lib"
        INTERFACE_INCLUDE_DIRECTORIES "${GLFW_ROOT}/include"
    )
endif()

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS 
    "src/*.cpp" 
    "src/*.c"
)

add_executable(Cubify ${SOURCES})

target_include_directories(Cubify PRIVATE 
    src
    src/Helpers
    src/Helpers/imgui
)

target_link_libraries(Cubify PRIVATE
    GLEW::GLEW
    glfw
    glm::glm
    opengl32
)

# --- Post-Build ---
add_custom_command(TARGET Cubify POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${GLEW_ROOT}/bin/Release/x64/glew32.dll"
    "$<TARGET_FILE_DIR:Cubify>"
)

add_custom_command(TARGET Cubify POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_SOURCE_DIR}/shaders"
    "$<TARGET_FILE_DIR:Cubify>/shaders"
)

add_custom_command(TARGET Cubify POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_SOURCE_DIR}/textures"
    "$<TARGET_FILE_DIR:Cubify>/textures"
)
