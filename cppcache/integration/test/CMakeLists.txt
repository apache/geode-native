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

add_executable(cpp-integration-test
  AuthInitializeTest.cpp
  CacheXmlTest.cpp
  CommitConflictExceptionTest.cpp
  CqPlusAuthInitializeTest.cpp
  CqTest.cpp
  DataSerializableTest.cpp
  EnableChunkHandlerThreadTest.cpp
  ExampleTest.cpp
  ExpirationTest.cpp
  FunctionExecutionTest.cpp
  LocatorRequestsTest.cpp
  LRUEvictionTest.cpp
  PartitionRegionOpsTest.cpp
  PdxInstanceTest.cpp
  PdxJsonTypeTest.cpp
  PdxSerializerTest.cpp
  PdxSerializableTest.cpp
  Order.cpp
  Order.hpp
  RegionGetAllTest.cpp
  RegionPutAllTest.cpp
  RegionPutGetAllTest.cpp
  RegisterKeysTest.cpp
  SimpleAuthInitialize.cpp
  SimpleAuthInitialize.hpp
  SimpleCqListener.cpp
  SimpleCqListener.hpp
  SslOneWayTest.cpp
  SslTwoWayTest.cpp
  StructTest.cpp
  TransactionsTest.cpp
  TransactionCleaningTest.cpp
  CleanIdleConnections.cpp
  BasicIPv6Test.cpp
  CacheWriterTest.cpp
  PdxInstanceFactoryTest.cpp
  DisconnectEndPointAtException.cpp
  ContainsKeyOnServerExceptionTest.cpp
  WanDeserializationTest.cpp
)

target_compile_definitions(cpp-integration-test
  PUBLIC
    GTEST_ELLIPSIS_NEEDS_POD_
)

target_include_directories(cpp-integration-test
  PUBLIC
   ${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(cpp-integration-test
  PUBLIC
    apache-geode
    integration-framework
    testobject
    ACE::ACE
    GTest::gtest
    GTest::gtest_main
    GTest::gmock
    Boost::boost
    Boost::system
    Boost::log
    Boost::filesystem
    Boost::chrono
    cryptoImpl
  PRIVATE
    _WarningsAsError
    internal
)

add_dependencies(cpp-integration-test cryptoImpl)

if(WIN32)
  foreach (_target apache-geode testobject cryptoImpl)
    add_custom_command(TARGET cpp-integration-test POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$<TARGET_FILE:${_target}>"
        "$<$<CONFIG:Debug>:$<TARGET_PDB_FILE:${_target}>>"
        "$<TARGET_FILE_DIR:cpp-integration-test>")
  endforeach()
endif()

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/func_cacheserver1_pool.xml
  ${CMAKE_CURRENT_BINARY_DIR}/func_cacheserver1_pool.xml COPYONLY)
configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/func_cacheserver2_pool.xml
  ${CMAKE_CURRENT_BINARY_DIR}/func_cacheserver2_pool.xml COPYONLY)

set_target_properties(cpp-integration-test PROPERTIES
  CXX_VISIBILITY_PRESET hidden
  VISIBILITY_INLINES_HIDDEN ON
  FOLDER cpp/test/integration
)

add_clangformat(cpp-integration-test)

enable_testing()
include(GoogleTest)
gtest_discover_tests(cpp-integration-test)

file(GLOB SSL_CERTIFICATES
        "${CMAKE_CURRENT_SOURCE_DIR}/*Keys"
        )

file(INSTALL ${SSL_CERTIFICATES} DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
