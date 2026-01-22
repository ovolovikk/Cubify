cmake_minimum_required(VERSION 3.20)

project(Cubify VERSION 1.0 LANGUAGES CXX C)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(GLEW_ROOT "${CMAKE_SOURCE_DIR}/external/glew")
set(GLFW_ROOT "${CMAKE_SOURCE_DIR}/external/glfw")
set(GLM_ROOT  "${CMAKE_SOURCE_DIR}/external/glm")

list(APPEND CMAKE_PREFIX_PATH "${GLEW_ROOT}" "${GLFW_ROOT}" "${GLM_ROOT}")

add_library(glew SHARED IMPORTED)
set_target_properties(glew PROPERTIES
    IMPORTED_IMPLIB "${GLEW_ROOT}/lib/Release/x64/glew32.lib"
    IMPORTED_LOCATION "${GLEW_ROOT}/bin/Release/x64/glew32.dll"
    INTERFACE_INCLUDE_DIRECTORIES "${GLEW_ROOT}/include"
)

add_library(glfw STATIC IMPORTED)
set_target_properties(glfw PROPERTIES
    IMPORTED_LOCATION "${GLFW_ROOT}/lib-vc2022/glfw3.lib"
    INTERFACE_INCLUDE_DIRECTORIES "${GLFW_ROOT}/include"
)

add_library(glm INTERFACE)
target_include_directories(glm INTERFACE "${GLM_ROOT}")


file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS 
    "src/*.cpp" 
    "src/*.c"
)

set(IMGUI_DIR "${CMAKE_SOURCE_DIR}/external/imgui")
set(IMGUI_SOURCES
    "${IMGUI_DIR}/imgui.cpp"
    "${IMGUI_DIR}/imgui_draw.cpp"
    "${IMGUI_DIR}/imgui_tables.cpp"
    "${IMGUI_DIR}/imgui_widgets.cpp"
    "${IMGUI_DIR}/imgui_impl_glfw.cpp"
    "${IMGUI_DIR}/imgui_impl_opengl3.cpp"
)

add_executable(Cubify ${SOURCES} ${IMGUI_SOURCES})

target_include_directories(Cubify PRIVATE 
    "${CMAKE_SOURCE_DIR}/src"
    "${CMAKE_SOURCE_DIR}/external"
    "${CMAKE_SOURCE_DIR}/external/imgui"
    "${CMAKE_SOURCE_DIR}/external/imgui/backends"
)

target_link_libraries(Cubify PRIVATE
    glew
    glfw
    glm
    opengl32
)
add_custom_command(TARGET Cubify POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${GLEW_ROOT}/bin/Release/x64/glew32.dll"
    "$<TARGET_FILE_DIR:Cubify>"
)

add_custom_command(TARGET Cubify POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${CMAKE_SOURCE_DIR}/config.json"
    "$<TARGET_FILE_DIR:Cubify>"
)

add_custom_command(TARGET Cubify POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_SOURCE_DIR}/shaders"
    "$<TARGET_FILE_DIR:Cubify>/shaders"
)

add_custom_command(TARGET Cubify POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_SOURCE_DIR}/assets"
    "$<TARGET_FILE_DIR:Cubify>/assets"
)