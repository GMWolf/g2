add_executable(g2_test src/main.cpp)

target_link_libraries(g2_test PRIVATE g2_platform g2_gfx g2_assets)

g2_add_pipeline(assets/pipeline.json)

g2_add_mesh(assets/DamagedHelmet/DamagedHelmet.gltf mesh_helmet_LP_13930damagedHelmet)

add_custom_target(g2_test_pipelines DEPENDS assets/pipeline.json.g2ppln assets/DamagedHelmet/DamagedHelmet.gltf.mesh_helmet_LP_13930damagedHelmet.g2mesh)
add_dependencies(g2_test g2_test_pipelines)

g2_add_texture(assets/DamagedHelmet/Default_albedo.jpg)
g2_add_texture(assets/DamagedHelmet/Default_normal.jpg)

g2_add_materials(assets/DamagedHelmet/DamagedHelmet.gltf)

add_custom_target(textures DEPENDS assets/DamagedHelmet/Default_albedo.jpg.ktx2 assets/DamagedHelmet/Default_normal.jpg.ktx2 assets/DamagedHelmet/DamagedHelmet.gltf.g2mat)
add_dependencies(g2_test textures)


g2_add_assets(g2_test_assets assets)