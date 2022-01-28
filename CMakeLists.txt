cmake_minimum_required(VERSION 3.0)
project(Almost-15)

find_package(raylib 4.0 REQUIRED)

add_executable(${PROJECT_NAME} main.cpp)
target_link_libraries(${PROJECT_NAME} raylib)
