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

cmake_minimum_required(VERSION 3.18)
project(nativeclient LANGUAGES C CXX)

option(USE_PCH "Use precompiled headers (PCH)." OFF)
option(USE_CPP_COVERAGE "Enable profiling and coverage report analysis for apache-geode cpp library." OFF)
option(USE_RAT "Enable Apache Rat checking." OFF)
option(WITH_IPV6 "Enable IPv6 support." OFF)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

if (USE_PCH)
  include(cotire)
endif()

include(ClangFormat)
include(ApacheGeodeExportHeader)

if(CMAKE_GENERATOR MATCHES Win64*)
  if ((CMAKE_GENERATOR MATCHES "Visual Studio") AND (CMAKE_GENERATOR_TOOLSET STREQUAL ""))
    message(FATAL_ERROR "GEODE expects that a user must provide -Thost=x64 if you are using a"
            " 64-bit toolset, otherwise you may get a linker error when DLLs are larger"
            " than 2GB saying \"Unable to open file apache-geode-static.dll.\" This is due"
            " to Visual Studio using the 32-bit toolset by default.")
  endif()
endif()
if(APPLE AND NOT DEFINED ENV{MACOSX_DEPLOYMENT_TARGET} AND NOT DEFINED ENV{SDKROOT})
  set(CMAKE_OSX_DEPLOYMENT_TARGET 10.14)
endif()

get_property(GENERATOR_IS_MULTI_CONFIG GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
if(NOT CMAKE_BUILD_TYPE AND NOT GENERATOR_IS_MULTI_CONFIG)
  message(STATUS "Using default build type 'Debug'.")
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "Specifies the build type on single-configuration generators." FORCE)
endif()

set(BUILD_BITS 64 CACHE STRING "Build for 64 (Geode default) or 32 bit.")
option(BUILD_BENCHMARKS "Build benchmarks" ON)

set(PRODUCT_VENDOR "Apache" CACHE STRING "Product vendor")
set(PRODUCT_VENDOR_NAME "The Apache Software Foundation" CACHE STRING "Product vendor full legal name")
set(PRODUCT_BASE_NAME "Geode" CACHE STRING "Product base name")
set(PRODUCT_NAME "${PRODUCT_BASE_NAME} Native" CACHE STRING "Product name")
set(PRODUCT_VERSION "0.0.42-build.0" CACHE STRING "Product version")
set(PRODUCT_PACKAGE_NAME "apache-geode-native" CACHE STRING "Product package name")
set(PRODUCT_BUILDDATE "1970-01-01" CACHE STRING "Product build date")
set(PRODUCT_SOURCE_REVISION "0000000000000000000000000000000000000000" CACHE STRING "Product source SHA")
set(PRODUCT_SOURCE_REPOSITORY "<unspecified>" CACHE STRING "Product source branch")
set(PRODUCT_BITS "${BUILD_BITS}bit")

set(PRODUCT_LIB_NAME "apache-geode" CACHE STRING "Binary name")
set(PRODUCT_DLL_NAME "${PRODUCT_VENDOR}.${PRODUCT_BASE_NAME}" CACHE STRING ".Net Binary name")

if(CMAKE_SYSTEM_NAME STREQUAL "SunOS")
  if (CMAKE_SYSTEM_PROCESSOR STREQUAL "sparc")
    set(PRODUCT_SYSTEM_NAME "solaris-sparc")
  else()
    set(PRODUCT_SYSTEM_NAME "solaris-x86")
  endif()
else()
  set(PRODUCT_SYSTEM_NAME "${CMAKE_SYSTEM_NAME}" CACHE STRING "Product built for system name")
endif()

string(REGEX REPLACE "-build" "" PRODUCT_VERSION_DOTTED ${PRODUCT_VERSION})
string(REGEX MATCH "^[0-9]+\\.[0-9]+\\.[0-9]+" PRODUCT_VERSION_LONG ${PRODUCT_VERSION})
string(REGEX MATCH "^[0-9]+\\.[0-9]+" PRODUCT_VERSION_SHORT ${PRODUCT_VERSION})
string(REGEX REPLACE "\\." ";" PRODUCT_VERSION_LIST ${PRODUCT_VERSION_DOTTED})
list(LENGTH PRODUCT_VERSION_LIST _length)
if (_length LESS 4)
  foreach(_index RANGE ${_length} 3)
    list(APPEND PRODUCT_VERSION_LIST 0)
  endforeach()
endif()
list(GET PRODUCT_VERSION_LIST 0 PRODUCT_VERSION_MAJOR)
list(GET PRODUCT_VERSION_LIST 1 PRODUCT_VERSION_MINOR)
list(GET PRODUCT_VERSION_LIST 2 PRODUCT_VERSION_PATCH)
list(GET PRODUCT_VERSION_LIST 3 PRODUCT_VERSION_BUILD)

