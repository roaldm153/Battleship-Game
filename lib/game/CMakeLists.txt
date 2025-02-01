add_library(
    game 
    game.hpp
    game.cpp
)

target_include_directories(game PUBLIC &{PROJECT_SOURCE_DIR}/lib)
target_link_libraries(game PRIVATE stream)