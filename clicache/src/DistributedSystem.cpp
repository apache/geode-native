/*
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


#include "begin_native.hpp"
#include <version.h>
#include <geode/CacheLoader.hpp>
#include <geode/CacheListener.hpp>
#include <geode/FixedPartitionResolver.hpp>
#include <geode/CacheWriter.hpp>
#include <geode/GeodeTypeIds.hpp>
#include <geode/Cache.hpp>
#include <CacheImpl.hpp>
#include <CacheXmlParser.hpp>
#include <DistributedSystemImpl.hpp>
#include "end_native.hpp"

#include "Cache.hpp"
#include "Serializable.hpp"
#include "DistributedSystem.hpp"
#include "SystemProperties.hpp"
#include "CacheFactory.hpp"
#include "CacheableDate.hpp"
#include "CacheableFileName.hpp"
#include "CacheableHashMap.hpp"
#include "CacheableHashSet.hpp"
#include "CacheableHashTable.hpp"
#include "CacheableIdentityHashMap.hpp"
#include "CacheableObjectArray.hpp"
#include "CacheableString.hpp"
#include "CacheableStringArray.hpp"
#include "CacheableUndefined.hpp"
#include "CacheableVector.hpp"
#include "CacheableArrayList.hpp"
#include "CacheableStack.hpp"
#include "CacheableObject.hpp"
#include "CacheableObjectXml.hpp"
#include "CacheableBuiltins.hpp"
#include "Log.hpp"
#include "Struct.hpp"
#include "impl/MemoryPressureHandler.hpp"
#include "impl/SafeConvert.hpp"
#include "impl/PdxType.hpp"
#include "impl/EnumInfo.hpp"
#include "impl/ManagedPersistenceManager.hpp"

// disable spurious warning
#pragma warning(disable:4091)
#include <msclr/lock.h>
#pragma warning(default:4091)


using namespace System;

using namespace Apache::Geode::Client;

namespace apache
{
  namespace geode
  {
    namespace client
    {
      class ManagedCacheLoaderGeneric
        : public CacheLoader
      {
      public:

        static CacheLoader* create(const char* assemblyPath,
                                   const char* factoryFunctionName);
      };

      class ManagedCacheListenerGeneric
        : public CacheListener
      {
      public:

        static CacheListener* create(const char* assemblyPath,
                                     const char* factoryFunctionName);
      };

      class ManagedFixedPartitionResolverGeneric
        : public FixedPartitionResolver
      {
      public:

        static PartitionResolver* create(const char* assemblyPath,
                                         const char* factoryFunctionName);
      };

      class ManagedCacheWriterGeneric
        : public CacheWriter
      {
      public:

        static CacheWriter* create(const char* assemblyPath,
                                   const char* factoryFunctionName);
      };

    }  // namespace client
  }  // namespace geode
}  // namespace apache


namespace Apache
{
  namespace Geode
  {
    namespace Client
    {


      namespace native = apache::geode::client;

      DistributedSystem^ DistributedSystem::Connect(String^ name, Cache^ cache)
      {
        return DistributedSystem::Connect(name, nullptr, cache);
      }

      DistributedSystem^ DistributedSystem::Connect(String^ name, Properties<String^, String^>^ config, Cache ^ cache)
      {
        // TODO AppDomain should we be able to create a DS directly?
        _GF_MG_EXCEPTION_TRY2

        auto nativeDistributedSystem = native::DistributedSystem::create(marshal_as<std::string>(name),
                                                           config->GetNative());
        nativeDistributedSystem.connect();

        ManagedPostConnect(cache);

        return gcnew DistributedSystem(std::unique_ptr<native::DistributedSystem>(
            new native::DistributedSystem(std::move(nativeDistributedSystem))));

        _GF_MG_EXCEPTION_CATCH_ALL2
      }

      void DistributedSystem::Disconnect(Cache^ cache)
      {
        _GF_MG_EXCEPTION_TRY2


        TypeRegistry::UnregisterNativesGeneric(cache);
        DistributedSystem::UnregisterBuiltinManagedTypes(cache);
        m_nativeDistributedSystem->get()->disconnect();
        GC::KeepAlive(m_nativeDistributedSystem);

        _GF_MG_EXCEPTION_CATCH_ALL2
      }

      void DistributedSystem::AppDomainInstanceInitialization(Cache^ cache)

        // TODO AppDomain move this.

      {
        _GF_MG_EXCEPTION_TRY2

          // Register wrapper types for built-in types, this are still cpp wrapper

        //byte
        TypeRegistry::RegisterWrapperGeneric(
        gcnew WrapperDelegateGeneric(CacheableByte::Create),
        native::GeodeTypeIds::CacheableByte, SByte::typeid);

        //boolean
        TypeRegistry::RegisterWrapperGeneric(
          gcnew WrapperDelegateGeneric(CacheableBoolean::Create),
          native::GeodeTypeIds::CacheableBoolean, Boolean::typeid);
        //wide char
        TypeRegistry::RegisterWrapperGeneric(
          gcnew WrapperDelegateGeneric(CacheableCharacter::Create),
          native::GeodeTypeIds::CacheableCharacter, Char::typeid);
        //double
        TypeRegistry::RegisterWrapperGeneric(
          gcnew WrapperDelegateGeneric(CacheableDouble::Create),
          native::GeodeTypeIds::CacheableDouble, Double::typeid);
        //ascii string
        TypeRegistry::RegisterWrapperGeneric(
          gcnew WrapperDelegateGeneric(CacheableString::Create),
          native::GeodeTypeIds::CacheableASCIIString, String::typeid);

        TypeRegistry::RegisterWrapperGeneric(
          gcnew WrapperDelegateGeneric(CacheableFloat::Create),
          native::GeodeTypeIds::CacheableFloat, float::typeid);
        //int 16
        TypeRegistry::RegisterWrapperGeneric(
          gcnew WrapperDelegateGeneric(CacheableInt16::Create),
          native::GeodeTypeIds::CacheableInt16, Int16::typeid);
        //int32
        TypeRegistry::RegisterWrapperGeneric(
          gcnew WrapperDelegateGeneric(CacheableInt32::Create),
          native::GeodeTypeIds::CacheableInt32, Int32::typeid);
        //int64
        TypeRegistry::RegisterWrapperGeneric(
          gcnew WrapperDelegateGeneric(CacheableInt64::Create),
          native::GeodeTypeIds::CacheableInt64, Int64::typeid);

        //Now onwards all will be wrap in managed cacheable key..

        cache->TypeRegistry->RegisterType(
          native::GeodeTypeIds::CacheableBytes,
          gcnew TypeFactoryMethod(CacheableBytes::CreateDeserializable),
          Type::GetType("System.Byte[]"));

        cache->TypeRegistry->RegisterType(
          native::GeodeTypeIds::CacheableDoubleArray,
          gcnew TypeFactoryMethod(CacheableDoubleArray::CreateDeserializable),
          Type::GetType("System.Double[]"));

        cache->TypeRegistry->RegisterType(
          native::GeodeTypeIds::CacheableFloatArray,
          gcnew TypeFactoryMethod(CacheableFloatArray::CreateDeserializable),
          Type::GetType("System.Single[]"));

        //TODO:
        //as it is
        cache->TypeRegistry->RegisterType(
          native::GeodeTypeIds::CacheableHashSet,
          gcnew TypeFactoryMethod(CacheableHashSet::CreateDeserializable),
          nullptr);

        //as it is
        cache->TypeRegistry->RegisterType(
          native::GeodeTypeIds::CacheableLinkedHashSet,
          gcnew TypeFactoryMethod(CacheableLinkedHashSet::CreateDeserializable),
          nullptr);


        cache->TypeRegistry->RegisterType(
          native::GeodeTypeIds::CacheableInt16Array,
          gcnew TypeFactoryMethod(CacheableInt16Array::CreateDeserializable),
          Type::GetType("System.Int16[]"));

        cache->TypeRegistry->RegisterType(
          native::GeodeTypeIds::CacheableInt32Array,
          gcnew TypeFactoryMethod(CacheableInt32Array::CreateDeserializable),
          Type::GetType("System.Int32[]"));


        cache->TypeRegistry->RegisterType(
          native::GeodeTypeIds::CacheableInt64Array,
          gcnew TypeFactoryMethod(CacheableInt64Array::CreateDeserializable),
          Type::GetType("System.Int64[]"));

        cache->TypeRegistry->RegisterType(
          native::GeodeTypeIds::BooleanArray,
          gcnew TypeFactoryMethod(BooleanArray::CreateDeserializable),
          Type::GetType("System.Boolean[]"));

        cache->TypeRegistry->RegisterType(
          native::GeodeTypeIds::CharArray,
          gcnew TypeFactoryMethod(CharArray::CreateDeserializable),
          Type::GetType("System.Char[]"));

        cache->TypeRegistry->RegisterType(
          native::GeodeTypeIds::CacheableStringArray,
          gcnew TypeFactoryMethod(CacheableStringArray::CreateDeserializable),
          Type::GetType("System.String[]"));

        //as it is
        cache->TypeRegistry->RegisterType(
          native::GeodeTypeIds::Struct,
          gcnew TypeFactoryMethod(Struct::CreateDeserializable),
          nullptr);

        cache->TypeRegistry->RegisterType(
          native::GeodeTypeIds::EnumInfo,
          gcnew TypeFactoryMethod(Apache::Geode::Client::Internal::EnumInfo::CreateDeserializable),
          nullptr);

        // End register generic wrapper types for built-in types

        //if (!native::DistributedSystem::isConnected())
        //{

          // Set the Generic ManagedCacheLoader/Listener/Writer factory functions.
          native::CacheXmlParser::managedCacheLoaderFn =
            native::ManagedCacheLoaderGeneric::create;
          native::CacheXmlParser::managedCacheListenerFn =
            native::ManagedCacheListenerGeneric::create;
          native::CacheXmlParser::managedCacheWriterFn =
            native::ManagedCacheWriterGeneric::create;

          // Set the Generic ManagedPartitionResolver factory function
          native::CacheXmlParser::managedPartitionResolverFn =
            native::ManagedFixedPartitionResolverGeneric::create;

          // Set the Generic ManagedPersistanceManager factory function
          native::CacheXmlParser::managedPersistenceManagerFn =
            native::ManagedPersistenceManagerGeneric::create;
        //}

        _GF_MG_EXCEPTION_CATCH_ALL2
      }

      void DistributedSystem::ManagedPostConnect(Cache^ cache)
      {
        //  The registration into the native map should be after
        // native connect since it can be invoked only once

        // Register other built-in types

        // End register other built-in types

        // Register other built-in types for generics
        //c# datatime

        cache->TypeRegistry->RegisterType(
          native::GeodeTypeIds::CacheableDate,
          gcnew TypeFactoryMethod(CacheableDate::CreateDeserializable),
          Type::GetType("System.DateTime"));

        //as it is
        cache->TypeRegistry->RegisterType(
          native::GeodeTypeIds::CacheableFileName,
          gcnew TypeFactoryMethod(CacheableFileName::CreateDeserializable),
          nullptr);

        //for generic dictionary define its type in static constructor of Serializable.hpp
        cache->TypeRegistry->RegisterType(
          native::GeodeTypeIds::CacheableHashMap,
          gcnew TypeFactoryMethod(CacheableHashMap::CreateDeserializable),
          nullptr);

        //c# hashtable
        cache->TypeRegistry->RegisterType(
          native::GeodeTypeIds::CacheableHashTable,
          gcnew TypeFactoryMethod(CacheableHashTable::CreateDeserializable),
          Type::GetType("System.Collections.Hashtable"));

        //Need to keep public as no counterpart in c#
        cache->TypeRegistry->RegisterType(
          native::GeodeTypeIds::CacheableIdentityHashMap,
          gcnew TypeFactoryMethod(
          CacheableIdentityHashMap::CreateDeserializable),
          nullptr);

        //keep as it is
        cache->TypeRegistry->RegisterType(
          native::GeodeTypeIds::CacheableUndefined,
          gcnew TypeFactoryMethod(CacheableUndefined::CreateDeserializable),
          nullptr);

        //c# arraylist
        cache->TypeRegistry->RegisterType(
          native::GeodeTypeIds::CacheableVector,
          gcnew TypeFactoryMethod(CacheableVector::CreateDeserializable),
          nullptr);

        //as it is
        cache->TypeRegistry->RegisterType(
          native::GeodeTypeIds::CacheableObjectArray,
          gcnew TypeFactoryMethod(
          CacheableObjectArray::CreateDeserializable),
          nullptr);

        //Generic::List
        cache->TypeRegistry->RegisterType(
          native::GeodeTypeIds::CacheableArrayList,
          gcnew TypeFactoryMethod(CacheableArrayList::CreateDeserializable),
          nullptr);

        //c# generic stack
        cache->TypeRegistry->RegisterType(
          native::GeodeTypeIds::CacheableStack,
          gcnew TypeFactoryMethod(CacheableStack::CreateDeserializable),
          nullptr);

        //as it is
        cache->TypeRegistry->RegisterType(
          GeodeClassIds::CacheableManagedObject - 0x80000000,
          gcnew TypeFactoryMethod(CacheableObject::CreateDeserializable),
          nullptr);

        //as it is
        cache->TypeRegistry->RegisterType(
          GeodeClassIds::CacheableManagedObjectXml - 0x80000000,
          gcnew TypeFactoryMethod(CacheableObjectXml::CreateDeserializable),
          nullptr);

        // End register other built-in types

        // Log the version of the C# layer being used
        Log::Config(".NET layer assembly version: {0}({1})", System::Reflection::
                    Assembly::GetExecutingAssembly()->GetName()->Version->ToString(),
                    System::Reflection::Assembly::GetExecutingAssembly()->ImageRuntimeVersion);

        Log::Config(".NET runtime version: {0} ", System::Environment::Version);
        Log::Config(".NET AppDomain: {0} - {1}",
          System::AppDomain::CurrentDomain->Id,
          System::AppDomain::CurrentDomain->FriendlyName);
      }

      void DistributedSystem::UnregisterBuiltinManagedTypes(Cache^ cache)
      {
        _GF_MG_EXCEPTION_TRY2

          TypeRegistry::UnregisterNativesGeneric(cache);

          cache->TypeRegistry->UnregisterTypeGeneric(
            native::GeodeTypeIds::CacheableDate);
          cache->TypeRegistry->UnregisterTypeGeneric(
            native::GeodeTypeIds::CacheableFileName);
          cache->TypeRegistry->UnregisterTypeGeneric(
            native::GeodeTypeIds::CacheableHashMap);
          cache->TypeRegistry->UnregisterTypeGeneric(
            native::GeodeTypeIds::CacheableHashTable);
          cache->TypeRegistry->UnregisterTypeGeneric(
            native::GeodeTypeIds::CacheableIdentityHashMap);
          cache->TypeRegistry->UnregisterTypeGeneric(
            native::GeodeTypeIds::CacheableVector);
          cache->TypeRegistry->UnregisterTypeGeneric(
            native::GeodeTypeIds::CacheableObjectArray);
          cache->TypeRegistry->UnregisterTypeGeneric(
            native::GeodeTypeIds::CacheableArrayList);
          cache->TypeRegistry->UnregisterTypeGeneric(
            native::GeodeTypeIds::CacheableStack);
          cache->TypeRegistry->UnregisterTypeGeneric(
            GeodeClassIds::CacheableManagedObject - 0x80000000);
          cache->TypeRegistry->UnregisterTypeGeneric(
            GeodeClassIds::CacheableManagedObjectXml - 0x80000000);

        _GF_MG_EXCEPTION_CATCH_ALL2
      }

      Apache::Geode::Client::SystemProperties^ DistributedSystem::SystemProperties::get()
      {
        _GF_MG_EXCEPTION_TRY2

          return Apache::Geode::Client::SystemProperties::Create(
          &(m_nativeDistributedSystem->get()->getSystemProperties()));

        _GF_MG_EXCEPTION_CATCH_ALL2
      }

      String^ DistributedSystem::Name::get()
      {
        try
        {
          return marshal_as<String^>(m_nativeDistributedSystem->get()->getName());
        }
        finally
        {
          GC::KeepAlive(m_nativeDistributedSystem);
        }
      }

      void DistributedSystem::HandleMemoryPressure(System::Object^ state)
      {
        // TODO global - Need single memory pressue event running?
        ACE_Time_Value dummy(1);
        MemoryPressureHandler handler;
        handler.handle_timeout(dummy, nullptr);
      }

      DistributedSystem^ DistributedSystem::Create(native::DistributedSystem* nativeptr)
      {
        auto instance = gcnew DistributedSystem(nativeptr);
        return instance;
      }

      DistributedSystem::DistributedSystem(std::unique_ptr<native::DistributedSystem> nativeDistributedSystem)
      {
        m_nativeDistributedSystem = gcnew native_conditional_unique_ptr<native::DistributedSystem>(std::move(nativeDistributedSystem));
      }

      DistributedSystem::DistributedSystem(native::DistributedSystem* nativeDistributedSystem)
      {
        m_nativeDistributedSystem = gcnew native_conditional_unique_ptr<native::DistributedSystem>(nativeDistributedSystem);
      }

      DistributedSystem::~DistributedSystem()
      {
        m_memoryPressureHandler->Dispose(nullptr);
      }

      void DistributedSystem::registerCliCallback()
      {
        m_cliCallBackObj = gcnew CliCallbackDelegate();
        auto nativeCallback =
          gcnew cliCallback(m_cliCallBackObj,
          &CliCallbackDelegate::Callback);

        native::DistributedSystemImpl::registerCliCallback(System::Threading::Thread::GetDomainID(),
                                                                          (void (*)(apache::geode::client::Cache &cache))System::Runtime::InteropServices::
                                                                          Marshal::GetFunctionPointerForDelegate(
                                                                          nativeCallback).ToPointer());
      }

      void DistributedSystem::unregisterCliCallback()
      {
        native::DistributedSystemImpl::unregisterCliCallback(System::Threading::Thread::GetDomainID());
      }
    }  // namespace Client
  }  // namespace Geode
}  // namespace Apache
