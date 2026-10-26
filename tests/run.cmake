# cmake -DSOURCE_DIR=<cmake_git_version> -DWORK_DIR=<dir> -DGIT=<git> -DGENERATOR=<gen> -P run.cmake Scratch
# repositories with fixed authors and dates (so fixed hashes); no test reads the real clock: the one configure-time
# stamp is checked for its shape only.
cmake_minimum_required(VERSION 3.28)
file(REMOVE_RECURSE "${WORK_DIR}")
file(MAKE_DIRECTORY "${WORK_DIR}")
set(EXPECTED "${CMAKE_CURRENT_LIST_DIR}/expected")
set(failures 0)

set(ENV{TZ} UTC)
set(ENV{GIT_CONFIG_GLOBAL} /dev/null)
set(ENV{GIT_CONFIG_NOSYSTEM} 1)
set(ENV{GIT_AUTHOR_NAME} "Test Author")
set(ENV{GIT_AUTHOR_EMAIL} "author@example.invalid")
set(ENV{GIT_COMMITTER_NAME} "Test Committer")
set(ENV{GIT_COMMITTER_EMAIL} "committer@example.invalid")
set(ENV{GIT_AUTHOR_DATE} "2001-02-03T04:05:06Z")
set(ENV{GIT_COMMITTER_DATE} "2001-02-03T04:05:06Z")

function(git dir)
    execute_process(
        COMMAND ${GIT} ${ARGN}
        WORKING_DIRECTORY "${dir}"
        RESULT_VARIABLE rc
        OUTPUT_QUIET
        ERROR_VARIABLE err)
    if(NOT rc EQUAL 0)
        message(FATAL_ERROR "git ${ARGN} in ${dir}: ${err}")
    endif()
endfunction()

function(new_repo dir)
    file(MAKE_DIRECTORY "${dir}")
    git("${dir}" init -q -b master)
    file(WRITE "${dir}/file.txt" "one\n")
    git("${dir}" add file.txt)
    git("${dir}" commit -q -m "first commit")
endfunction()

# runs the library's generation step (what the build's cmake_git_version_check_repository target runs) for <dir>
function(generate name dir)
    set(out "${WORK_DIR}/out/${name}")
    execute_process(
        COMMAND ${CMAKE_COMMAND} -DCMAKE_GIT_VERSION_SUBPROCESS_RUNNING=TRUE -Dcmake_git_version_working_dir=${dir}
                -Dcmake_git_version_out_dir=${out} -P ${SOURCE_DIR}/CMakeLists.txt
        RESULT_VARIABLE rc
        OUTPUT_VARIABLE log
        ERROR_VARIABLE log)
    set(header "${out}/cmake_git_version/git_version_generated.hpp")
    set(log_${name}
        "${log}"
        PARENT_SCOPE)
    if(NOT rc EQUAL 0)
        message(SEND_ERROR "${name}: generation failed (rc ${rc}):\n${log}")
        set(failures
            1
            PARENT_SCOPE)
        return()
    endif()
    if(UPDATE_EXPECTED)
        configure_file("${header}" "${EXPECTED}/${name}.txt" COPYONLY)
    endif()
    execute_process(COMMAND ${CMAKE_COMMAND} -E compare_files "${EXPECTED}/${name}.txt" "${header}"
                    RESULT_VARIABLE same)
    if(NOT same EQUAL 0)
        file(READ "${header}" got)
        message(SEND_ERROR "${name}: header differs from expected/${name}.txt:\n${got}")
        set(failures
            1
            PARENT_SCOPE)
    else()
        message(STATUS "${name}: as expected")
    endif()
endfunction()

set(R "${WORK_DIR}/repos")
# a work dir inside a checkout (a build tree) must not make not_a_repo part of that repository
set(ENV{GIT_CEILING_DIRECTORIES} "${R}")

file(MAKE_DIRECTORY "${R}/not_a_repo")
generate(not_a_repo "${R}/not_a_repo")

file(MAKE_DIRECTORY "${R}/no_commit")
git("${R}/no_commit" init -q -b master)
generate(no_commit "${R}/no_commit")

new_repo("${R}/clean")
generate(clean "${R}/clean")
# the same state again: the state file says so and the header is not written again
generate(clean "${R}/clean")
if(log_clean MATCHES "regenerating")
    message(SEND_ERROR "clean, second run: regenerated an unchanged state:\n${log_clean}")
    set(failures 1)
endif()

