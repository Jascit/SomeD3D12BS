cmake_minimum_required(VERSION 4.2)

add_library(D3D12 STATIC)
target_link_libraries(D3D12 PRIVATE CompileFlagsAllShitTogether)

target_sources(D3D12 PUBLIC
        FILE_SET CXX_MODULES FILES
        context/Factory.cppm
        context/Device.cppm
        context/Context.cppm
        context/CommandQueue.cppm
        context/Fence.cppm

        resources/DepthStencil.cppm
        resources/RenderTarget.cppm
        resources/resources.cppm
        resources/ResourceFactory.cppm
        resources/Texture2D.cppm
        resources/UploadBuffer.cppm
        resources/UploadManager.cppm

        pipeline/shaders.cppm
        pipeline/pipeline.cppm
        pipeline/RootSignature.cppm
        pipeline/PipelineStateObject.cppm
        pipeline/InputElementLayout.cppm
        pipeline/ShaderCompiler.cppm

        frame/FrameManager.cppm
        frame/CommandList.cppm

        descriptors/DescriptorHeap.cppm

        SwapChain.cppm
        ComResource.cppm
        utils.cppm
        PRIVATE
        context/Context.cpp
        resources/resources.cpp
        pipeline/pipeline.cpp
        frame/FrameManager.cpp
)

target_link_libraries(D3D12 PRIVATE d3d12.lib dxgi.lib dxcompiler.lib dxguid.lib)
target_link_libraries(D3D12 PRIVATE Core)

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

add_custom_command(TARGET D3D12 POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${WINDOWS_KITS_BIN}/dxcompiler.dll"
        "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/dxcompiler.dll"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${WINDOWS_KITS_BIN}/dxil.dll"
        "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/dxil.dll"
        COMMENT "Copying DXC DLLs automatically from Windows SDK..."
)
