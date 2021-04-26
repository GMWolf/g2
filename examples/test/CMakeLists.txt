add_executable(g2_test src/main.cpp)

target_link_libraries(g2_test PRIVATE g2_platform g2_gfx)

g2_add_pipeline(pipeline.json)
g2_add_pipeline(pipeline2.json)

g2_add_mesh(DamagedHelmet/DamagedHelmet.gltf mesh_helmet_LP_13930damagedHelmet)

add_custom_target(g2_test_pipelines DEPENDS pipeline.json.bin pipeline2.json.bin DamagedHelmet/DamagedHelmet.gltf.mesh_helmet_LP_13930damagedHelmet.bin)
add_dependencies(g2_test g2_test_pipelines)

g2_add_texture(DamagedHelmet/Default_albedo.jpg)

add_custom_target(textures DEPENDS DamagedHelmet/Default_albedo.jpg.ktx2)
add_dependencies(g2_test textures)