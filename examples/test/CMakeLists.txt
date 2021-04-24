add_executable(g2_test src/main.cpp)

target_link_libraries(g2_test PRIVATE g2_platform g2_gfx)

g2_add_pipeline(pipeline.json)
g2_add_pipeline(pipeline2.json)

add_custom_target(g2_test_pipelines DEPENDS pipeline.json.bin pipeline2.json.bin)
add_dependencies(g2_test g2_test_pipelines)


#
add_custom_command(
        OUTPUT OldWoodPlanks.ktx2
        COMMAND toktx --uastc 0 --uastc_rdo_l --zcmp 1 --genmipmap OldWoodPlanks.ktx2 ${CMAKE_CURRENT_LIST_DIR}/textures/TexturesCom_OldWoodPlanks_3.5x3.5_1K_albedo.png
        DEPENDS ./textures/TexturesCom_OldWoodPlanks_3.5x3.5_1K_albedo.png
        USES_TERMINAL
)

add_custom_target(textures DEPENDS OldWoodPlanks.ktx2)
add_dependencies(g2_test textures)