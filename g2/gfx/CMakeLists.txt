add_library(g2_gfx
        include/g2/gfx_instance.h src/validation.h src/device.cpp src/device.h src/swapchain.cpp src/swapchain.h src/shader.cpp src/shader.h src/pipeline.cpp src/pipeline.h src/renderpass.cpp src/renderpass.h include/g2/viewport.h include/g2/command_encoder.h src/command_encoder.cpp)

target_sources(g2_gfx PRIVATE
        src/vk.h
        src/gfx_instance.cpp
        src/validation.cpp)

target_include_directories(g2_gfx PUBLIC include)
target_link_libraries(g2_gfx PRIVATE vulkan)
target_include_directories(g2_gfx PUBLIC gen )

target_link_libraries(g2_gfx PUBLIC g2_platform glm)