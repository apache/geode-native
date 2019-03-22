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
#

execute_process(
  COMMAND ${Java_JAVA_EXECUTABLE} -jar ${Rat_JAR} -d . -E .ratignore >
  OUTPUT_VARIABLE ratOutput
)

set(pass FALSE)
if (ratOutput MATCHES "([0-9]+) Unknown Licenses")
  set(unknownLicenses ${CMAKE_MATCH_1})
  if (unknownLicenses GREATER 0)
    message(SEND_ERROR "${ratOutput}")
    message(FATAL_ERROR "${unknownLicenses} Unknown licenses detected.")
  endif()
else()
  message(SEND_ERROR "${ratOutput}")
  message(FATAL_ERROR "Unknown failure")
endif()
