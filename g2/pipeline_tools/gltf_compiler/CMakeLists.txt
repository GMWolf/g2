add_executable(g2_gltf_compiler src/mesh_compiler.cpp)
target_link_libraries(g2_gltf_compiler PRIVATE cgltf g2_gfx_fbs flatbuffers g2_archive)


function(g2_add_gltf input)
    add_custom_command(
            OUTPUT ${input}.g2ar
            COMMAND g2_gltf_compiler ARGS ${CMAKE_CURRENT_LIST_DIR}/${input} ${input}.g2ar
            DEPENDS "${input}" g2_gltf_compiler
            DEPFILE ${input}.g2ar.d
            USES_TERMINAL
    )
endfunction()