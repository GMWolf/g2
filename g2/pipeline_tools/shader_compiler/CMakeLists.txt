add_executable(g2_shader_compiler src/main.cpp)


target_link_libraries(g2_shader_compiler PRIVATE shaderc spirv-cross-cpp flatbuffers g2_gfx_fbs rapidjson g2_util)


function(g2_add_pipeline input)
    add_custom_command(
            OUTPUT ${input}.bin
            COMMAND g2_shader_compiler ARGS ${CMAKE_CURRENT_LIST_DIR}/${input} ${input}.bin
            DEPENDS "${input}" g2_shader_compiler
            DEPFILE ${input}.bin.d
            USES_TERMINAL
    )
endfunction()