add_library(g2_ecs include/g2/ecs/registry.h include/g2/ecs/type.h src/registry.cpp include/g2/ecs/query.h src/query.cpp)

target_include_directories(g2_ecs
        PUBLIC include
        PRIVATE include/g2/ecs)