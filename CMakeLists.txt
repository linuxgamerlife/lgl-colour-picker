cmake_minimum_required(VERSION 3.20)
project(lgl-colour-picker LANGUAGES C CXX)

include(CTest)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
add_subdirectory(gtk)