set(APIDOC_VERSION ${PRODUCT_VERSION_MAJOR}.${PRODUCT_VERSION_MINOR}.${PRODUCT_VERSION_PATCH})

# Please note that attempts to set CMAKE_INSTALL_PREFIX to a *ROOTED*
# path here will fail due to the design of CMake according to its
# development team. Setting it to a relative path appears to work.
# To override the install location, CMAKE_INSTALL_PREFIX must be
# specified at the time of generation, e.g.:
# $ cmake -G Xcode -DCMAKE_INSTALL_PREFIX=/my/favorite/location ..
set(CMAKE_INSTALL_PREFIX "nativeclient" CACHE STRING "Install prefix")

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# TODO this doesn't seem to have effect
set(CMAKE_ECLIPSE_VERSION Mars)

#TODO this check is failing to fail properly on solaris with sun CC 5.10
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CPACK_PACKAGE_VERSION_MAJOR ${PRODUCT_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${PRODUCT_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${PRODUCT_VERSION_PATCH})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "${PRODUCT_VENDOR} ${PRODUCT_NAME}")
set(CPACK_PACKAGE_VENDOR "${PRODUCT_VENDOR}")
set(CPACK_PACKAGE_NAME "${PRODUCT_PACKAGE_NAME}")
set(CPACK_SYSTEM_NAME "${PRODUCT_SYSTEM_NAME}-${PRODUCT_BITS}")
set(CPACK_PACKAGE_FILE_NAME ${CPACK_PACKAGE_NAME}-${PRODUCT_VERSION}-${CPACK_SYSTEM_NAME})
set(CPACK_SOURCE_PACKAGE_FILE_NAME ${CPACK_PACKAGE_NAME}-${PRODUCT_VERSION}-src)
set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY 0)
set(CPACK_PACKAGING_INSTALL_PREFIX "/${CPACK_PACKAGE_NAME}")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/dist/LICENSE")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "${CPACK_PACKAGE_NAME}")
set(CPACK_GENERATOR TGZ;ZIP)
set(CPACK_PACKAGE_CHECKSUM "SHA512")

# Build a comprehensive list of things to leave out of the cpack package for our
# source code releases.  Use .gitignore as a base, then use '.cpackignore' to
# leave out .git and a couple other items we want in the repository but not in
# the release.  Everything gets concatenated into a ';' separated list for
# cpack.
file(READ ".cpackignore" IGNORED_PARTIAL)
set(IGNORED_FULL ${IGNORED_PARTIAL})
string(REGEX REPLACE "\n" ";" IGNORED_FULL "${IGNORED_FULL}")
string(REGEX REPLACE " " "" IGNORED_FULL "${IGNORED_FULL}")

list(APPEND CPACK_SOURCE_IGNORE_FILES ${IGNORED_FULL})

include(CPack)

include(CheckCXXCompilerFlag)
include(CheckCCompilerFlag)

set(CMAKE_REQUIRED_LIBRARIES -m64)
check_c_compiler_flag(-m64 CFLAGS_M64_ALLOWED)
check_cxx_compiler_flag(-m64 CXXFLAGS_M64_ALLOWED)
set(CMAKE_REQUIRED_LIBRARIES)

check_c_compiler_flag(-mt CFLAGS_mt_ALLOWED)

if (CFLAGS_M64_ALLOWED AND CXXFLAGS_M64_ALLOWED)
  set(CMAKE_REQUIRED_FLAGS ${CMAKE_REQUIRED_FLAGS} -m64)
  add_compile_options(-m64)
#TODO cmake find better way to set linker flags
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -m64")
  set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -m64")
  set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -m64")
endif()

if (CFLAGS_mt_ALLOWED)
  set(CMAKE_REQUIRED_FLAGS ${CMAKE_REQUIRED_FLAGS} -mt)
  add_compile_options(-mt)
endif()

#TODO remove this debugging for NMake
set(CMAKE_VERBOSE_MAKEFILE 0)

# Interface for C++11 language
add_library(c++11 INTERFACE)

# Interface for warning flags
add_library(_WarningsAsError INTERFACE)
add_library(_CppCodeCoverage INTERFACE)

include(CheckIPOSupported)
check_ipo_supported(RESULT ipo_supported)
if(ipo_supported)
  set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE)
  set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELWITHDEBINFO TRUE)
endif()

include(CheckLinkerFlag)
check_linker_flag(CXX -Wl,--exclude-libs,ALL has_linker_exclude_libs)

