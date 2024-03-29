add_subdirectory(shader_compiler)
add_subdirectory(gltf_compiler)
add_subdirectory(texture_compiler)
add_subdirectory(effect_compiler)


function(g2_add_assets target directory)

    file(GLOB_RECURSE GLTF_FILES RELATIVE ${CMAKE_CURRENT_LIST_DIR} CONFIGURE_DEPENDS "*.gltf")
    foreach(GLTF_FILE IN ITEMS ${GLTF_FILES})
        g2_add_gltf(${GLTF_FILE})
        list(APPEND ASSETS ${GLTF_FILE}.g2ar)
    endforeach()

    file(GLOB_RECURSE IMAGE_FILES RELATIVE ${CMAKE_CURRENT_LIST_DIR}} CONFIGURE_DEPENDS "*.jpg" "*.png")
    foreach(IMAGE_FILE IN ITEMS ${IMAGE_FILES})
        g2_add_texture(${IMAGE_FILE})
        list(APPEND ASSETS ${IMAGE_FILE}.g2img)
    endforeach()

    file(GLOB_RECURSE PIPELINE_FILES RELATIVE ${CMAKE_CURRENT_LIST_DIR}} CONFIGURE_DEPENDS "*.pipeline")
    foreach(PIPELINE_FILE IN ITEMS ${PIPELINE_FILES})
        g2_add_pipeline(${PIPELINE_FILE})
        list(APPEND ASSETS ${PIPELINE_FILE}.g2ppln)
    endforeach()

    file(GLOB_RECURSE FX_FILES RELATIVE ${CMAKE_CURRENT_LIST_DIR}} CONFIGURE_DEPENDS "*.fx")
    foreach(FX_FILE IN ITEMS ${FX_FILES})
        g2_add_effect(${FX_FILE})
        list(APPEND ASSETS ${FX_FILE}.g2fx)
    endforeach()

    add_custom_target(${target} DEPENDS "${ASSETS}")

endfunction()