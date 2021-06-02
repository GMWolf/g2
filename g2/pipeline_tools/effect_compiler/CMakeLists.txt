add_executable(g2_effect_compiler src/effect_compiler.cpp)
target_link_libraries(g2_effect_compiler PRIVATE g2_gfx_fbs flatbuffers rapidjson g2_util)

function(g2_add_effect input)
    add_custom_command(
            OUTPUT ${input}.g2fx
            COMMAND g2_effect_compiler ARGS  ${CMAKE_CURRENT_LIST_DIR}/${input} ${input}.g2fx
            DEPENDS "${input}" g2_effect_compiler
            USES_TERMINAL
    )
endfunction()