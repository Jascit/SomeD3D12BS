cmake_minimum_required(VERSION 4.2)

add_library(Core STATIC diagnostic/log.cpp)
target_link_libraries(Core PRIVATE CompileFlagsAllShitTogether)

target_sources(Core PUBLIC
        FILE_SET CXX_MODULES FILES
        diagnostic/log.cppm
        diagnostic/last_error.cppm
        Singleton.cppm)
