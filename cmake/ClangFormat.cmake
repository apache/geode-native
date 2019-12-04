# Licensed to the Apache Software Foundation (ASF) under one or more
# contributor license agreements.  See the NOTICE file distributed with
# this work for additional information regarding copyright ownership.
# The ASF licenses this file to You under the Apache License, Version 2.0
# (the "License"); you may not use this file except in compliance with
# the License.  You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

set(ClangFormat_FLAGS "-style=file;-fallback-style=Google" CACHE STRING "Flags sent to clang-format")

find_package(ClangFormat QUIET)

add_custom_target(all-clangformat)
set_target_properties(all-clangformat PROPERTIES
        EXCLUDE_FROM_ALL TRUE
        EXCLUDE_FROM_DEFAULT_BUILD TRUE
        )

function(add_clangformat _target)
  if (ClangFormat_FOUND)
    if (NOT TARGET ${_target})
      message(FATAL_ERROR "add_clangformat should only be called on targets (got " ${_target} ")")
    endif ()

    # figure out which sources this should be applied to
    get_target_property(_target_SOURCES ${_target} SOURCES)
    get_target_property(_target_BINARY_DIR ${_target} BINARY_DIR)
    get_target_property(_target_SOURCE_DIR ${_target} SOURCE_DIR)

    set(_clangformat "${_target}-clangformat")
    set(_clangformat_SOURCES "")
    foreach (_source ${_target_SOURCES})
      if (NOT TARGET ${_source})
        get_source_file_property(_source_GENERATED "${_source}" GENERATED)
        string(REGEX MATCH "\\.(h(pp|xx)?|c(pp|xx)?)(\\..*)?$" _source_extension_match ${_source})
        if(_source_GENERATED
            OR _source_extension_match STREQUAL "")
          break()
        endif()

        get_source_file_property(_source_LOCATION "${_source}" LOCATION)
        file(RELATIVE_PATH _source_RELATIVE_PATH ${_target_SOURCE_DIR} ${_source_LOCATION})
        string(REPLACE ".." "__" _format_file "${_target_BINARY_DIR}/CMakeFiles/${_clangformat}.dir/${_source_RELATIVE_PATH}.format")
        get_filename_component(_format_DIRECTORY ${_format_file} DIRECTORY)
        file(MAKE_DIRECTORY ${_format_DIRECTORY})

        add_custom_command(OUTPUT ${_format_file}
            DEPENDS ${_source}
            COMMENT "Clang-Format ${_source}"
            COMMAND ${ClangFormat_EXECUTABLE} ${ClangFormat_FLAGS} -i ${_source_LOCATION}
            COMMAND ${CMAKE_COMMAND} -E touch ${_format_file})
        list(APPEND _clangformat_SOURCES ${_format_file})
      endif ()
    endforeach ()

    if (_clangformat_SOURCES)
      add_custom_target(${_clangformat}
          SOURCES ${_clangformat_SOURCES}
          COMMENT "Clang-Format for target ${_target}"
      )
      set_target_properties(${_clangformat} PROPERTIES FOLDER clangformat)
      add_dependencies(${_target} ${_clangformat})
      add_dependencies(all-clangformat ${_clangformat})
    endif ()

  endif ()
endfunction()
