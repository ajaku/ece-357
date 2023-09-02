cmake_minimum_required(VERSION 3.12)
project(MyStdioLib VERSION 1.0.0)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_BUILD_TYPE RelWithDebInfo)

add_library(
    my_stdio
    meow/my_stdio.c
    meow/my_stdio.h
)

add_executable(meow meow/meow.c)

target_link_libraries(meow PRIVATE my_stdio)
