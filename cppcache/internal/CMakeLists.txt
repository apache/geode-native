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

set(CMAKE_CXX_STANDARD 11)

add_library(internal_ INTERFACE)

add_clangformat(internal_)

set(INTERNAL_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(INTERNAL_SRC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/string.cpp ${CMAKE_CURRENT_SOURCE_DIR}/src/JavaModifiedUtf8.cpp ${CMAKE_CURRENT_SOURCE_DIR}/src/chrono/time_point.cpp ${CMAKE_CURRENT_SOURCE_DIR}/src/concurrent/binary_semaphore.cpp ${CMAKE_CURRENT_SOURCE_DIR}/src/concurrent/spinlock_mutex.cpp)

add_subdirectory(shared)