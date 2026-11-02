cmake_minimum_required(VERSION 4.2)
project(SomeD3D12BS)

set(CMAKE_CXX_STANDARD 26)

set(CMAKE_CXX_FLAGS "" CACHE STRING "Clear CXX flags" FORCE)
set(CMAKE_CXX_FLAGS_DEBUG "" CACHE STRING "Clear CXX debug flags" FORCE)
set(CMAKE_CXX_FLAGS_RELEASE "" CACHE STRING "Clear CXX release flags" FORCE)
set(CMAKE_CXX_FLAGS_MINSIZEREL "" CACHE STRING "Clear CXX minsize flags" FORCE)
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "" CACHE STRING "Clear CXX relwithdebinfo flags" FORCE)

add_library(CompileFlagsAllShitTogether INTERFACE)
if (MSVC)
    target_compile_options(CompileFlagsAllShitTogether INTERFACE
            $<$<CONFIG:DEBUG>:
            /Zi
            /Od
            /W4
            /Oy-
            /RTC1
            /EHs-c-
            >
            $<$<CONFIG:RELEASE>:
            /O2
            /Ot
            /Oi
            /GL
            /W3
            /EHs-c-
            /fp:fast
            >
    )
    target_compile_definitions(CompileFlagsAllShitTogether INTERFACE
            $<$<CONFIG:RELEASE>: NDEBUG >
            $<$<CONFIG:DEBUG>: DEBUG >
    )
elseif (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(CompileFlagsAllShitTogether INTERFACE
            $<$<CONFIG:DEBUG>:
            -g
            -O0
            -fno-omit-frame-pointer
            -Wall -Wextra -Wpedantic
            -fno-inline
            -fno-exceptions
            >
            $<$<CONFIG:RELEASE>:
            -O3
            -march=native
            -mtune=native
            -ffast-math
            -funroll-loops
            -fstrict-aliasing
            -fno-exceptions
            >
    )
    target_compile_definitions(CompileFlagsAllShitTogether INTERFACE
            $<$<CONFIG:RELEASE>: NDEBUG >
            $<$<CONFIG:DEBUG>: DEBUG >
            $<$<CONFIG:DEBUG>: _DEBUG >
    )
else ()
    MESSAGE(FATAL_ERROR "Unsupported compiler: ${CMAKE_CXX_COMPILER_ID}")
endif ()

add_subdirectory(engine/core)
add_subdirectory(engine/D3D12)
add_subdirectory(engine/assets)
add_subdirectory(engine/platform)
add_subdirectory(engine/app)

add_subdirectory(tests)

