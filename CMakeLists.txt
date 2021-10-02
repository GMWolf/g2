cmake_minimum_required(VERSION 3.19)
project(g2 C CXX ISPC)

set(CMAKE_CXX_STANDARD 20)

#setup global defs
add_compile_definitions(GLM_CLIP_CONTROL_LH_BIT GLM_FORCE_DEPTH_ZERO_TO_ONE)


add_subdirectory(thirdparty)

add_subdirectory(g2)

add_subdirectory(examples)