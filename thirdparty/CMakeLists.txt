set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_USE_WAYLAND OFF CACHE BOOL "" FORCE)
set(GLFW_VULKAN_STATIC OFF CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS ON CACHE BOOL "" FORCE)
add_subdirectory(glfw)

add_subdirectory(glm)

#add_subdirectory(volk)

add_library(vma INTERFACE)
target_include_directories(vma INTERFACE VulkanMemoryAllocator/include)

add_subdirectory(SPIRV-Headers)

set(SPIRV_SKIP_EXECUTABLES ON)
set(SPIRV_WERROR OFF)
add_subdirectory(SPIRV-Tools)


set(SPIRV_CROSS_ENABLE_C_API OFF)
set(SPIRV_CROSS_ENABLE_MSL OFF)
set(SPIRV_CROSS_ENABLE_TESTS OFF)
set(SPIRV_CROSS_CLI OFF)
add_subdirectory(SPIRV-Cross)

set(ENABLE_SPVREMAPPER OFF)
set(ENABLE_GLSLANG_BINARIES OFF)
set(ENABLE_GLSLANG_JS OFF)
set(ENABLE_OPT OFF)
set(ENABLE_CTEST OFF)
add_subdirectory(glslang)
set(SHADERC_SKIP_TESTS ON CACHE BOOL "" FORCE)
add_subdirectory(shaderc)


add_library(rapidjson INTERFACE)
target_include_directories(rapidjson INTERFACE rapidjson/include)

set(KTX_FEATURE_TESTS OFF)
set(KTX_FEATURE_KTX1 OFF)
set(KTX_FEATURE_VULKAN OFF)

add_subdirectory(KTX-Software)

add_library(cgltf INTERFACE)
target_include_directories(cgltf INTERFACE cgltf)

include(stb.cmake)
include(bc7e.cmake)

add_subdirectory(zstd/build/cmake)
target_compile_options(libzstd_static PRIVATE "-O3") #always optimize