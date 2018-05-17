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
#include "CacheableLinkedList.hpp"
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
          DistributedSystem::UnregisterBuiltinManagedTypes(cache);
          m_nativeDistributedSystem->get()->disconnect();
          GC::KeepAlive(m_nativeDistributedSystem);
        _GF_MG_EXCEPTION_CATCH_ALL2
      }

      void DistributedSystem::RegisterDataSerializablePrimitives(Cache^ cache)
      {
        RegisterDataSerializablePrimitivesWrapNativeDeserialization();
        RegisterDataSerializablePrimitivesOverrideNativeDeserialization(cache);
      }

      void DistributedSystem::RegisterDataSerializablePrimitivesWrapNativeDeserialization()
      {
        // TODO serializable - These appear to be global, either make all prmitive types global or not.
        // Register wrappers for primitive types
        // Does not intercept deserialization

        TypeRegistry::RegisterDataSerializablePrimitiveWrapper(
          gcnew DataSerializablePrimitiveWrapperDelegate(CacheableByte::Create),
          native::GeodeTypeIds::CacheableByte, SByte::typeid);

        TypeRegistry::RegisterDataSerializablePrimitiveWrapper(
          gcnew DataSerializablePrimitiveWrapperDelegate(CacheableBoolean::Create),
          native::GeodeTypeIds::CacheableBoolean, Boolean::typeid);

        TypeRegistry::RegisterDataSerializablePrimitiveWrapper(
          gcnew DataSerializablePrimitiveWrapperDelegate(CacheableCharacter::Create),
          native::GeodeTypeIds::CacheableCharacter, Char::typeid);

        TypeRegistry::RegisterDataSerializablePrimitiveWrapper(
          gcnew DataSerializablePrimitiveWrapperDelegate(CacheableDouble::Create),
          native::GeodeTypeIds::CacheableDouble, Double::typeid);

        TypeRegistry::RegisterDataSerializablePrimitiveWrapper(
          gcnew DataSerializablePrimitiveWrapperDelegate(CacheableString::Create),
          native::GeodeTypeIds::CacheableASCIIString, String::typeid);

        TypeRegistry::RegisterDataSerializablePrimitiveWrapper(
          gcnew DataSerializablePrimitiveWrapperDelegate(CacheableFloat::Create),
          native::GeodeTypeIds::CacheableFloat, float::typeid);

        TypeRegistry::RegisterDataSerializablePrimitiveWrapper(
          gcnew DataSerializablePrimitiveWrapperDelegate(CacheableInt16::Create),
          native::GeodeTypeIds::CacheableInt16, Int16::typeid);

        TypeRegistry::RegisterDataSerializablePrimitiveWrapper(
          gcnew DataSerializablePrimitiveWrapperDelegate(CacheableInt32::Create),
          native::GeodeTypeIds::CacheableInt32, Int32::typeid);

        TypeRegistry::RegisterDataSerializablePrimitiveWrapper(
          gcnew DataSerializablePrimitiveWrapperDelegate(CacheableInt64::Create),
          native::GeodeTypeIds::CacheableInt64, Int64::typeid);
			}

      void DistributedSystem::RegisterDataSerializablePrimitivesOverrideNativeDeserialization(Cache^ cache)
      {
        // TODO serializable - appears to register per cache while other types are global, make consistent.
        // Registers overrides in the C++ layer to incercept deserialization into managed layer.

        auto&& typeRegistry = cache->TypeRegistry;

        typeRegistry->RegisterDataSerializablePrimitiveOverrideNativeDeserialization(
            native::GeodeTypeIds::CacheableBytes,
            gcnew TypeFactoryMethod(CacheableBytes::CreateDeserializable),
            Type::GetType("System.Byte[]"));

        typeRegistry->RegisterDataSerializablePrimitiveOverrideNativeDeserialization(
            native::GeodeTypeIds::CacheableDoubleArray,
            gcnew TypeFactoryMethod(CacheableDoubleArray::CreateDeserializable),
            Type::GetType("System.Double[]"));

        typeRegistry->RegisterDataSerializablePrimitiveOverrideNativeDeserialization(
            native::GeodeTypeIds::CacheableFloatArray,
            gcnew TypeFactoryMethod(CacheableFloatArray::CreateDeserializable),
            Type::GetType("System.Single[]"));

        typeRegistry->RegisterDataSerializablePrimitiveOverrideNativeDeserialization(
            native::GeodeTypeIds::CacheableHashSet,
            gcnew TypeFactoryMethod(CacheableHashSet::CreateDeserializable),
            nullptr);

        typeRegistry->RegisterDataSerializablePrimitiveOverrideNativeDeserialization(
            native::GeodeTypeIds::CacheableLinkedHashSet,
            gcnew TypeFactoryMethod(CacheableLinkedHashSet::CreateDeserializable),
            nullptr);

        typeRegistry->RegisterDataSerializablePrimitiveOverrideNativeDeserialization(
            native::GeodeTypeIds::CacheableInt16Array,
            gcnew TypeFactoryMethod(CacheableInt16Array::CreateDeserializable),
            Type::GetType("System.Int16[]"));

        typeRegistry->RegisterDataSerializablePrimitiveOverrideNativeDeserialization(
            native::GeodeTypeIds::CacheableInt32Array,
            gcnew TypeFactoryMethod(CacheableInt32Array::CreateDeserializable),
            Type::GetType("System.Int32[]"));

        typeRegistry->RegisterDataSerializablePrimitiveOverrideNativeDeserialization(
            native::GeodeTypeIds::CacheableInt64Array,
            gcnew TypeFactoryMethod(CacheableInt64Array::CreateDeserializable),
            Type::GetType("System.Int64[]"));

        typeRegistry->RegisterDataSerializablePrimitiveOverrideNativeDeserialization(
            native::GeodeTypeIds::BooleanArray,
            gcnew TypeFactoryMethod(BooleanArray::CreateDeserializable),
            Type::GetType("System.Boolean[]"));

        typeRegistry->RegisterDataSerializablePrimitiveOverrideNativeDeserialization(
            native::GeodeTypeIds::CharArray,
            gcnew TypeFactoryMethod(CharArray::CreateDeserializable),
            Type::GetType("System.Char[]"));

        typeRegistry->RegisterDataSerializablePrimitiveOverrideNativeDeserialization(
            native::GeodeTypeIds::CacheableStringArray,
            gcnew TypeFactoryMethod(CacheableStringArray::CreateDeserializable),
            Type::GetType("System.String[]"));

        typeRegistry->RegisterDataSerializablePrimitiveOverrideNativeDeserialization(
            native::GeodeTypeIds::Struct,
            gcnew TypeFactoryMethod(Struct::CreateDeserializable),
            nullptr);
        
        typeRegistry->RegisterDataSerializablePrimitiveOverrideNativeDeserialization(
            native::GeodeTypeIds::CacheableDate,
            gcnew TypeFactoryMethod(CacheableDate::CreateDeserializable),
            Type::GetType("System.DateTime"));

        typeRegistry->RegisterDataSerializablePrimitiveOverrideNativeDeserialization(
            native::GeodeTypeIds::CacheableFileName,
            gcnew TypeFactoryMethod(CacheableFileName::CreateDeserializable),
            nullptr);

        typeRegistry->RegisterDataSerializablePrimitiveOverrideNativeDeserialization(
            native::GeodeTypeIds::CacheableHashMap,
            gcnew TypeFactoryMethod(CacheableHashMap::CreateDeserializable),
            nullptr);

        typeRegistry->RegisterDataSerializablePrimitiveOverrideNativeDeserialization(
            native::GeodeTypeIds::CacheableHashTable,
            gcnew TypeFactoryMethod(CacheableHashTable::CreateDeserializable),
            Type::GetType("System.Collections.Hashtable"));

        typeRegistry->RegisterDataSerializablePrimitiveOverrideNativeDeserialization(
            native::GeodeTypeIds::CacheableIdentityHashMap,
            gcnew TypeFactoryMethod(CacheableIdentityHashMap::CreateDeserializable),
            nullptr);

        typeRegistry->RegisterDataSerializablePrimitiveOverrideNativeDeserialization(
            native::GeodeTypeIds::CacheableUndefined,
            gcnew TypeFactoryMethod(CacheableUndefined::CreateDeserializable),
            nullptr);

        typeRegistry->RegisterDataSerializablePrimitiveOverrideNativeDeserialization(
            native::GeodeTypeIds::CacheableVector,
            gcnew TypeFactoryMethod(CacheableVector::CreateDeserializable),
            nullptr);

        typeRegistry->RegisterDataSerializablePrimitiveOverrideNativeDeserialization(
            native::GeodeTypeIds::CacheableObjectArray,
            gcnew TypeFactoryMethod(CacheableObjectArray::CreateDeserializable),
            nullptr);

        typeRegistry->RegisterDataSerializablePrimitiveOverrideNativeDeserialization(
            native::GeodeTypeIds::CacheableArrayList,
            gcnew TypeFactoryMethod(CacheableArrayList::CreateDeserializable),
            nullptr);

        typeRegistry->RegisterDataSerializablePrimitiveOverrideNativeDeserialization(
            native::GeodeTypeIds::CacheableLinkedList,
            gcnew TypeFactoryMethod(CacheableLinkedList::CreateDeserializable),
            nullptr);

        typeRegistry->RegisterDataSerializablePrimitiveOverrideNativeDeserialization(
            native::GeodeTypeIds::CacheableStack,
            gcnew TypeFactoryMethod(CacheableStack::CreateDeserializable),
            nullptr);

        typeRegistry->RegisterDataSerializablePrimitiveOverrideNativeDeserialization(
            native::GeodeTypeIds::CacheableManagedObject,
            gcnew TypeFactoryMethod(CacheableObject::CreateDeserializable),
            nullptr);

        typeRegistry->RegisterDataSerializablePrimitiveOverrideNativeDeserialization(
            native::GeodeTypeIds::CacheableManagedObjectXml,
            gcnew TypeFactoryMethod(CacheableObjectXml::CreateDeserializable),
            nullptr);
      }

      void DistributedSystem::RegisterDataSerializableFixedIdsOverrideNativeDeserialization(Cache^ cache)
      {
        auto&& typeRegistry = cache->TypeRegistry;
 
        typeRegistry->RegisterDataSerializableFixedIdTypeOverrideNativeDeserialization(
            native::GeodeTypeIds::EnumInfo,
            gcnew TypeFactoryMethod(Internal::EnumInfo::CreateDeserializable));
      }

      void DistributedSystem::AppDomainInstanceInitialization(Cache^ cache)
      {
        RegisterDataSerializablePrimitives(cache);
        RegisterDataSerializableFixedIdsOverrideNativeDeserialization(cache);

        // Actually an internal type being registered as a primitive
        cache->TypeRegistry->RegisterDataSerializablePrimitiveOverrideNativeDeserialization(
            native::GeodeTypeIds::PdxType,
            gcnew TypeFactoryMethod(Apache::Geode::Client::Internal::PdxType::CreateDeserializable),
            nullptr);

        _GF_MG_EXCEPTION_TRY2

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

        _GF_MG_EXCEPTION_CATCH_ALL2
      }

      void DistributedSystem::ManagedPostConnect(Cache^ cache)
      {
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
