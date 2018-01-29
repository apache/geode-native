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

#.rst:
# Findgeode-native
# ---------
#
# Find the Geode Native headers and library.
#
# Imported Targets
# ^^^^^^^^^^^^^^^^
#
# This module defines the following :prop_tgt:`IMPORTED` targets:
#
# ``apache.geode::cpp``
#
# Result Variables
# ^^^^^^^^^^^^^^^^
#
# This module will set the following variables in your project:
#
# ``GEODE_NATIVE_INCLUDE_DIRS``
#   where to find CacheFactory.hpp, etc.
# ``GEODE_NATIVE_LIBRARIES``
#   the libraries to link against to use Geode Native.
# ``GEODE_NATIVE_FOUND``
#   true if the Geode Native headers and libraries were found.
#


set(_GEODE_NATIVE_ROOT "")
if(GEODE_NATIVE_ROOT AND IS_DIRECTORY "${GEODE_NATIVE_ROOT}")
    set(_GEODE_NATIVE_ROOT "${GEODE_NATIVE_ROOT}")
    set(_GEODE_NATIVE_ROOT_EXPLICIT 1)
else()
    set(_ENV_GEODE_NATIVE_ROOT "")
    if(DEFINED ENV{GFCPP})
        file(TO_CMAKE_PATH "$ENV{GFCPP}" _ENV_GEODE_NATIVE_ROOT)
    endif()
    if(_ENV_GEODE_NATIVE_ROOT AND IS_DIRECTORY "${_ENV_GEODE_NATIVE_ROOT}")
        set(_GEODE_NATIVE_ROOT "${_ENV_GEODE_NATIVE_ROOT}")
        set(_GEODE_NATIVE_ROOT_EXPLICIT 0)
    endif()
    unset(_ENV_GEODE_NATIVE_ROOT)
endif()

set(_GEODE_NATIVE_HINTS)
set(_GEODE_NATIVE_PATHS)

if(_GEODE_NATIVE_ROOT)
    set(_GEODE_NATIVE_HINTS ${_GEODE_NATIVE_ROOT})
else()
    set(_GEODE_NATIVE_PATHS (
        "/opt/local"
        "/usr/local"
        "../../../../"
        "C:/program files" ))
endif()

set(_GEODE_NATIVE_NAMES apache-geode)

find_library(GEODE_NATIVE_LIBRARY
    NAMES ${_GEODE_NATIVE_NAMES}
    HINTS ${_GEODE_NATIVE_HINTS}
    PATHS ${_GEODE_NATIVE_PATHS}
    PATH_SUFFIXES geode-native/lib
)



# Look for the header file.
find_path(GEODE_NATIVE_INCLUDE_DIR NAMES geode/CacheFactory.hpp
    HINTS ${_GEODE_NATIVE_HINTS}
    PATHS ${_GEODE_NATIVE_PATHS}
    PATH_SUFFIXES geode-native/include
)

# TODO find version
set(GEODE_NATIVE_VERSION_STRING 1.0)

include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(GEODE_NATIVE
                                  REQUIRED_VARS GEODE_NATIVE_LIBRARY GEODE_NATIVE_INCLUDE_DIR
                                  VERSION_VAR GEODE_NATIVE_VERSION_STRING)

# Copy the results to the output variables and target.
if(GEODE_NATIVE_FOUND)
  set(GEODE_NATIVE_LIBRARIES ${GEODE_NATIVE_LIBRARY})
  set(GEODE_NATIVE_INCLUDE_DIRS ${GEODE_NATIVE_INCLUDE_DIR})
  set(GEODE_NATIVE_CPP_TARGET "apache.geode::cpp")
  if(NOT TARGET ${GEODE_NATIVE_CPP_TARGET})
    add_library(${GEODE_NATIVE_CPP_TARGET} UNKNOWN IMPORTED)
    set_target_properties(${GEODE_NATIVE_CPP_TARGET} PROPERTIES
      IMPORTED_LINK_INTERFACE_LANGUAGES "CXX"
      IMPORTED_LOCATION "${GEODE_NATIVE_LIBRARY}"
      INTERFACE_INCLUDE_DIRECTORIES "${GEODE_NATIVE_INCLUDE_DIRS}")
  endif()
endif()

mark_as_advanced(GEODE_NATIVE_INCLUDE_DIR GEODE_NATIVE_LIBRARY)
