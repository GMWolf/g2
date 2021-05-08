#add_custom_command(OUTPUT bc7e/bc7e.obj
#        COMMAND ispc -g -O2 "${CMAKE_CURRENT_LIST_DIR}/bc7e/bc7e.ispc" -o "bc7e/bc7e.o" -h "bc7e/bc7e_ispc.h" --target=sse2,sse4,avx,avx2 --opt=fast-math --opt=disable-assertions
#        DEPENDS bc7e/bc7e.ispc
#        )


add_library(bc7e OBJECT
        bc7e/bc7e.ispc
        )

target_compile_options(bc7e PRIVATE --opt=fast-math --opt=disable-assertions)
set_target_properties(bc7e PROPERTIES ISPC_INSTRUCTION_SETS "sse2;sse4;avx;avx2")
set_target_properties(bc7e PROPERTIES POSITION_INDEPENDENT_CODE ON)