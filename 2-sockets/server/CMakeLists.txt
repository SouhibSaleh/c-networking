cmake_minimum_required(VERSION 4.2)
project(server C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_executable(server main.c)

find_package(Threads REQUIRED)
target_link_libraries(server PRIVATE Threads::Threads)
