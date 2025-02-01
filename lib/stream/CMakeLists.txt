add_library(
    stream 
    stream.hpp
    stream.cpp
)

target_include_directories(stream PUBLIC ${PROJECT_SOURCE_DIR}/lib)
target_link_libraries(stream PRIVATE game)