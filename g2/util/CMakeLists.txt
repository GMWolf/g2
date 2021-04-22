add_library(g2_util src/file.cpp include/g2/file.h include/g2/fbutil.h src/fbutil.cpp)
target_include_directories(g2_util PUBLIC include
        PRIVATE include/g2)