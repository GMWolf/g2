add_executable(g2_pipeline_build_gen src/pipeline_build_gen.cpp src/ninja.cpp src/ninja.h)
target_link_libraries(g2_pipeline_build_gen PRIVATE cgltf)


target_compile_definitions(g2_pipeline_build_gen PRIVATE
        TOKTX_PATH="$<TARGET_FILE:toktx>"
        G2MESH_PATH="$<TARGET_FILE:g2_mesh_compiler>"
        G2SHADER_PATH="$<TARGET_FILE:g2_shader_compiler>"
        G2MAT_PATH="$<TARGET_FILE:g2_material_compiler>"
        )

function(g2_add_assets target input)
    add_custom_command(
            OUTPUT  ${input}/build.ninja
            COMMAND g2_pipeline_build_gen ARGS ${CMAKE_CURRENT_LIST_DIR}/${input} ${input}
            DEPENDS "${input}" g2_pipeline_build_gen
            USES_TERMINAL
    )

    add_custom_target(${target}
            COMMAND cd ${input} && ninja
            DEPENDS ${input}/build.ninja
    )
endfunction()