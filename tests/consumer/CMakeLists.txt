# A project that takes the headers as a firmware does (include() of the library's CMakeLists.txt).
cmake_minimum_required(VERSION 3.28)
project(
    consumer
    VERSION 1.2.3
    LANGUAGES CXX)
include(${CMAKE_GIT_VERSION_DIR}/CMakeLists.txt)
add_executable(consumer_target main.cpp)
cmake_git_version_add_headers_with_type(consumer_target size)
