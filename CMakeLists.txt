set(cmake_git_version_dir
    ${CMAKE_CURRENT_LIST_DIR}
    CACHE INTERNAL "")

find_package(Git REQUIRED)

set(cmake_git_version_template_file_git
    ${cmake_git_version_dir}/templates/git_version_generated.hpp.in)

if(NOT cmake_git_version_working_dir)
  set(cmake_git_version_working_dir ${PROJECT_SOURCE_DIR})
endif()

if(NOT cmake_git_version_out_dir)
  set(cmake_git_version_out_dir
      ${CMAKE_CURRENT_BINARY_DIR}/cmake_git_version_generated)
endif()

set(cmake_git_version_state_file ${cmake_git_version_out_dir}/git-state)
set(cmake_git_version_out_file_git
    ${cmake_git_version_out_dir}/cmake_git_version/git_version_generated.hpp)

function(cmake_git_version_get_git_state _state)
  set(success "true")
  execute_process(
    COMMAND ${GIT_EXECUTABLE} rev-parse --short HEAD
    WORKING_DIRECTORY ${cmake_git_version_working_dir}
    RESULT_VARIABLE res
    OUTPUT_VARIABLE hashvar
    ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)
  if(NOT res EQUAL 0)
    set(success "false")
    set(hashvar "not-found")
  endif()

  execute_process(
    COMMAND ${GIT_EXECUTABLE} status --porcelain
    WORKING_DIRECTORY ${cmake_git_version_working_dir}
    RESULT_VARIABLE res
    OUTPUT_VARIABLE out
    ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)
  if(NOT res EQUAL 0)
    set(success "false")
    set(dirty "false")
  else()
    if(NOT "" STREQUAL "${out}")
      string(REPLACE "\n" ";" out ${out})
      list(FILTER out EXCLUDE REGEX "^\\?\\? ")
      list(LENGTH out out)
      if(NOT ${out} EQUAL 0)
        set(dirty "true")
      else()
        set(dirty "false")
      endif()
    else()
      set(dirty "false")
    endif()
  endif()

  execute_process(
    COMMAND ${GIT_EXECUTABLE} log -n 1 --pretty=format:%s
    WORKING_DIRECTORY ${cmake_git_version_working_dir}
    RESULT_VARIABLE res
    OUTPUT_VARIABLE subjectvar
    ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)
  if(NOT res EQUAL 0)
    set(success "false")
    set(subjectvar "not-found")
  endif()

  execute_process(
    COMMAND ${GIT_EXECUTABLE} show --quiet --date=format-local:%Y-%m-%dT%H:%M:%S
            --format=%cd
    WORKING_DIRECTORY ${cmake_git_version_working_dir}
    RESULT_VARIABLE res
    OUTPUT_VARIABLE commit_timevar
    ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)
  if(NOT res EQUAL 0)
    set(success "false")
    set(commit_timevar "0000-01-01T00:00:00")
  endif()

  set(${_state}
      ${success} ${hashvar} ${dirty} "${subjectvar}" ${commit_timevar}
      PARENT_SCOPE)
endfunction()

function(cmake_git_version_check_git _state_changed _state)
  cmake_git_version_get_git_state(state)

  set(${_state}
      ${state}
      PARENT_SCOPE)

  if(EXISTS ${cmake_git_version_state_file})
    file(READ ${cmake_git_version_state_file} old_sate_contents)
    if(old_state_contents STREQUAL "${state}")
      set(${_state_changed}
          "false"
          PARENT_SCOPE)
      return()
    endif()
  endif()

  file(WRITE "${cmake_git_version_state_file}" "${state}")
  set(${_state_changed}
      "true"
      PARENT_SCOPE)
endfunction()

if(CMAKE_GIT_VERSION_SUBPROCESS_RUNNING)
  cmake_git_version_check_git(did_change state)
  if(did_change)
    list(GET state 0 GEN_GIT_RETRIEVED_STATE)
    list(GET state 1 GEN_GIT_HEAD_SHA1)
    list(GET state 2 GEN_GIT_IS_DIRTY)
    list(GET state 3 GEN_GIT_SUBJECT)
    list(GET state 4 GEN_GIT_COMMIT_TIME)
    configure_file(${cmake_git_version_template_file_git}
                   ${cmake_git_version_out_file_git} @ONLY)
    set_property(
      DIRECTORY
      APPEND
      PROPERTY CMAKE_CONFIGURE_DEPENDS ${cmake_git_version_out_file_git})
  endif()
else()
  add_custom_target(
    cmake_git_version_check_repository ALL
    DEPENDS ${cmake_git_version_template_file_git}
    BYPRODUCTS ${cmake_git_version_out_file_git} ${cmake_git_version_state_file}
    COMMAND
      ${CMAKE_COMMAND} -DCMAKE_GIT_VERSION_SUBPROCESS_RUNNING=TRUE
      -Dcmake_git_version_working_dir=${cmake_git_version_working_dir}
      -Dcmake_git_version_out_dir=${cmake_git_version_out_dir} -P
      ${CMAKE_CURRENT_LIST_FILE})
endif()

function(target_add_version_headers target optimize_type)
  set(outdir ${CMAKE_CURRENT_BINARY_DIR}/cmake_git_version_generated)
  set(outdir_target ${outdir}/${target})
  set(outfile_target
      ${outdir_target}/cmake_git_version/project_version_generated.hpp)
  set(template_dir ${cmake_git_version_dir}/templates)

  FetchContent_GetProperties(cmake_git_version
                             BINARY_DIR cmake_git_version_bindir)
  if(NOT "${cmake_git_version_bindir}" STREQUAL "")
    target_include_directories(
      ${target}
      PRIVATE "${cmake_git_version_bindir}/cmake_git_version_generated")
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
  configure_file(${template_dir}/project_version_generated.hpp.in
                 ${outfile_target} @ONLY)
  set_property(
    DIRECTORY
    APPEND
    PROPERTY CMAKE_CONFIGURE_DEPENDS "${outfile_target}")
  set_property(
    TARGET ${target}
    APPEND
    PROPERTY ADDITIONAL_CLEAN_FILES ${outfile_target})
endfunction()
