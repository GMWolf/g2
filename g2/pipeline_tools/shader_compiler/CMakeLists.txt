add_executable(g2_shader_compiler src/main.cpp src/module.h src/module.cpp src/reflection.h src/reflection.cpp)


target_link_libraries(g2_shader_compiler PRIVATE
        shaderc
        spirv-cross-cpp
        flatbuffers
        g2_gfx_fbs
        rapidjson
        g2_util)


function(g2_add_pipeline input)
    add_custom_command(
            OUTPUT ${input}.g2ppln
            COMMAND g2_shader_compiler ARGS ${CMAKE_CURRENT_LIST_DIR}/${input} ${input}.g2ppln
            DEPENDS "${input}" g2_shader_compiler
            DEPFILE ${input}.g2ppln.d
            USES_TERMINAL
    )
endfunction()