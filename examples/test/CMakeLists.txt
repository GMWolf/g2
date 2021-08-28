add_executable(g2_test src/main.cpp src/camera.h)

target_link_libraries(g2_test PRIVATE g2_platform g2_gfx g2_assets g2_render g2_ecs)

g2_add_assets(g2_test_assets assets)

add_dependencies(g2_test g2_test_assets)