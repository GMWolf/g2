add_library(g2_scene_fbs INTERFACE)
target_include_directories(g2_scene_fbs INTERFACE gen)

add_library(g2_scene include/g2/scene.h src/scene.cpp)
target_include_directories(g2_scene PUBLIC include
        PRIVATE include/g2)
target_link_libraries(g2_scene PUBLIC g2_scene_fbs g2_core g2_render g2_assets g2_ecs)