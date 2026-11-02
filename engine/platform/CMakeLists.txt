cmake_minimum_required(VERSION 4.2)

add_library(Platform STATIC)
target_link_libraries(Platform PRIVATE CompileFlagsAllShitTogether)

target_sources(Platform PUBLIC
        FILE_SET CXX_MODULES FILES
        Window.cppm)

target_link_libraries(Platform PRIVATE Core)
