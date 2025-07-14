cmake_minimum_required(VERSION 3.20.0)
project(qompiler LANGUAGES CXX C)

set(CMAKE_BUILD_WITH_INSTALL_NAME_DIR ON)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(mlir)