add_executable(g2_material_compiler src/material_compiler.cpp)

target_link_libraries(g2_material_compiler PRIVATE cgltf g2_gfx_fbs flatbuffers)

function(g2_add_materials input)
    add_custom_command(
            OUTPUT ${input}.g2mat
            COMMAND g2_material_compiler ARGS ${CMAKE_CURRENT_LIST_DIR}/${input} ${input}.g2mat
            DEPENDS "${input}" g2_material_compiler
            USES_TERMINAL
    )
endfunction()