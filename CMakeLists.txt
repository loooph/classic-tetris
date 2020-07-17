cmake_minimum_required(VERSION 3.18)

project(classic_tetris
    VERSION 0.1
    DESCRIPTION "NES tetris rewrite in C++"
    LANGUAGES C CXX)

add_executable(tetris main.c)
target_link_libraries(tetris SDL2)
