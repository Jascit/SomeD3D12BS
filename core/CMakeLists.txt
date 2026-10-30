cmake_minimum_required(VERSION 4.2)

add_library(AllShitTogether STATIC diagnostic/log.cpp)
target_link_libraries(AllShitTogether PUBLIC CompileFlagsAllShitTogether)

target_sources(AllShitTogether PUBLIC
        FILE_SET CXX_MODULES FILES
        diagnostic/log.cppm
        diagnostic/last_error.cppm
        D3D12/RootSignature.cppm
        D3D12/Factory.cppm
        D3D12/commands/commands.cppm
        D3D12/Device.cppm
        D3D12/utils.cppm
        D3D12/SwapChain.cppm
        D3D12/DescriptorHeap.cppm
        D3D12/PipelineStateObject.cppm
        D3D12/shaders.cppm
        D3D12/ComResource.cppm
        D3D12/InputElementLayout.cppm
        D3D12/CommitedResource.cppm
        D3D12/ObjParser.cppm
        Singleton.cppm
        Window.cppm)

target_link_libraries(AllShitTogether PUBLIC d3d12.lib dxgi.lib dxcompiler.lib dxguid.lib)

if (CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION)
    set(WINDOWS_KITS_BIN "C:/Program Files (x86)/Windows Kits/10/bin/${CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION}/x64")
endif ()

if (NOT EXISTS "${WINDOWS_KITS_BIN}/dxcompiler.dll")
    file(GLOB SDK_VERSIONS RELATIVE "C:/Program Files (x86)/Windows Kits/10/bin" "C:/Program Files (x86)/Windows Kits/10/bin/10.*")
    if (SDK_VERSIONS)
        list(SORT SDK_VERSIONS COMPARE NATURAL ORDER DESCENDING)
        list(GET SDK_VERSIONS 0 LATEST_SDK_VERSION)
        set(WINDOWS_KITS_BIN "C:/Program Files (x86)/Windows Kits/10/bin/${LATEST_SDK_VERSION}/x64")
    endif ()
endif ()

message(STATUS "Auto-detected Windows SDK DXC path: ${WINDOWS_KITS_BIN}")


add_executable(SomeD3D12BS main.cpp)
target_link_libraries(SomeD3D12BS AllShitTogether)
add_custom_command(TARGET AllShitTogether POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${WINDOWS_KITS_BIN}/dxcompiler.dll"
        "$<TARGET_FILE_DIR:SomeD3D12BS>/dxcompiler.dll"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${WINDOWS_KITS_BIN}/dxil.dll"
        "$<TARGET_FILE_DIR:SomeD3D12BS>/dxil.dll"
        COMMENT "Copying DXC DLLs automatically from Windows SDK..."
)
