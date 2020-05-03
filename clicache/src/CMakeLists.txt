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

cmake_minimum_required(VERSION 3.10)
project(Apache.Geode CXX)

if(NOT "${STRONG_NAME_PUBLIC_KEY}" STREQUAL "")
  set(STRONG_NAME_PUBLIC_KEY_ATTRIBUTE ", PublicKey=${STRONG_NAME_PUBLIC_KEY}")
endif()
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/impl/AssemblyInfo.cpp.in ${CMAKE_CURRENT_BINARY_DIR}/impl/AssemblyInfo.cpp)
list(APPEND CONFIGURE_IN_FILES ${CMAKE_CURRENT_SOURCE_DIR}/impl/AssemblyInfo.cpp.in)
list(APPEND CONFIGURE_OUT_FILES ${CMAKE_CURRENT_BINARY_DIR}/impl/AssemblyInfo.cpp)

add_library(Apache.Geode SHARED
  Apache.Geode.rc
  AttributesMutator.cpp
  AttributesMutator.hpp
  begin_native.hpp
  Cache.cpp
  Cache.hpp
  CacheableArrayList.hpp
  CacheableBuiltins.hpp
  CacheableDate.cpp
  CacheableDate.hpp
  CacheableFileName.cpp
  CacheableFileName.hpp
  CacheableHashMap.cpp
  CacheableHashMap.hpp
  CacheableHashSet.hpp
  CacheableHashTable.hpp
  CacheableIdentityHashMap.hpp
  CacheableKey.cpp
  CacheableKey.hpp
  CacheableLinkedList.hpp
  CacheableObject.cpp
  CacheableObject.hpp
  CacheableObjectArray.cpp
  CacheableObjectArray.hpp
  CacheableObjectXml.cpp
  CacheableObjectXml.hpp
  CacheableStack.cpp
  CacheableStack.hpp
  CacheableString.cpp
  CacheableString.hpp
  CacheableStringArray.cpp
  CacheableStringArray.hpp
  CacheableUndefined.cpp
  CacheableUndefined.hpp
  CacheableVector.cpp
  CacheableVector.hpp
  CacheFactory.cpp
  CacheFactory.hpp
  CacheListenerAdapter.hpp
  CacheStatistics.cpp
  CacheStatistics.hpp
  CacheTransactionManager.cpp
  CacheTransactionManager.hpp
  CacheWriterAdapter.hpp
  CMakeLists.txt
  CqAttributes.cpp
  CqAttributes.hpp
  CqAttributesFactory.cpp
  CqAttributesFactory.hpp
  CqAttributesMutator.cpp
  CqAttributesMutator.hpp
  CqEvent.cpp
  CqEvent.hpp
  CqOperation.hpp
  CqQuery.cpp
  CqQuery.hpp
  CqServiceStatistics.cpp
  CqServiceStatistics.hpp
  CqState.hpp
  CqStatistics.cpp
  CqStatistics.hpp
  DataInput.cpp
  DataInput.hpp
  DataOutput.cpp
  DataOutput.hpp
  DiskPolicyType.hpp
  DistributedSystem.cpp
  DistributedSystem.hpp
  end_native.hpp
  EntryEvent.cpp
  EntryEvent.hpp
  ExceptionTypes.cpp
  ExceptionTypes.hpp
  Execution.cpp
  Execution.hpp
  ExpirationAction.hpp
  FunctionService.cpp
  FunctionService.hpp
  GeodeClassIds.hpp
  geode_defs.hpp
  geode_includes.cpp
  geode_includes.hpp
  IAuthInitialize.hpp
  ICacheableKey.hpp
  ICacheListener.hpp
  ICacheLoader.hpp
  ICacheWriter.hpp
  ICqAttributes.hpp
  ICqEvent.hpp
  ICqListener.hpp
  ICqResults.hpp
  ICqStatusListener.hpp
  IDataSerializable.hpp
  IDataSerializableFixedId.hpp
  IDataSerializableInternal.hpp
  IDataSerializablePrimitive.hpp
  IFixedPartitionResolver.hpp
  IGeodeCache.hpp
  IDelta.hpp
  ISerializable.hpp
  IPartitionResolver.hpp
  IPdxInstance.hpp
  IPdxInstanceFactory.hpp
  IPdxReader.hpp
  IPdxSerializable.hpp
  IPdxSerializer.hpp
  IPdxTypeMapper.hpp
  IPdxUnreadFields.hpp
  IPdxWriter.hpp
  IPersistenceManager.hpp
  IRegion.hpp
  IRegionService.hpp
  IResultCollector.hpp
  ISelectResults.hpp
  ISubscriptionService.hpp
  ITransactionListener.hpp
  ITransactionWriter.hpp
  IWritablePdxInstance.hpp
  LocalRegion.cpp
  LocalRegion.hpp
  Log.cpp
  Log.hpp
  ManagedDataSerializableHandler.hpp
  ManagedPdxTypeHandler.hpp
  native_conditional_shared_ptr.hpp
  native_conditional_unique_ptr.hpp
  native_shared_ptr.hpp
  native_unique_ptr.hpp
  PdxIdentityFieldAttribute.hpp
  Pool.cpp
  Pool.hpp
  PoolFactory.cpp
  PoolFactory.hpp
  PoolManager.cpp
  PoolManager.hpp
  Properties.cpp
  Properties.hpp
  Query.cpp
  Query.hpp
  QueryService.cpp
  QueryService.hpp
  ReflectionBasedAutoSerializer.cpp
  ReflectionBasedAutoSerializer.hpp
  Region.cpp
  Region.hpp
  RegionAttributes.cpp
  RegionAttributes.hpp
  RegionAttributesFactory.cpp
  RegionAttributesFactory.hpp
  RegionEntry.cpp
  RegionEntry.hpp
  RegionEvent.cpp
  RegionEvent.hpp
  RegionFactory.cpp
  RegionFactory.hpp
  RegionShortcut.hpp
  ResultCollector.cpp
  ResultCollector.hpp
  ResultSet.cpp
  ResultSet.hpp
  SelectResultsIterator.cpp
  SelectResultsIterator.hpp
  Serializable.cpp
  Serializable.hpp
  Struct.cpp
  Struct.hpp
  StructSet.cpp
  StructSet.hpp
  SystemProperties.cpp
  SystemProperties.hpp
  TimeUtils.hpp
  TransactionEvent.cpp
  TransactionEvent.hpp
  TransactionId.hpp
  TransactionListenerAdapter.hpp
  TransactionWriterAdapter.hpp
  TypeRegistry.cpp
  TypeRegistry.hpp
  UserFunctionExecutionException.cpp
  UserFunctionExecutionException.hpp
  Utils.cpp
  Utils.hpp
  impl/AppDomainContext.cpp
  impl/AppDomainContext.hpp
  impl/AssemblyInfo.cpp.in
  impl/AuthenticatedView.cpp
  impl/AuthenticatedView.hpp
  impl/CacheListener.hpp
  impl/CacheLoader.hpp
  impl/CacheResolver.cpp
  impl/CacheResolver.hpp
  impl/CacheWriter.hpp
  impl/CliCallbackDelgate.cpp
  impl/CliCallbackDelgate.hpp
  impl/CqListenerProxy.hpp
  impl/CqStatusListenerProxy.hpp
  impl/DelegateWrapper.hpp
  impl/DotNetTypes.hpp
  impl/EnumInfo.cpp
  impl/EnumInfo.hpp
  impl/FixedPartitionResolver.hpp
  impl/GeodeDataInputStream.hpp
  impl/GeodeDataOutputStream.hpp
  impl/GeodeNullStream.hpp
  impl/ManagedAuthInitialize.cpp
  impl/ManagedAuthInitialize.hpp
  impl/ManagedCacheableDelta.cpp
  impl/ManagedCacheableDelta.hpp
  impl/ManagedCacheableKey.cpp
  impl/ManagedCacheableKey.hpp
  impl/ManagedCacheListener.cpp
  impl/ManagedCacheListener.hpp
  impl/ManagedCacheLoader.cpp
  impl/ManagedCacheLoader.hpp
  impl/ManagedCacheWriter.cpp
  impl/ManagedCacheWriter.hpp
  impl/ManagedCqListener.cpp
  impl/ManagedCqListener.hpp
  impl/ManagedCqStatusListener.cpp
  impl/ManagedCqStatusListener.hpp
  impl/ManagedFixedPartitionResolver.cpp
  impl/ManagedFixedPartitionResolver.hpp
  impl/ManagedPartitionResolver.cpp
  impl/ManagedPartitionResolver.hpp
  impl/ManagedPersistenceManager.cpp
  impl/ManagedPersistenceManager.hpp
  impl/ManagedResultCollector.cpp
  impl/ManagedResultCollector.hpp
  impl/ManagedString.hpp
  impl/ManagedTransactionListener.cpp
  impl/ManagedTransactionListener.hpp
  impl/ManagedTransactionWriter.cpp
  impl/ManagedTransactionWriter.hpp
  impl/ManagedVisitor.cpp
  impl/ManagedVisitor.hpp
  impl/MemoryPressureHandler.cpp
  impl/MemoryPressureHandler.hpp
  impl/PartitionResolver.hpp
  impl/PdxFieldType.cpp
  impl/PdxFieldType.hpp
  impl/PdxHelper.cpp
  impl/PdxHelper.hpp
  impl/PdxInstanceFactoryImpl.cpp
  impl/PdxInstanceFactoryImpl.hpp
  impl/PdxInstanceImpl.cpp
  impl/PdxInstanceImpl.hpp
  impl/PdxLocalReader.cpp
  impl/PdxLocalReader.hpp
  impl/PdxLocalWriter.cpp
  impl/PdxLocalWriter.hpp
  impl/PdxManagedCacheableKey.cpp
  impl/PdxManagedCacheableKey.hpp
  impl/PdxReaderWithTypeCollector.cpp
  impl/PdxReaderWithTypeCollector.hpp
  impl/PdxRemotePreservedData.hpp
  impl/PdxRemoteReader.cpp
  impl/PdxRemoteReader.hpp
  impl/PdxRemoteWriter.cpp
  impl/PdxRemoteWriter.hpp
  impl/PdxType.cpp
  impl/PdxType.hpp
  impl/PdxTypeRegistry.cpp
  impl/PdxTypeRegistry.hpp
  impl/PdxWrapper.hpp
  impl/PdxWriterWithTypeCollector.cpp
  impl/PdxWriterWithTypeCollector.hpp
  impl/PersistenceManagerProxy.hpp
  impl/RegionImpl.cpp
  impl/RegionImpl.hpp
  impl/ResultCollectorProxy.hpp
  impl/SafeConvert.hpp
  impl/TransactionListener.hpp
  impl/TransactionWriter.hpp
  impl/WeakhashMap.hpp

  ${CONFIGURE_OUT_FILES}
)

