cmake_minimum_required(VERSION 3.20)

project(Cubify VERSION 1.0 LANGUAGES CXX C)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(CUBIFY_STRICT_WARNINGS "Enable a strict warning set" OFF)

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

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
    "src/*.cpp" 
    "src/*.c"
)

#TODO: Return imgui

if(WIN32)
    set(WIN_RESOURCES "${CMAKE_SOURCE_DIR}/resources.rc")
endif()

add_executable(Cubify WIN32 ${SOURCES} ${WIN_RESOURCES})

target_precompile_headers(Cubify PRIVATE "${CMAKE_SOURCE_DIR}/src/PrecompilerHeader.hpp")

target_include_directories(Cubify PRIVATE 
    "${CMAKE_SOURCE_DIR}/src"
    "${CMAKE_SOURCE_DIR}/external"
)

if(CUBIFY_STRICT_WARNINGS AND MSVC)
    target_compile_options(Cubify PRIVATE
        /W4
        /permissive-
        /Zc:__cplusplus

        # these ignored because not i've written them
        /external:W0
        "/external:I${CMAKE_SOURCE_DIR}/external"
    )
endif()

target_link_libraries(Cubify PRIVATE
    glfw
    glm
)

if(WIN32)
    target_link_libraries(Cubify PRIVATE d3d12 dxgi dxguid d3dcompiler dxcompiler)
    file(TO_CMAKE_PATH "$ENV{ProgramFiles\(x86\)}" PROGRAM_FILES_X86)
    set(WINDOWS_KIT_BIN "${PROGRAM_FILES_X86}/Windows Kits/10/bin")

    if(CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION)
        set(DXC_SEARCH_PATHS "${WINDOWS_KIT_BIN}/${CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION}/x64")
    else()
        file(GLOB DXC_SEARCH_PATHS "${WINDOWS_KIT_BIN}/*/x64") # Used in non MSVC builds
    endif()

    find_file(DXC_COMPILER_DLL NAMES dxcompiler.dll PATHS ${DXC_SEARCH_PATHS} NO_DEFAULT_PATH)
    find_file(DXC_DXIL_DLL NAMES dxil.dll PATHS ${DXC_SEARCH_PATHS} NO_DEFAULT_PATH)

    if(NOT DXC_COMPILER_DLL OR NOT DXC_DXIL_DLL)
        message(WARNING
            "dxcompiler.dll / dxil.dll not found under ${WINDOWS_KIT_BIN}. "
            "Shaders will fail to compile at runtime.")
    endif()
endif()

set_target_properties(Cubify PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:Cubify>"
)

if(WIN32 AND DXC_COMPILER_DLL AND DXC_DXIL_DLL)
    add_custom_command(TARGET Cubify POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${DXC_COMPILER_DLL}" "${DXC_DXIL_DLL}"
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
