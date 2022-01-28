cmake_minimum_required(VERSION 3.0)
project(Almost-15)

if (EMSCRIPTEN)
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -s USE_GLFW=3 -s ASSERTIONS=1 -s WASM=1")
  set(CMAKE_EXECUTABLE_SUFFIX ".html")
endif ()

find_package(raylib 4.0 REQUIRED)

add_executable(${PROJECT_NAME} main.cpp)
target_link_libraries(${PROJECT_NAME} raylib)
