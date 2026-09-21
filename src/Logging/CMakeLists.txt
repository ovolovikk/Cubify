
add_library(CubifyLogging STATIC
            src/Log.cpp
            include/Log.hpp
            src/LogScope.cpp
            include/LogScope.hpp)

add_library(Cubify::Logging ALIAS CubifyLogging)

target_include_directories(CubifyLogging PUBLIC include)

target_link_libraries(CubifyLogging PRIVATE PrecompiledHeader)
