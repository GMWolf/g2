add_executable(g2_texture_compiler src/texture_compiler.cpp)
target_link_libraries(g2_texture_compiler PRIVATE g2_gfx_fbs flatbuffers bc7e stb libzstd_static)

function(g2_add_texture input)
    add_custom_command(
            OUTPUT ${input}.g2img
            COMMAND g2_texture_compiler ARGS  ${CMAKE_CURRENT_LIST_DIR}/${input} ${input}.g2img
            DEPENDS "${input}" g2_texture_compiler
            USES_TERMINAL
    )
endfunction()