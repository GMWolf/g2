add_library(g2_core include/g2/core/Transform.h include/g2/core/core.h src/core.cpp)
target_include_directories(g2_core PUBLIC include
        PRIVATE include/g2/core)
target_link_libraries(g2_core PUBLIC glm g2_ecs)