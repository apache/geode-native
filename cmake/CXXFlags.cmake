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

include(CheckLinkerFlag)

set(CHECK_FEATURE "Undefined Behavior Sanitizer")
message(CHECK_START ${CHECK_FEATURE})

list(APPEND CMAKE_MESSAGE_INDENT "  ")
check_cxx_compiler_flag(-fsanitize=undefined CXX_HAS_SANITIZE_UNDEFINED_FLAG)
unset(CXX_SANITIZE_UNDEFINED_FLAG)
if (CXX_HAS_SANITIZE_UNDEFINED_FLAG)
  set(CXX_SANITIZE_UNDEFINED_FLAG -fsanitize=undefined)
endif()

check_linker_flag(CXX -fsanitize=undefined CXX_LINKER_HAS_SANITIZE_UNDEFINED_FLAG)
unset(CXX_LINKER_SANITIZE_UNDEFINED_FLAG)
if (CXX_LINKER_HAS_SANITIZE_UNDEFINED_FLAG)
  set(CXX_LINKER_SANITIZE_UNDEFINED_FLAG -fsanitize=undefined)
endif()
list(POP_BACK CMAKE_MESSAGE_INDENT)

if (CXX_HAS_SANITIZE_UNDEFINED_FLAG)
  option(USE_SANITIZE_UNDEFINED "Use ${CHECK_FEATURE}" FALSE)
  if (USE_SANITIZE_UNDEFINED)
    message(CHECK_PASS "Enabled")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CXX_SANITIZE_UNDEFINED_FLAG}")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${CXX_LINKER_SANITIZE_UNDEFINED_FLAG}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${CXX_LINKER_SANITIZE_UNDEFINED_FLAG}")
  else()
    message(CHECK_FAIL "Disabled")
  endif()
else()
  if (USE_SANITIZE_UNDEFINED)
    message(FATAL_ERROR "${CHECK_FEATURE} enabled but not supported.")
  else()
    message(CHECK_FAIL "Unsupported")
  endif()
endif()


set(CHECK_FEATURE "Address Sanitizer")
message(CHECK_START ${CHECK_FEATURE})

list(APPEND CMAKE_MESSAGE_INDENT "  ")
check_linker_flag(CXX -fsanitize=address CXX_LINKER_HAS_SANITIZE_ADDRESS_FLAG)
unset(CXX_LINKER_SANITIZE_ADDRESS_FLAG)
if (CXX_LINKER_HAS_SANITIZE_ADDRESS_FLAG)
  set(CXX_LINKER_SANITIZE_ADDRESS_FLAG -fsanitize=address)
endif()

if (CXX_LINKER_HAS_SANITIZE_ADDRESS_FLAG)
  list(APPEND CMAKE_REQUIRED_LINK_OPTIONS -fsanitize=address)
endif()
check_cxx_compiler_flag(-fsanitize=address CXX_HAS_SANITIZE_ADDRESS_FLAG)
if (CXX_LINKER_HAS_SANITIZE_ADDRESS_FLAG)
  list(POP_BACK CMAKE_REQUIRED_LINK_OPTIONS)
endif()
unset(CXX_SANITIZE_ADDRESS_FLAG)
if (CXX_HAS_SANITIZE_ADDRESS_FLAG)
  set(CXX_SANITIZE_ADDRESS_FLAG -fsanitize=address)
endif()

check_linker_flag(CXX -fno-omit-frame-pointer CXX_HAS_NO_OMIT_FRAME_POINTER_FLAG)
unset(CXX_NO_OMIT_FRAME_POINTER_FLAG)
if (CXX_HAS_NO_OMIT_FRAME_POINTER_FLAG)
  set(CXX_NO_OMIT_FRAME_POINTER_FLAG -fno-omit-frame-pointer)
endif()

list(POP_BACK CMAKE_MESSAGE_INDENT)

if (CXX_HAS_SANITIZE_ADDRESS_FLAG)
  option(USE_SANITIZE_ADDRESS "Use ${CHECK_FEATURE}" FALSE)
  if (USE_SANITIZE_ADDRESS)
    message(CHECK_PASS "Enabled")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CXX_SANITIZE_ADDRESS_FLAG} ${CXX_NO_OMIT_FRAME_POINTER_FLAG}")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${CXX_LINKER_SANITIZE_ADDRESS_FLAG}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${CXX_LINKER_SANITIZE_ADDRESS_FLAG}")
  else()
    message(CHECK_FAIL "Disabled")
  endif()
else()
  if (USE_SANITIZE_ADDRESS)
    message(FATAL_ERROR "${CHECK_FEATURE} enabled but not supported.")
  else()
    message(CHECK_FAIL "Unsupported")
  endif()
endif()


set(CHECK_FEATURE "Sanitizer errors fatal.")
message(CHECK_START ${CHECK_FEATURE})

list(APPEND CMAKE_MESSAGE_INDENT "  ")
check_cxx_compiler_flag(-fno-sanitize-recover CXX_HAS_NO_SANITIZE_RECOVER_FLAG)
unset(CXX_NO_SANITIZE_RECOVER_FLAG)
if (CXX_HAS_NO_SANITIZE_RECOVER_FLAG)
  set(CXX_NO_SANITIZE_RECOVER_FLAG -fno-sanitize-recover)
endif()
list(POP_BACK CMAKE_MESSAGE_INDENT)

if (CXX_NO_SANITIZE_RECOVER_FLAG)
  option(USE_SANITIZE_FATAL "Use ${CHECK_FEATURE}" ${CXX_HAS_NO_SANITIZE_RECOVER_FLAG})
  if (USE_SANITIZE_FATAL)
    message(CHECK_PASS "Enabled")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CXX_NO_SANITIZE_RECOVER_FLAG}")
  else()
    message(CHECK_FAIL "Disabled")
  endif()
else()
  if (USE_SANITIZE_FATAL)
    message(FATAL_ERROR "${CHECK_FEATURE} enabled but not supported.")
  else()
    message(CHECK_FAIL "Unsupported")
  endif()
endif()


set(CHECK_FEATURE "Interprocedural or link time optimizations (IPO/LTO)")
message(CHECK_START ${CHECK_FEATURE})

list(APPEND CMAKE_MESSAGE_INDENT "  ")
include(CheckIPOSupported)
if (NOT DEFINED CXX_HAS_IPO_SUPPORT)
  check_ipo_supported(RESULT CXX_HAS_IPO_SUPPORT LANGUAGES CXX)
  set(CXX_HAS_IPO_SUPPORT ${CXX_HAS_IPO_SUPPORT} CACHE INTERNAL "Test check_ipo_supported" FORCE)
endif()
list(POP_BACK CMAKE_MESSAGE_INDENT)

if(CXX_HAS_IPO_SUPPORT)
  option(USE_IPO "Use ${CHECK_FEATURE}" ${CXX_HAS_IPO_SUPPORT})
  if (USE_IPO)
    message(CHECK_PASS "Enabled")
  else()
    message(CHECK_FAIL "Disabled")
  endif()
  set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE ${USE_IPO})
  set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELWITHDEBINFO ${USE_IPO})
else()
  set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE FALSE)
  set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELWITHDEBINFO FALSE)
  if (USE_IPO)
    message(FATAL_ERROR "${CHECK_FEATURE} enabled but not supported.")
  else()
    message(CHECK_FAIL "Unsupported")
  endif()
endif()
