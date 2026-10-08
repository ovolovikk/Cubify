file(GLOB_RECURSE GRAPHICS_SOURCES CONFIGURE_DEPENDS "*.cpp")

add_library(CubifyGraphics STATIC ${GRAPHICS_SOURCES})

target_precompile_headers(CubifyGraphics PRIVATE "${CMAKE_SOURCE_DIR}/src/PrecompilerHeader.hpp")

target_include_directories(CubifyGraphics
    PUBLIC "${CMAKE_SOURCE_DIR}/src"
    PRIVATE "${CMAKE_SOURCE_DIR}/external"
)

# The renderer interface exposes glm matrices
target_link_libraries(CubifyGraphics PUBLIC glm)
target_link_libraries(CubifyGraphics PRIVATE Cubify::Logging)

if(WIN32)
    target_link_libraries(CubifyGraphics PRIVATE d3d12 dxgi dxguid d3dcompiler dxcompiler)

    # GLOBAL, so the executable in the parent directory can link through to it
    find_package(winpixevent CONFIG REQUIRED GLOBAL)
    target_link_libraries(CubifyGraphics PRIVATE Microsoft::WinPixEventRuntime)
    target_compile_definitions(CubifyGraphics PRIVATE $<$<CONFIG:RelWithDebInfo>:USE_PIX>)

    file(TO_CMAKE_PATH "$ENV{ProgramFiles\(x86\)}" PROGRAM_FILES_X86)
    set(WINDOWS_KIT_BIN "${PROGRAM_FILES_X86}/Windows Kits/10/bin")

    if(CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION)
        set(DXC_SEARCH_PATHS "${WINDOWS_KIT_BIN}/${CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION}/x64")
    else()
        file(GLOB DXC_SEARCH_PATHS "${WINDOWS_KIT_BIN}/*/x64") # Used in non MSVC builds
    endif()

    find_file(DXC_COMPILER_DLL NAMES dxcompiler.dll PATHS ${DXC_SEARCH_PATHS} NO_DEFAULT_PATH)
    find_file(DXC_DXIL_DLL NAMES dxil.dll PATHS ${DXC_SEARCH_PATHS} NO_DEFAULT_PATH)

    if(DXC_COMPILER_DLL AND DXC_DXIL_DLL)
        set(CUBIFY_GRAPHICS_RUNTIME_DLLS "${DXC_COMPILER_DLL}" "${DXC_DXIL_DLL}" PARENT_SCOPE)
    else()
        message(WARNING
            "dxcompiler.dll / dxil.dll not found under ${WINDOWS_KIT_BIN}. "
            "Shaders will fail to compile at runtime.")
    endif()
endif()
