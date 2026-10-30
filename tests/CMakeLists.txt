cmake_minimum_required(VERSION 4.2)

include(FetchContent)
FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG v3.16.0
)
FetchContent_MakeAvailable(Catch2)

add_library(AllTestsTogether OBJECT
        d3d12/stream.cpp)
target_link_libraries(AllTestsTogether PRIVATE
        Catch2)
target_link_libraries(AllTestsTogether PRIVATE
        AllShitTogether)

add_executable(tests tests.cpp)
target_link_libraries(tests PRIVATE
        AllTestsTogether
        Catch2::Catch2WithMain)

set_target_properties(tests PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/tests_output"
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/tests_output/modules"
        ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/lib/tests_output")