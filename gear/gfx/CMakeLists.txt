add_library(g2_gfx
        include/g2/gfx_instance.h src/validation.h src/physical_device.cpp src/physical_device.h)

target_sources(g2_gfx PRIVATE
        src/vk.h
        src/gfx_instance.cpp
        src/validation.cpp)

target_include_directories(g2_gfx PUBLIC include)
target_link_libraries(g2_gfx PRIVATE vulkan)