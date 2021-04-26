add_executable(g2_mesh_compiler src/mesh_compiler.cpp)
target_link_libraries(g2_mesh_compiler PRIVATE cgltf g2_gfx_fbs flatbuffers)


function(g2_add_mesh input mesh)
    add_custom_command(
            OUTPUT ${input}.${mesh}.bin
            COMMAND g2_mesh_compiler ARGS ${CMAKE_CURRENT_LIST_DIR}/${input} ${mesh} ${input}.${mesh}.bin
            DEPENDS "${input}" g2_mesh_compiler
            DEPFILE ${input}.${mesh}.bin.d
            USES_TERMINAL
    )
endfunction()