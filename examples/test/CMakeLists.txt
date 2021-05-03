add_executable(g2_test src/main.cpp)

target_link_libraries(g2_test PRIVATE g2_platform g2_gfx g2_assets)

g2_add_assets(g2_test_assets assets)