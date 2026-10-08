cmake_minimum_required(VERSION 3.25)

# Has to be set before project(). Dependencies come from vcpkg.json
if(NOT CMAKE_TOOLCHAIN_FILE AND DEFINED ENV{VCPKG_ROOT})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")
endif()

project(Cubify VERSION 1.0 LANGUAGES CXX C)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(CUBIFY_STRICT_WARNINGS "Enable a strict warning set" OFF)

add_library(PrecompiledHeader INTERFACE)
target_include_directories(PrecompiledHeader INTERFACE "${CMAKE_SOURCE_DIR}/src")
target_precompile_headers(PrecompiledHeader INTERFACE "${CMAKE_SOURCE_DIR}/src/PrecompilerHeader.hpp")

set(GLFW_ROOT "${CMAKE_SOURCE_DIR}/external/glfw")
set(GLM_ROOT  "${CMAKE_SOURCE_DIR}/external/glm")

list(APPEND CMAKE_PREFIX_PATH "${GLFW_ROOT}" "${GLM_ROOT}")

add_library(glfw STATIC IMPORTED)
set_target_properties(glfw PROPERTIES
    IMPORTED_LOCATION "${GLFW_ROOT}/lib-vc2022/glfw3.lib"
    INTERFACE_INCLUDE_DIRECTORIES "${GLFW_ROOT}/include"
)

add_library(glm INTERFACE)
target_include_directories(glm INTERFACE "${GLM_ROOT}")

add_subdirectory(src/Logging)
add_subdirectory(src/Graphics)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
    "src/Core/*.cpp"
    "src/Math/*.cpp"
    "src/Player/*.cpp"
    "src/Sound/*.cpp"
    "src/Utils/*.cpp"
    "src/World/*.cpp"
)
list(APPEND SOURCES "${CMAKE_SOURCE_DIR}/src/main.cpp")

#TODO: Return imgui

if(WIN32)
    set(WIN_RESOURCES "${CMAKE_SOURCE_DIR}/resources.rc")
endif()

add_executable(Cubify WIN32 ${SOURCES} ${WIN_RESOURCES})

target_include_directories(Cubify PRIVATE 
    "${CMAKE_SOURCE_DIR}/src"
    "${CMAKE_SOURCE_DIR}/external"
)

if(CUBIFY_STRICT_WARNINGS AND MSVC)
    foreach(target Cubify CubifyGraphics)
        target_compile_options(${target} PRIVATE
            /W4
            /permissive-
            /Zc:__cplusplus

            # these ignored because not i've written them
            /external:W0
            "/external:I${CMAKE_SOURCE_DIR}/external"
        )
    endforeach()
endif()

target_link_libraries(Cubify PRIVATE
    PrecompiledHeader
    glfw
    glm
    Cubify::Logging
    CubifyGraphics
)

set_target_properties(Cubify PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:Cubify>"
)

if(CUBIFY_GRAPHICS_RUNTIME_DLLS)
    add_custom_command(TARGET Cubify POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CUBIFY_GRAPHICS_RUNTIME_DLLS}
        "$<TARGET_FILE_DIR:Cubify>"
    )
endif()

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

add_custom_command(TARGET Cubify POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_SOURCE_DIR}/tests"
    "$<TARGET_FILE_DIR:Cubify>/tests"
)
