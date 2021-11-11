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
# FindGeodeNative
# ---------
#
# Find the Geode Native headers and library.
#
# Imported Targets
# ^^^^^^^^^^^^^^^^
#
# This module defines the following :prop_tgt:`IMPORTED` targets:
#
# ``GeodeNative::cpp``
# ``GeodeNative::dotnet``
#
# Result Variables
# ^^^^^^^^^^^^^^^^
#
# This module will set the following variables in your project:
#
# ``GeodeNative_FOUND``
#   true if the Geode Native headers and libraries were found.
#
# ``GeodeNative_DOTNET_LIBRARY``
#   Path to .NET assembly file.
#

set(_GeodeNative_ROOT "")
if(GeodeNative_ROOT AND IS_DIRECTORY "${GeodeNative_ROOT}")
    set(_GeodeNative_ROOT "${GeodeNative_ROOT}")
    set(_GeodeNative_ROOT_EXPLICIT 1)
else()
    set(_ENV_GeodeNative_ROOT "")
    if(DEFINED ENV{GEODE_NATIVE_HOME})
        file(TO_CMAKE_PATH "$ENV{GEODE_NATIVE_HOME}" _ENV_GeodeNative_ROOT)
    endif()
    if(_ENV_GeodeNative_ROOT AND IS_DIRECTORY "${_ENV_GeodeNative_ROOT}")
        set(_GeodeNative_ROOT "${_ENV_GeodeNative_ROOT}")
        set(_GeodeNative_ROOT_EXPLICIT 0)
    endif()
    unset(_ENV_GeodeNative_ROOT)
endif()

set(_GeodeNative_HINTS)
set(_GeodeNative_PATHS)

if(_GeodeNative_ROOT)
    set(_GeodeNative_HINTS ${_GeodeNative_ROOT})
else()
    set(_GeodeNative_PATHS (
        "/opt/local"
        "/usr/local"
        "${CMAKE_CURRENT_SOURCE_DIR}/../../../"
        "C:/program files" ))
endif()

# Begin - component "cpp"
set(_GeodeNative_CPP_NAMES apache-geode)


find_library(GeodeNative_CPP_LIBRARY
    NAMES ${_GeodeNative_CPP_NAMES}
    HINTS ${_GeodeNative_HINTS}
    PATHS ${_GeodeNative_PATHS}
    PATH_SUFFIXES apache-geode/lib lib
)

# Look for the header file.
find_path(GeodeNative_CPP_INCLUDE_DIR NAMES geode/CacheFactory.hpp
    HINTS ${_GeodeNative_HINTS}
    PATHS ${_GeodeNative_PATHS}
    PATH_SUFFIXES apache-geode/include include
)
# End - component "cpp"


# Begin - component "dotnet"
set(_GeodeNative_DOTNET_NAMES Apache.Geode.dll)

find_file(GeodeNative_DOTNET_LIBRARY
  NAMES ${_GeodeNative_DOTNET_NAMES}
  HINTS ${_GeodeNative_HINTS}
  PATHS ${_GeodeNative_PATHS}
  PATH_SUFFIXES apache-geode/bin bin
)
# End - component "dotnet"


# TODO find version
set(GeodeNative_VERSION_STRING 1.0)

if (GeodeNative_FIND_COMPONENTS)
  set(_GeodeNative_REQUIRED_VARS)
  foreach (component ${GeodeNative_FIND_COMPONENTS})
    if (component STREQUAL "cpp")
      list(APPEND _GeodeNative_REQUIRED_VARS GeodeNative_CPP_LIBRARY GeodeNative_CPP_INCLUDE_DIR)
    endif()
    if (component STREQUAL "dotnet")
      list(APPEND _GeodeNative_REQUIRED_VARS GeodeNative_DOTNET_LIBRARY)
    endif()
  endforeach()
endif()

include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(GeodeNative
                                  REQUIRED_VARS ${_GeodeNative_REQUIRED_VARS}
                                  VERSION_VAR GeodeNative_VERSION_STRING)

# Copy the results to the output variables and target.
if(GeodeNative_FOUND)
  if(NOT TARGET ${GeodeNative_CPP_TARGET})
    set(GeodeNative_CPP_TARGET "GeodeNative::cpp")
    add_library(${GeodeNative_CPP_TARGET} UNKNOWN IMPORTED)
    set_target_properties(${GeodeNative_CPP_TARGET} PROPERTIES
      IMPORTED_LINK_INTERFACE_LANGUAGES "CXX"
      IMPORTED_LOCATION "${GeodeNative_CPP_LIBRARY}"
      INTERFACE_INCLUDE_DIRECTORIES "${GeodeNative_CPP_INCLUDE_DIR}")
  endif()
  set(GeodeNative_DOTNET_TARGET "GeodeNative::dotnet")
  if(NOT TARGET ${GeodeNative_DOTNET_TARGET})
    add_library(${GeodeNative_DOTNET_TARGET} UNKNOWN IMPORTED)
    set_target_properties(${GeodeNative_DOTNET_TARGET} PROPERTIES
      IMPORTED_LINK_INTERFACE_LANGUAGES "CSharp"
      IMPORTED_LOCATION "${GeodeNative_DOTNET_LIBRARY}")
  endif()
else()
  message(STATUS "FOUND var not set")
endif()

mark_as_advanced(GeodeNative_CPP_INCLUDE_DIR GeodeNative_CPP_LIBRARY GeodeNative_DOTNET_LIBRARY)
