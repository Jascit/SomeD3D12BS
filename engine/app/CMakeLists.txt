cmake_minimum_required(VERSION 4.2)

add_library(App STATIC)
target_sources(App PUBLIC
        FILE_SET CXX_MODULES FILES
        _.cppm
)
target_link_libraries(App PUBLIC CompileFlagsAllShitTogether)
target_link_libraries(App PUBLIC
        D3D12
        Assets
        Platform
)
add_executable(SomeD3D12BS main.cpp)
target_link_libraries(SomeD3D12BS PRIVATE App)