include(CheckCXXSymbolExists)
check_cxx_symbol_exists("PRId8" "cinttypes" HAVE_STDC_FORMAT_MACROS)
if (NOT HAVE_STDC_FORMAT_MACROS)
  target_compile_definitions(c++11 INTERFACE __STDC_FORMAT_MACROS)
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "SunPro")
  # Force linker to error on undefined symbols in shared libraries
  set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -z defs")
  if (PRODUCT_SYSTEM_NAME STREQUAL "solaris-sparc")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -xatomic=studio")
  elseif(PRODUCT_SYSTEM_NAME STREQUAL "solaris-x86")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -xatomic=none")
  endif()
  # TODO cmake find a better way to set runtime libraries
  # C++11 requires these libraries, treat -std=c++11 as library
  #TODO look into CMAKE_CXX_STANDARD_LIBRARIES
  target_link_libraries(c++11 INTERFACE -std=c++11 stdc++ gcc_s CrunG3 m c)

  target_compile_options(_WarningsAsError INTERFACE
    -errtags
    -errwarn=%all
  )
elseif (CMAKE_CXX_COMPILER_ID MATCHES "GNU")
  set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,-z,defs")
  target_compile_options(_WarningsAsError INTERFACE
    -Werror
    -Wall
    -Wno-unused-variable #TODO fix
    -Wpedantic
    # -Weffc++ TODO fix
    )

  if (USE_CPP_COVERAGE)
    target_compile_options(_CppCodeCoverage INTERFACE --coverage)
    target_link_libraries(_CppCodeCoverage INTERFACE --coverage)
  endif()

elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  check_cxx_compiler_flag(-Wno-defaulted-function-deleted CXXFLAGS_NO_DEFAULTED_FUNCTION_DELETED_ALLOWED)
  if( CXXFLAGS_NO_DEFAULTED_FUNCTION_DELETED_ALLOWED )
    set(DISABLED_NO_DEFAULTED_FUNCTION_DELETED_ALLOWED -Wno-defaulted-function-deleted)
  endif()

  check_cxx_compiler_flag(-Wno-c++2a-compat CXXFLAGS_NO_C++2a_COMPAT_ALLOWED)
  if( CXXFLAGS_NO_C++2a_COMPAT_ALLOWED )
    set(DISABLED_NO_C++2a_COMPAT_ALLOWED -Wno-c++2a-compat)
  endif()
  target_compile_options(_WarningsAsError INTERFACE
    -Werror
    -Wall
    -Wextra
    -Wno-unused-variable #TODO fix
    -Wno-unused-function #TODO fix
    -Wpedantic
    -Weverything
    -Wno-non-virtual-dtor #TODO fix
    -Wno-missing-prototypes #TODO fix
    -Wno-deprecated #TODO fix
    -Wno-format-nonliteral #TODO fix
    -Wno-double-promotion #TODO fix
    -Wno-undefined-func-template #TODO fix
    -Wno-float-equal #TODO fix
    -Wno-conversion #TODO fix
    -Wno-reserved-id-macro #TODO fix
    -Wno-range-loop-analysis
    -Wno-weak-vtables
    -Wno-weak-template-vtables
    -Wno-padded
    -Wno-c++98-compat
    -Wno-c++98-compat-pedantic
    -Wno-global-constructors
    -Wno-exit-time-destructors
    -Wno-documentation-unknown-command
    ${DISABLED_NO_DEFAULTED_FUNCTION_DELETED_ALLOWED}
    ${DISABLED_NO_C++2a_COMPAT_ALLOWED}
    )

  if (USE_CPP_COVERAGE)
    target_compile_options(_CppCodeCoverage INTERFACE --coverage)
    target_link_libraries(_CppCodeCoverage INTERFACE --coverage)
  endif()

