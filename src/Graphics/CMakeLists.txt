file(GLOB_RECURSE GRAPHICS_SOURCES CONFIGURE_DEPENDS "*.cpp")

add_library(CubifyGraphics STATIC ${GRAPHICS_SOURCES})

target_precompile_headers(CubifyGraphics PRIVATE "${CMAKE_SOURCE_DIR}/src/PrecompilerHeader.hpp")

target_include_directories(CubifyGraphics PUBLIC "${CMAKE_SOURCE_DIR}/src")
target_include_directories(CubifyGraphics SYSTEM PRIVATE "${Stb_INCLUDE_DIR}")

# The renderer interface exposes glm matrices
target_link_libraries(CubifyGraphics PUBLIC glm::glm)

if(WIN32)
    find_package(directx-headers CONFIG REQUIRED)
    target_link_libraries(CubifyGraphics PRIVATE Microsoft::DirectX-Headers)
    target_link_libraries(CubifyGraphics PRIVATE d3d12 dxgi dxguid)

    # GLOBAL, so the executable in the parent directory can link through to it
    find_package(winpixevent CONFIG REQUIRED GLOBAL)
    target_link_libraries(CubifyGraphics PRIVATE Microsoft::WinPixEventRuntime)
    target_compile_definitions(CubifyGraphics PRIVATE $<$<CONFIG:RelWithDebInfo>:USE_PIX>)

    find_package(directx-dxc CONFIG REQUIRED)
    set(SHADER_DIR "${CMAKE_SOURCE_DIR}/shaders/dx12")
    set(SHADER_OUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/shaders/$<CONFIG>")
    file(GLOB SHADER_INCLUDES CONFIGURE_DEPENDS "${SHADER_DIR}/*.hlsli")

    function(cubify_compile_shader name profile entry)
        set(output "${SHADER_OUT_DIR}/${name}.h")
        add_custom_command(
            OUTPUT "${output}"
            COMMAND "${CMAKE_COMMAND}" -E make_directory "${SHADER_OUT_DIR}"
            COMMAND "${DIRECTX_DXC_TOOL}" "${SHADER_DIR}/${name}.hlsl"
                -T ${profile} -E ${entry} -I "${SHADER_DIR}"
                "$<$<CONFIG:Debug>:-Zi;-Qembed_debug;-Od>"
                "$<$<NOT:$<CONFIG:Debug>>:-Qstrip_reflect;-O3>"
                -Vn "g_${name}" -Fh "${output}"
            DEPENDS "${SHADER_DIR}/${name}.hlsl" ${SHADER_INCLUDES}
            COMMENT "Compiling shader ${name}.hlsl"
            COMMAND_EXPAND_LISTS
            VERBATIM
        )
        target_sources(CubifyGraphics PRIVATE "${output}")
    endfunction()

    cubify_compile_shader(vertex_shader vs_6_0 VSMain)
    cubify_compile_shader(pixel_shader ps_6_0 PSMain)
    target_include_directories(CubifyGraphics PRIVATE "${SHADER_OUT_DIR}")
endif()
