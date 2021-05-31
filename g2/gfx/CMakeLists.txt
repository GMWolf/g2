
add_library(g2_gfx_fbs INTERFACE)
target_include_directories(g2_gfx_fbs INTERFACE gen)


add_library(g2_gfx
        include/g2/gfx_instance.h
        src/validation.h
        src/device.cpp
        src/device.h
        src/swapchain.cpp
        src/swapchain.h
        src/shader.cpp
        src/shader.h
        src/pipeline.cpp
        src/pipeline.h
        src/renderpass.cpp
        src/renderpass.h
        include/g2/viewport.h
        src/vma_impl.cpp
        src/mesh.cpp
        src/mesh.h
        src/Buffer.cpp
        src/Buffer.h
        src/descriptors.cpp
        src/descriptors.h
        src/image.cpp
        src/image.h
        src/upload.cpp
        src/upload.h
        src/material.cpp
        src/material.h )

target_sources(g2_gfx PRIVATE
        src/gfx_instance.cpp
        src/validation.cpp)

target_include_directories(g2_gfx PUBLIC include)
target_link_libraries(g2_gfx PRIVATE vulkan vma g2_util libzstd_static)
target_link_libraries(g2_gfx PUBLIC g2_platform glm g2_gfx_fbs g2_assets)