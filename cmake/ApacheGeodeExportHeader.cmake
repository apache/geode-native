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
##
# FindGeode CMake find module.
##

if (MSVC)
  set(EXPORT_HEADER_CUSTOM_CONTENT "
#define APACHE_GEODE_EXPLICIT_TEMPLATE_EXPORT APACHE_GEODE_EXPORT

#define APACHE_GEODE_EXTERN_TEMPLATE_EXPORT
")

  target_compile_options(${PROJECT_NAME}
    PRIVATE
    /bigobj # C1128 - large number of templates causes too many section.
  )
else()
  set(EXPORT_HEADER_CUSTOM_CONTENT "
#define APACHE_GEODE_EXPLICIT_TEMPLATE_EXPORT

#define APACHE_GEODE_EXTERN_TEMPLATE_EXPORT APACHE_GEODE_EXPORT
")
endif()

include(GenerateExportHeader)
generate_export_header(${PROJECT_NAME}
    BASE_NAME APACHE_GEODE
    EXPORT_FILE_NAME apache-geode_export.h
    CUSTOM_CONTENT_FROM_VARIABLE EXPORT_HEADER_CUSTOM_CONTENT)
