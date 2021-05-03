add_library(g2_archive include/g2/archive/g2archive.h src/g2archive.cpp)
target_include_directories(g2_archive
        PRIVATE include/g2/archive
        PUBLIC include gen
)
target_link_libraries(g2_archive PUBLIC flatbuffers)