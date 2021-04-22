add_executable(g2_test src/main.cpp)

target_link_libraries(g2_test PRIVATE g2_platform g2_gfx)

g2_add_pipeline(pipeline.json)
g2_add_pipeline(pipeline2.json)

add_custom_target(g2_test_pipelines DEPENDS pipeline.json.bin pipeline2.json.bin)
add_dependencies(g2_test g2_test_pipelines)