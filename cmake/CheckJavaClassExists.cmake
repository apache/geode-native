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

function(check_java_class_exists CLASS CLASSPATH VAR)
  if(NOT DEFINED "${VAR}" OR "x${${VAR}}" STREQUAL "x${VAR}")
    message(CHECK_START "Looking for Java class ${CLASS} in ${CLASSPATH}")

    set(OUTPUT_PATH "${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/check_java_class_exists.dir")
    file(MAKE_DIRECTORY "${OUTPUT_PATH}")

    set(GENERATED_SOURCE "${OUTPUT_PATH}/Check.java")
    file(WRITE "${GENERATED_SOURCE}" "import ${CLASS};")

    execute_process(
      COMMAND ${Java_JAVAC_EXECUTABLE}
      ${CMAKE_JAVA_COMPILE_FLAGS}
      -classpath "${CLASSPATH}"
      -d ${OUTPUT_PATH}
      ${GENERATED_SOURCE}
      RESULT_VARIABLE exit_code
      OUTPUT_QUIET
      ERROR_QUIET
      OUTPUT_FILE "${OUTPUT_PATH}/check.stdout"
      ERROR_FILE "${OUTPUT_PATH}/check.stderr"
    )

    if (${exit_code} EQUAL 0)
      message(CHECK_PASS "found")
      set(${VAR} 1 CACHE INTERNAL "Have Java class ${CLASS}")
    else()
      message(CHECK_FAIL "not found")
      set(${VAR} 0 CACHE INTERNAL "Have Java class ${CLASS}")
    endif()
  endif()
endfunction()