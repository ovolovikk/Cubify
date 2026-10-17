cmake_minimum_required(VERSION 3.25)

# Has to be set before project()
if(NOT CMAKE_TOOLCHAIN_FILE)
    include(FetchContent)
    FetchContent_Declare(vcpkg
        GIT_REPOSITORY https://github.com/microsoft/vcpkg.git
        GIT_TAG 2026.07.29
        GIT_SHALLOW TRUE
    )
    FetchContent_MakeAvailable(vcpkg)
    set(CMAKE_TOOLCHAIN_FILE "${vcpkg_SOURCE_DIR}/scripts/buildsystems/vcpkg.cmake" CACHE FILEPATH "")
endif()

set(VCPKG_TARGET_TRIPLET "x64-windows-static-md" CACHE STRING "")

project(Cubify VERSION 1.0 LANGUAGES CXX C)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(CUBIFY_STRICT_WARNINGS "Enable a strict warning set" OFF)

find_package(glfw3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(Stb REQUIRED)
find_path(MINIAUDIO_INCLUDE_DIR miniaudio.h REQUIRED)

include(FetchContent)
FetchContent_Declare(fastnoiselite
    URL https://raw.githubusercontent.com/Auburn/FastNoiseLite/bae5a7c25fbf9482a010a77576598deee51411af/C/FastNoiseLite.h
    URL_HASH SHA256=8ed686eff7476303f6c06cc1ec7bdb4a86fe4d35240c1bd20f8c6a5ff69e74b5
    DOWNLOAD_NO_EXTRACT TRUE
)
FetchContent_MakeAvailable(fastnoiselite)

add_subdirectory(src/Graphics)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
    "src/*.cpp" 
    "src/*.c"
)
# Built as the CubifyGraphics library
list(FILTER SOURCES EXCLUDE REGEX "/src/Graphics/")

#TODO: Return imgui

if(WIN32)
    set(WIN_RESOURCES "${CMAKE_SOURCE_DIR}/resources.rc")
endif()

add_executable(Cubify WIN32 ${SOURCES} ${WIN_RESOURCES})

target_precompile_headers(Cubify PRIVATE "${CMAKE_SOURCE_DIR}/src/PrecompilerHeader.hpp")

target_include_directories(Cubify PRIVATE "${CMAKE_SOURCE_DIR}/src")
target_include_directories(Cubify SYSTEM PRIVATE
    "${Stb_INCLUDE_DIR}"
    "${MINIAUDIO_INCLUDE_DIR}"
    "${fastnoiselite_SOURCE_DIR}"
)

if(CUBIFY_STRICT_WARNINGS AND MSVC)
    foreach(target Cubify CubifyGraphics)
        target_compile_options(${target} PRIVATE
            /W4
            /permissive-
            /Zc:__cplusplus

            # SYSTEM includes (vcpkg, FetchContent) are not mine, so no warnings there
            /external:W0
        )
    endforeach()
endif()

target_link_libraries(Cubify PRIVATE
    glfw
    glm::glm
    nlohmann_json::nlohmann_json
    CubifyGraphics
)

set_target_properties(Cubify PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:Cubify>"
)

add_custom_command(TARGET Cubify POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${CMAKE_SOURCE_DIR}/config.json"
    "$<TARGET_FILE_DIR:Cubify>"
)

add_custom_command(TARGET Cubify POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_SOURCE_DIR}/assets"
    "$<TARGET_FILE_DIR:Cubify>/assets"
)
