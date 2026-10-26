cmake_minimum_required(VERSION 3.28)

set(cmake_git_version_dir
    ${CMAKE_CURRENT_LIST_DIR}
    CACHE INTERNAL "")

find_package(Git REQUIRED QUIET)

cmake_path(SET cmake_git_version_template_file_git NORMALIZE
           "${cmake_git_version_dir}/templates/git_version_generated.hpp.in")
set(cmake_git_version_template_file_git
    ${cmake_git_version_template_file_git}
    CACHE INTERNAL "Template file for git version generation")

if(NOT cmake_git_version_working_dir)
    set(cmake_git_version_working_dir
        ${PROJECT_SOURCE_DIR}
        CACHE INTERNAL "Working directory for git operations")
endif()

if(NOT EXISTS "${cmake_git_version_working_dir}")
    message(WARNING "CMakeGitVersion: Working directory does not exist: ${cmake_git_version_working_dir}")
    set(cmake_git_version_working_dir "${PROJECT_SOURCE_DIR}")
endif()

# git <args...> in the working directory: its trimmed standard output into <out>, its exit code into <rc>
function(_cmake_git_version_git out rc)
    execute_process(
        COMMAND ${GIT_EXECUTABLE} ${ARGN}
        WORKING_DIRECTORY "${cmake_git_version_working_dir}"
        RESULT_VARIABLE res
        OUTPUT_VARIABLE output
        ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)
    set(${out}
        "${output}"
        PARENT_SCOPE)
    set(${rc}
        ${res}
        PARENT_SCOPE)
endfunction()

# <var> made fit for a C++ string literal in the template, cut to <max> characters ("..." at the end); TABS turns tabs
# into spaces, SHELL backticks into ' and $ into \$, a `;` becomes \073. An empty value stays empty.
function(_cmake_git_version_escape var max)
    cmake_parse_arguments(PARSE_ARGV 2 E "TABS;SHELL" "" "")
    # every expansion quoted: an empty value (git commit --allow-empty-message) stays one empty argument, and a `;`
    # stays in the value instead of splitting it
    set(value "${${var}}")
    string(REPLACE "\"" "\\\"" value "${value}")
    string(REPLACE "\n" " " value "${value}")
    string(REPLACE "\r" " " value "${value}")
    if(E_TABS)
        string(REPLACE "\t" " " value "${value}")
    endif()
    if(E_SHELL)
        string(REPLACE "`" "'" value "${value}")
        string(REPLACE "$" "\\$" value "${value}")
    endif()
    string(LENGTH "${value}" length)
    if(length GREATER max)
        math(EXPR keep "${max} - 3")
        string(SUBSTRING "${value}" 0 ${keep} value)
        string(APPEND value "...")
    endif()
    # a `;` as the C++ octal escape \073 (three digits: the escape ends there whatever follows), after the cut so that
    # the cut never splits it: the state is a CMake list, where a raw `;` would split the value into two elements
    string(REPLACE ";" "\\073" value "${value}")
    set(${var}
        "${value}"
        PARENT_SCOPE)
endfunction()

_cmake_git_version_git(ignored git_check_result rev-parse --git-dir)
if(NOT git_check_result EQUAL 0)
    message(STATUS "CMakeGitVersion: Directory is not a git repository: ${cmake_git_version_working_dir}")
endif()

if(NOT cmake_git_version_out_dir)
    cmake_path(SET cmake_git_version_out_dir_temp NORMALIZE "${CMAKE_CURRENT_BINARY_DIR}/cmake_git_version_generated")
    set(cmake_git_version_out_dir
        ${cmake_git_version_out_dir_temp}
        CACHE INTERNAL "Output directory for generated files")
endif()

cmake_path(SET cmake_git_version_state_file NORMALIZE "${cmake_git_version_out_dir}/git-state")
set(cmake_git_version_state_file
    ${cmake_git_version_state_file}
    CACHE INTERNAL "State file for git version tracking")
cmake_path(SET cmake_git_version_out_file_git NORMALIZE
           "${cmake_git_version_out_dir}/cmake_git_version/git_version_generated.hpp")
set(cmake_git_version_out_file_git
    ${cmake_git_version_out_file_git}
    CACHE INTERNAL "Output file for git version header")

