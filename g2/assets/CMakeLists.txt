add_library(g2_assets include/g2/assets/asset_registry.h src/asset_registry.cpp)
target_include_directories(g2_assets PRIVATE include/g2/assets
        PUBLIC include)