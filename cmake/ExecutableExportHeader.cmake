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

function(GENERATE_EXEC_EXPORT_HEADER TARGET)
  get_property(type TARGET ${TARGET} PROPERTY TYPE)
  if(NOT ${type} STREQUAL "EXECUTABLE")
    message(WARNING "This macro can only be used with executables")
    return()
  endif()

  include(GenerateExportHeader)

  _test_compiler_hidden_visibility()
  _test_compiler_has_deprecated()
  _do_set_macro_values(${TARGET})
  _do_generate_export_header(${TARGET} ${ARGN})
endfunction()