function(cmake_git_version_get_git_state _state)
    set(success "true")

    _cmake_git_version_git(hashvar res rev-parse --short HEAD)
    if(NOT res EQUAL 0)
        set(success "false")
        if(res EQUAL 128)
            message(STATUS "CMakeGitVersion: Not in a git repository or git repository not found")
            set(hashvar "no-git-repo")
        else()
            message(STATUS "CMakeGitVersion: Git rev-parse failed (exit code: ${res})")
            set(hashvar "git-error")
        endif()
    endif()

    _cmake_git_version_git(out res status --porcelain)
    set(dirty "false")
    if(NOT res EQUAL 0)
        set(success "false")
        if(res EQUAL 128)
            message(STATUS "CMakeGitVersion: Git status failed - not a git repository")
        else()
            message(STATUS "CMakeGitVersion: Git status failed (exit code: ${res})")
        endif()
    elseif(NOT "" STREQUAL "${out}")
        # untracked files ("?? ") do not make the tree dirty
        string(REPLACE "\n" ";" out ${out})
        list(FILTER out EXCLUDE REGEX "^\\?\\? ")
        list(LENGTH out out)
        if(NOT ${out} EQUAL 0)
            set(dirty "true")
        endif()
    endif()

    _cmake_git_version_git(subjectvar res log -n 1 --pretty=format:%s)
    if(NOT res EQUAL 0)
        set(success "false")
        if(res EQUAL 128)
            message(STATUS "CMakeGitVersion: No git commits found or repository is empty")
            set(subjectvar "no-commits")
        else()
            message(STATUS "CMakeGitVersion: Git log failed (exit code: ${res})")
            set(subjectvar "log-error")
        endif()
    else()
        _cmake_git_version_escape(subjectvar 200 TABS SHELL)
    endif()

    _cmake_git_version_git(commit_timevar res show --quiet --date=format-local:%Y-%m-%dT%H:%M:%S --format=%cd)
    if(NOT res EQUAL 0)
        set(success "false")
        if(res EQUAL 128)
            message(STATUS "CMakeGitVersion: Git show failed - no commits or invalid repository")
        else()
            message(STATUS "CMakeGitVersion: Git show failed (exit code: ${res})")
        endif()
        set(commit_timevar "0000-01-01T00:00:00")
    endif()

    _cmake_git_version_git(branchvar res rev-parse --abbrev-ref HEAD)
    if(NOT res EQUAL 0)
        set(success "false")
        if(res EQUAL 128)
            message(STATUS "CMakeGitVersion: Cannot determine git branch - not a git repository")
            set(branchvar "no-branch")
        else()
            message(STATUS "CMakeGitVersion: Git branch detection failed (exit code: ${res})")
            set(branchvar "unknown-branch")
        endif()
    endif()

    _cmake_git_version_git(tagvar res describe --tags --exact-match)
    if(NOT res EQUAL 0)
        set(tagvar "no-tag")
    else()
        _cmake_git_version_escape(tagvar 50)
    endif()

    _cmake_git_version_git(authorvar res log -1 --pretty=format:%an)
    if(NOT res EQUAL 0)
        set(success "false")
        if(res EQUAL 128)
            message(STATUS "CMakeGitVersion: Cannot get git author - no commits found")
            set(authorvar "no-author")
        else()
            message(STATUS "CMakeGitVersion: Git author extraction failed (exit code: ${res})")
            set(authorvar "unknown-author")
        endif()
    else()
        _cmake_git_version_escape(authorvar 100 TABS)
    endif()

    set(${_state}
        ${success}
        ${hashvar}
        ${dirty}
        "${subjectvar}"
        ${commit_timevar}
        "${branchvar}"
        "${tagvar}"
        "${authorvar}"
        PARENT_SCOPE)
endfunction()

function(cmake_git_version_check_git _state_changed _state)
    cmake_git_version_get_git_state(state)

    # quoted: an empty element (an empty subject) keeps its place in the list
    set(${_state}
        "${state}"
        PARENT_SCOPE)

    if(EXISTS ${cmake_git_version_state_file})
        file(READ ${cmake_git_version_state_file} old_state_contents)
        if(old_state_contents STREQUAL "${state}")
            set(${_state_changed}
                "false"
                PARENT_SCOPE)
            return()
        endif()
    endif()

    get_filename_component(state_file_dir "${cmake_git_version_state_file}" DIRECTORY)
    if(NOT EXISTS "${state_file_dir}")
        file(MAKE_DIRECTORY "${state_file_dir}")
        if(NOT EXISTS "${state_file_dir}")
            message(WARNING "CMakeGitVersion: Failed to create directory: ${state_file_dir}")
            set(${_state_changed}
                "false"
                PARENT_SCOPE)
            return()
        endif()
    endif()

    file(WRITE "${cmake_git_version_state_file}" "${state}")
    if(NOT EXISTS "${cmake_git_version_state_file}")
        message(WARNING "CMakeGitVersion: Failed to write state file: ${cmake_git_version_state_file}")
        set(${_state_changed}
            "false"
            PARENT_SCOPE)
        return()
    endif()
    set(${_state_changed}
        "true"
        PARENT_SCOPE)
endfunction()