elseif(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
  set(CMAKE_DOTNET_TARGET_FRAMEWORK_VERSION "v4.5.2")

  set(BUILD_CLI 1)

  # Interface for C++/CLI language.
  add_library(c++cli INTERFACE)
  target_compile_options(c++cli INTERFACE
    /EHa)

  target_compile_options(_WarningsAsError INTERFACE
    /WX
    /wd4996
    /we4596
  )

  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /ignore:4099")
  set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /ignore:4099")

  # /MP Enables multiprocess compiles
  # SPDLOG_NO_TLS disables use of tls code which clashes with managed build on Windows
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP /DSPDLOG_NO_TLS")

  # Enables strong name signing
  set(STRONG_NAME_KEY "" CACHE FILEPATH "Strong Name Key File")
  if(EXISTS "${STRONG_NAME_KEY}")
    set(SHARED_LINKER_FLAGS_STRONG_KEY "/keyfile:${STRONG_NAME_KEY}")
    execute_process(COMMAND sn -p ${STRONG_NAME_KEY} ${CMAKE_CURRENT_BINARY_DIR}/public.snk)
    execute_process(COMMAND sn -tp ${CMAKE_CURRENT_BINARY_DIR}/public.snk OUTPUT_VARIABLE STRONG_NAME_PUBLIC_KEY)
    string(REPLACE "\n" "" STRONG_NAME_PUBLIC_KEY ${STRONG_NAME_PUBLIC_KEY})
    string(REGEX REPLACE ".*sha1\\):([a-f0-9]+).*" "\\1" STRONG_NAME_PUBLIC_KEY ${STRONG_NAME_PUBLIC_KEY})
  endif()

  find_program(NUGET nuget)
  add_custom_target(nuget-restore 
    COMMAND ${NUGET} restore ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.sln
  )

endif()

find_package(Java 1.8.0.60 REQUIRED COMPONENTS Development)

if ("${CMAKE_AR}" STREQUAL "CMAKE_AR-NOTFOUND")
  message(FATAL_ERROR "Utility ar not found.")
endif()

# Default to only showing output on failure for unit tests but allow
# overriding with the CTEST_UNITTEST_VERBOSITY environment variable.
set(CTEST_UNITTEST_VERBOSITY --output-on-failure)
if(DEFINED ENV{CTEST_UNITTEST_VERBOSITY})
    set(CTEST_UNITTEST_VERBOSITY $ENV{CTEST_UNITTEST_VERBOSITY})
endif()

find_package(Geode 1.0 REQUIRED)

add_custom_target(client-libraries)
set_target_properties(client-libraries PROPERTIES
	FOLDER test-common
)

add_custom_target(unit-tests)
set_target_properties(unit-tests PROPERTIES
	FOLDER cpp/test/unit
)

add_custom_target(run-unit-tests)
add_dependencies(run-unit-tests unit-tests)
set_target_properties(run-unit-tests PROPERTIES
  EXCLUDE_FROM_ALL TRUE
  EXCLUDE_FROM_DEFAULT_BUILD TRUE
	FOLDER cpp/test/unit
)

add_custom_target(integration-tests)
set_target_properties(integration-tests PROPERTIES FOLDER cpp/test/integration)

add_custom_target(run-integration-tests)
add_dependencies(run-integration-tests integration-tests)
set_target_properties(run-integration-tests PROPERTIES
  EXCLUDE_FROM_ALL TRUE
  EXCLUDE_FROM_DEFAULT_BUILD TRUE
	FOLDER test-common
)

add_custom_target(acceptance-tests)
set_target_properties(acceptance-tests PROPERTIES FOLDER cpp/test/acceptance)

add_custom_target(run-acceptance-tests)
add_dependencies(run-acceptance-tests acceptance-tests)
set_target_properties(run-acceptance-tests PROPERTIES
  EXCLUDE_FROM_ALL TRUE
  EXCLUDE_FROM_DEFAULT_BUILD TRUE
	FOLDER test-common
)

find_package(OpenSSL REQUIRED)

find_program(DOCKER docker)
set(DOCKER_PGM ${DOCKER} CACHE INTERNAL "")

find_program(DOCKERCOMPOSE docker-compose)
set(DOCKERCOMPOSE_PGM ${DOCKERCOMPOSE} CACHE INTERNAL "")

add_subdirectory(tests/javaobject)
add_subdirectory(dependencies)
add_subdirectory(cppcache)
add_subdirectory(sqliteimpl)
add_subdirectory(templates/security)
add_subdirectory(docs/api)
add_subdirectory(examples)
if (${BUILD_CLI})
  add_subdirectory(clicache)
  add_subdirectory(templates/security/csharp)
endif()
add_subdirectory(tests)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/xsds/ DESTINATION xsds)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/defaultSystem/ DESTINATION defaultSystem)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/dist/ DESTINATION .)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/templates/ DESTINATION templates
  PATTERN "templates/security/CMakeLists.txt" EXCLUDE
  PATTERN "templates/security/CMakeLists.txt.forInstall" EXCLUDE)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/templates/security/CMakeLists.txt.forInstall RENAME CMakeLists.txt DESTINATION templates/security)

install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/ssl_keys/server_keys/server_keystore_chained.p12 RENAME server_keystore.p12 DESTINATION examples/utilities/ServerSslKeys)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/ssl_keys/server_keys/server_truststore_chained_root.jks RENAME server_truststore.jks DESTINATION examples/utilities/ServerSslKeys)

install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/ssl_keys/client_keys/client_keystore_chained.pem RENAME client_keystore.pem DESTINATION examples/utilities/ClientSslKeys)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/ssl_keys/client_keys/client_truststore_chained_root.pem RENAME client_truststore.pem DESTINATION examples/utilities/ClientSslKeys)

if (USE_RAT)
  add_custom_target( rat-check
    COMMAND ${CMAKE_COMMAND}
        -DJava_JAVA_EXECUTABLE=${Java_JAVA_EXECUTABLE}
        -DRat_JAR=${Rat_JAR}
        -P cmake/RatCheck.cmake
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Checking source with Apache Rat."
  )
endif()
