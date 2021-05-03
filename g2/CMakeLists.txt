
add_compile_options(-Werror)

add_subdirectory(util)
add_subdirectory(core)
add_subdirectory(assets)
add_subdirectory(platform)
add_subdirectory(gfx)

add_subdirectory(pipeline_tools/shader_compiler)
add_subdirectory(pipeline_tools/mesh_compiler)
add_subdirectory(pipeline_tools/texture_compiler)
add_subdirectory(pipeline_tools/material_compiler)
add_subdirectory(pipeline_tools/pipeline_build_generator)