# Tests of cmake_git_version: `cmake -S tests -B <dir> -G Ninja && ctest --test-dir <dir> --output-on-failure`
cmake_minimum_required(VERSION 3.28)
project(cmake_git_version_tests LANGUAGES NONE)

enable_testing()
find_package(Git REQUIRED)

# The generated headers for scratch repositories in every git state (expected/*.txt), the state file that skips an
# unchanged regeneration, and a consumer project's project_version_generated.hpp (its configure time masked).
add_test(
    NAME cmake_git_version_headers
    COMMAND ${CMAKE_COMMAND} -DSOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}/.. -DWORK_DIR=${CMAKE_CURRENT_BINARY_DIR}/work
            -DGIT=${GIT_EXECUTABLE} -DGENERATOR=${CMAKE_GENERATOR} -P ${CMAKE_CURRENT_SOURCE_DIR}/run.cmake)