new_repo("${R}/dirty")
file(APPEND "${R}/dirty/file.txt" "two\n")
generate(dirty "${R}/dirty")

new_repo("${R}/untracked")
file(WRITE "${R}/untracked/new.txt" "untracked files do not make a tree dirty\n")
generate(untracked "${R}/untracked")

new_repo("${R}/tagged")
git("${R}/tagged" tag v1.2.3)
generate(tagged "${R}/tagged")

new_repo("${R}/detached")
file(APPEND "${R}/detached/file.txt" "two\n")
git("${R}/detached" commit -q -a -m "second commit")
git("${R}/detached" checkout -q HEAD~1)
generate(detached "${R}/detached")

new_repo("${R}/dir with spaces")
generate(spaces "${R}/dir with spaces")

new_repo("${R}/subject")
file(APPEND "${R}/subject/file.txt" "two\n")
string(REPEAT "x" 210 long)
# the message through a file: a `;` in a CMake argument would split it
file(WRITE "${WORK_DIR}/subject_message.txt" "quote \" dollar $HOME tick ` semicolon a;b tab\tend ${long}\n")
git("${R}/subject" commit -q -a -F "${WORK_DIR}/subject_message.txt")
generate(subject "${R}/subject")

# an empty subject (git commit --allow-empty-message): an empty Subject, not a failed generation
new_repo("${R}/empty_subject")
file(APPEND "${R}/empty_subject/file.txt" "two\n")
# the empty message through a file: an empty argument does not survive ARGN
file(WRITE "${WORK_DIR}/empty_message.txt" "")
git("${R}/empty_subject"
    commit
    -q
    -a
    --allow-empty-message
    -F
    "${WORK_DIR}/empty_message.txt")
generate(empty_subject "${R}/empty_subject")

new_repo("${R}/author")
file(APPEND "${R}/author/file.txt" "two\n")
string(REPEAT "y" 110 longname)
set(ENV{GIT_AUTHOR_NAME} "Name \"Nick\" ${longname}")
git("${R}/author" commit -q -a -m "long author")
set(ENV{GIT_AUTHOR_NAME} "Test Author")
generate(author "${R}/author")

# a consumer project: project_version_generated.hpp, the configure-time stamp masked after its shape is checked
new_repo("${R}/consumer")
file(COPY "${CMAKE_CURRENT_LIST_DIR}/consumer/" DESTINATION "${R}/consumer")
execute_process(
    COMMAND ${CMAKE_COMMAND} -S "${R}/consumer" -B "${WORK_DIR}/consumer_build" -G "${GENERATOR}"
            -DCMAKE_GIT_VERSION_DIR=${SOURCE_DIR}
    RESULT_VARIABLE rc
    OUTPUT_VARIABLE log
    ERROR_VARIABLE log)
set(project_header
    "${WORK_DIR}/consumer_build/cmake_git_version_generated/consumer_target/cmake_git_version/project_version_generated.hpp"
)
if(NOT rc EQUAL 0 OR NOT EXISTS "${project_header}")
    message(SEND_ERROR "consumer: configure failed (rc ${rc}):\n${log}")
    set(failures 1)
else()
    file(READ "${project_header}" text)
    set(stamp_re "CMakeTime\\{\"[0-9][0-9][0-9][0-9]-[0-9][0-9]-[0-9][0-9]T[0-9][0-9]:[0-9][0-9]:[0-9][0-9]\"\\}")
    if(NOT text MATCHES "${stamp_re}")
        message(SEND_ERROR "consumer: no CMakeTime stamp of the documented shape in:\n${text}")
        set(failures 1)
    endif()
    string(REGEX REPLACE "${stamp_re}" "CMakeTime{\"<configure time>\"}" text "${text}")
    file(WRITE "${WORK_DIR}/project_version_masked.hpp" "${text}")
    if(UPDATE_EXPECTED)
        file(WRITE "${EXPECTED}/project_version.txt" "${text}")
    endif()
    execute_process(COMMAND ${CMAKE_COMMAND} -E compare_files "${EXPECTED}/project_version.txt"
                            "${WORK_DIR}/project_version_masked.hpp" RESULT_VARIABLE same)
    if(NOT same EQUAL 0)
        message(
            SEND_ERROR "consumer: project_version_generated.hpp differs from expected/project_version.txt:\n${text}")
        set(failures 1)
    else()
        message(STATUS "consumer: as expected")
    endif()
endif()

if(failures)
    message(FATAL_ERROR "cmake_git_version: failures")
endif()
message(STATUS "cmake_git_version: all cases pass")