set_target_properties(Apache.Geode PROPERTIES
  COMMON_LANGUAGE_RUNTIME "" 
  OUTPUT_NAME ${PRODUCT_DLL_NAME}
  VS_GLOBAL_CLRSupport "true"
  VS_GLOBAL_KEYWORD "ManagedCProj"
  VS_GLOBAL_PROJECT_TYPES "{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}"
  VS_GLOBAL_ROOTNAMESPACE ${PROJECT_NAME}
  VS_DOTNET_REFERENCES "System;System.Xml;System.Core"
  FOLDER cli
)

target_compile_options(Apache.Geode
  PRIVATE
	/bigobj # C1128 - large number of templates causes too many section.
	/Zm32   # C3859 - internal heap limit reached
	#/doc # Documents to xml
	# warnings as error
	/we4488 # interfaces fully implemented.
	# disabled warnings
	/wd4947 /wd4251 /wd4635
)

target_include_directories(Apache.Geode
  PRIVATE
  	$<TARGET_PROPERTY:apache-geode,SOURCE_DIR>/../src
)

target_link_libraries(Apache.Geode
  PUBLIC
    psapi
  PRIVATE
    c++cli
    c++11
    apache-geode-static
	_WarningsAsError
)

target_link_options(Apache.Geode
  PRIVATE
    /DELAYLOAD:libssl-1_1-x64.dll
    /DELAYLOAD:libcrypto-1_1-x64.dll
)

string(REPLACE "/RTC1" "" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")

set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${SHARED_LINKER_FLAGS_STRONG_KEY}")


include(PrecompiledHeader)
add_precompiled_header(Apache.Geode geode_includes.hpp FORCEINCLUDE)

install(TARGETS Apache.Geode
  RUNTIME DESTINATION bin
  ARCHIVE DESTINATION lib
)

install ( 
  FILES $<TARGET_PDB_FILE:${PROJECT_NAME}> 
  DESTINATION bin 
  CONFIGURATIONS Debug RelWithDebInfo 
)

add_dependencies(client-libraries Apache.Geode)

add_subdirectory(templates)


# For Visual Studio organization
source_group("Configure In Files" FILES ${CONFIGURE_IN_FILES})
source_group("Configure Out Files" FILES ${CONFIGURE_OUT_FILES})
