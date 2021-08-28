add_library(g2_render src/render_system.cpp include/g2/render/render.h)
target_include_directories(g2_render
        PUBLIC include
        PRIVATE include/g2/render)

target_link_libraries(g2_render PUBLIC g2_ecs g2_gfx)
