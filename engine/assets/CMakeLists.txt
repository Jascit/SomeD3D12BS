cmake_minimum_required(VERSION 4.2)

add_library(Assets STATIC)
target_link_libraries(Assets PRIVATE CompileFlagsAllShitTogether)

target_sources(Assets PUBLIC
        FILE_SET CXX_MODULES FILES
        ObjParser.cppm)

target_link_libraries(Assets PRIVATE Core)
