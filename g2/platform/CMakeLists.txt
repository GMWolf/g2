add_library(g2_platform
        include/g2/application.h
        )

target_include_directories(g2_platform PUBLIC include)

target_sources(g2_platform PRIVATE
        desktop/application.cpp)

target_link_libraries(g2_platform PRIVATE glfw vulkan)
target_link_libraries(g2_platform PUBLIC glm)