if(CMAKE_GIT_VERSION_SUBPROCESS_RUNNING)
    cmake_git_version_check_git(did_change state)
    if(did_change)
        message(STATUS "CMakeGitVersion: Git state changed, regenerating version header")
        list(GET state 0 GEN_GIT_RETRIEVED_STATE)
        list(GET state 1 GEN_GIT_HEAD_SHA1)
        list(GET state 2 GEN_GIT_IS_DIRTY)
        list(GET state 3 GEN_GIT_SUBJECT)
        list(GET state 4 GEN_GIT_COMMIT_TIME)
        list(GET state 5 GEN_GIT_BRANCH)
        list(GET state 6 GEN_GIT_TAG)
        list(GET state 7 GEN_GIT_AUTHOR)

        get_filename_component(header_file_dir "${cmake_git_version_out_file_git}" DIRECTORY)
        if(NOT EXISTS "${header_file_dir}")
            file(MAKE_DIRECTORY "${header_file_dir}")
            if(NOT EXISTS "${header_file_dir}")
                message(FATAL_ERROR "CMakeGitVersion: Failed to create header output directory: ${header_file_dir}")
            endif()
        endif()

        configure_file(${cmake_git_version_template_file_git} ${cmake_git_version_out_file_git} @ONLY)

        if(NOT EXISTS "${cmake_git_version_out_file_git}")
            message(FATAL_ERROR "CMakeGitVersion: Failed to generate header file: ${cmake_git_version_out_file_git}")
        endif()
    endif()
else()
    add_custom_target(
        cmake_git_version_check_repository ALL
        DEPENDS ${cmake_git_version_template_file_git}
        BYPRODUCTS ${cmake_git_version_out_file_git} ${cmake_git_version_state_file}
        COMMAND
            ${CMAKE_COMMAND} -DCMAKE_GIT_VERSION_SUBPROCESS_RUNNING=TRUE
            -Dcmake_git_version_working_dir=${cmake_git_version_working_dir}
            -Dcmake_git_version_out_dir=${cmake_git_version_out_dir} -P ${CMAKE_CURRENT_LIST_FILE}
        COMMENT "Updating git version information")
endif()

function(cmake_git_version_add_headers_with_type target optimize_type)
    set(outdir ${CMAKE_CURRENT_BINARY_DIR}/cmake_git_version_generated)
    set(outdir_target ${outdir}/${target})
    set(outfile_target ${outdir_target}/cmake_git_version/project_version_generated.hpp)
    set(template_dir ${cmake_git_version_dir}/templates)

    include(FetchContent)

    FetchContent_GetProperties(cmake_git_version BINARY_DIR cmake_git_version_bindir)
    if(NOT "${cmake_git_version_bindir}" STREQUAL "")
        target_include_directories(${target} PRIVATE "${cmake_git_version_bindir}/cmake_git_version_generated")
    endif()

    target_include_directories(${target} PRIVATE ${outdir})
    target_include_directories(${target} PRIVATE ${outdir_target})
    target_include_directories(${target} PRIVATE ${cmake_git_version_out_dir})
    target_include_directories(${target} PRIVATE ${cmake_git_version_dir}/src)

    add_dependencies(${target} cmake_git_version_check_repository)

    string(TIMESTAMP timestamp "%Y-%m-%dT%H:%M:%S")

    set(GEN_TARGET_NAME "${target}")
    set(GEN_TARGET_OPTIMIZE_TYPE "${optimize_type}")
    set(GEN_TARGET_CMAKE_TIME "${timestamp}")

    set(GEN_PROJECT_NAME ${PROJECT_NAME})
    set(GEN_PROJECT_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
    set(GEN_PROJECT_VERSION_MINOR ${PROJECT_VERSION_MINOR})
    set(GEN_PROJECT_VERSION_PATCH ${PROJECT_VERSION_PATCH})
    configure_file(${template_dir}/project_version_generated.hpp.in ${outfile_target} @ONLY)
    set_property(
        DIRECTORY
        APPEND
        PROPERTY CMAKE_CONFIGURE_DEPENDS "${outfile_target}")
    set_property(
        DIRECTORY
        APPEND
        PROPERTY CMAKE_CONFIGURE_DEPENDS "${template_dir}/project_version_generated.hpp.in")
    set_property(
        DIRECTORY
        APPEND
        PROPERTY CMAKE_CONFIGURE_DEPENDS "${template_dir}/git_version_generated.hpp.in")
    set_property(
        TARGET ${target}
        APPEND
        PROPERTY ADDITIONAL_CLEAN_FILES ${outfile_target})
endfunction()

function(cmake_git_version_add_headers target)
    cmake_git_version_add_headers_with_type(${target} ${CMAKE_BUILD_TYPE})
endfunction()
