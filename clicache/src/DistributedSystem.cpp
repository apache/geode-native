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
        RegisterDataSerializablePrimitivesOverrideNativeDeserialization(cache);
      }

      void DistributedSystem::RegisterDataSerializablePrimitivesOverrideNativeDeserialization(Cache^ cache)
      {
        // Registers overrides in the C++ layer to incercept deserialization into managed layer.

        auto typeRegistry = cache->TypeRegistry;

        typeRegistry->RegisterDataSerializablePrimitiveOverrideNativeDeserialization(
            static_cast<int8_t>(native::internal::DSCode::CacheableBytes),
            gcnew TypeFactoryMethod(CacheableBytes::CreateDeserializable),
            Type::GetType("System.Byte[]"));

        typeRegistry->RegisterDataSerializablePrimitiveOverrideNativeDeserialization(
            static_cast<int8_t>(native::internal::DSCode::CacheableDoubleArray),
            gcnew TypeFactoryMethod(CacheableDoubleArray::CreateDeserializable),
            Type::GetType("System.Double[]"));

        typeRegistry->RegisterDataSerializablePrimitiveOverrideNativeDeserialization(
            static_cast<int8_t>(native::internal::DSCode::CacheableFloatArray),
            gcnew TypeFactoryMethod(CacheableFloatArray::CreateDeserializable),
            Type::GetType("System.Single[]"));

        typeRegistry->RegisterDataSerializablePrimitiveOverrideNativeDeserialization(
            static_cast<int8_t>(native::internal::DSCode::CacheableHashSet),
            gcnew TypeFactoryMethod(CacheableHashSet::CreateDeserializable),
            nullptr);

        typeRegistry->RegisterDataSerializablePrimitiveOverrideNativeDeserialization(
            static_cast<int8_t>(native::internal::DSCode::CacheableLinkedHashSet),
            gcnew TypeFactoryMethod(CacheableLinkedHashSet::CreateDeserializable),
            nullptr);

        typeRegistry->RegisterDataSerializablePrimitiveOverrideNativeDeserialization(
            static_cast<int8_t>(native::internal::DSCode::CacheableInt16Array),
            gcnew TypeFactoryMethod(CacheableInt16Array::CreateDeserializable),
            Type::GetType("System.Int16[]"));

        typeRegistry->RegisterDataSerializablePrimitiveOverrideNativeDeserialization(
            static_cast<int8_t>(native::internal::DSCode::CacheableInt32Array),
            gcnew TypeFactoryMethod(CacheableInt32Array::CreateDeserializable),
            Type::GetType("System.Int32[]"));

        typeRegistry->RegisterDataSerializablePrimitiveOverrideNativeDeserialization(
            static_cast<int8_t>(native::internal::DSCode::CacheableInt64Array),
            gcnew TypeFactoryMethod(CacheableInt64Array::CreateDeserializable),
            Type::GetType("System.Int64[]"));

        typeRegistry->RegisterDataSerializablePrimitiveOverrideNativeDeserialization(
            static_cast<int8_t>(native::internal::DSCode::BooleanArray),
            gcnew TypeFactoryMethod(BooleanArray::CreateDeserializable),
            Type::GetType("System.Boolean[]"));

        typeRegistry->RegisterDataSerializablePrimitiveOverrideNativeDeserialization(
            static_cast<int8_t>(native::internal::DSCode::CharArray),
            gcnew TypeFactoryMethod(CharArray::CreateDeserializable),
            Type::GetType("System.Char[]"));

        typeRegistry->RegisterDataSerializablePrimitiveOverrideNativeDeserialization(
            static_cast<int8_t>(native::internal::DSCode::CacheableStringArray),
            gcnew TypeFactoryMethod(CacheableStringArray::CreateDeserializable),
            Type::GetType("System.String[]"));

        typeRegistry->RegisterDataSerializablePrimitiveOverrideNativeDeserialization(
            static_cast<int8_t>(native::DSFid::Struct),
            gcnew TypeFactoryMethod(Struct::CreateDeserializable),
            nullptr);
        
        typeRegistry->RegisterDataSerializablePrimitiveOverrideNativeDeserialization(
            static_cast<int8_t>(native::internal::DSCode::CacheableDate),
            gcnew TypeFactoryMethod(CacheableDate::CreateDeserializable),
            Type::GetType("System.DateTime"));

        typeRegistry->RegisterDataSerializablePrimitiveOverrideNativeDeserialization(
            static_cast<int8_t>(native::internal::DSCode::CacheableFileName),
            gcnew TypeFactoryMethod(CacheableFileName::CreateDeserializable),
            nullptr);

        typeRegistry->RegisterDataSerializablePrimitiveOverrideNativeDeserialization(
            static_cast<int8_t>(native::internal::DSCode::CacheableHashMap),
            gcnew TypeFactoryMethod(CacheableHashMap::CreateDeserializable),
            nullptr);

        typeRegistry->RegisterDataSerializablePrimitiveOverrideNativeDeserialization(
            static_cast<int8_t>(native::internal::DSCode::CacheableHashTable),
            gcnew TypeFactoryMethod(CacheableHashTable::CreateDeserializable),
            Type::GetType("System.Collections.Hashtable"));

        typeRegistry->RegisterDataSerializablePrimitiveOverrideNativeDeserialization(
            static_cast<int8_t>(native::internal::DSCode::CacheableIdentityHashMap),
            gcnew TypeFactoryMethod(CacheableIdentityHashMap::CreateDeserializable),
            nullptr);

        typeRegistry->RegisterDataSerializablePrimitiveOverrideNativeDeserialization(
            static_cast<int8_t>(native::DSFid::CacheableUndefined),
            gcnew TypeFactoryMethod(CacheableUndefined::CreateDeserializable),
            nullptr);

        typeRegistry->RegisterDataSerializablePrimitiveOverrideNativeDeserialization(
            static_cast<int8_t>(native::internal::DSCode::CacheableVector),
            gcnew TypeFactoryMethod(CacheableVector::CreateDeserializable),
            nullptr);

        typeRegistry->RegisterDataSerializablePrimitiveOverrideNativeDeserialization(
            static_cast<int8_t>(native::internal::DSCode::CacheableObjectArray),
            gcnew TypeFactoryMethod(CacheableObjectArray::CreateDeserializable),
            nullptr);

        typeRegistry->RegisterDataSerializablePrimitiveOverrideNativeDeserialization(
            static_cast<int8_t>(native::internal::DSCode::CacheableArrayList),
            gcnew TypeFactoryMethod(CacheableArrayList::CreateDeserializable),
            nullptr);

        typeRegistry->RegisterDataSerializablePrimitiveOverrideNativeDeserialization(
            static_cast<int8_t>(native::internal::DSCode::CacheableLinkedList),
            gcnew TypeFactoryMethod(CacheableLinkedList::CreateDeserializable),
            nullptr);

        typeRegistry->RegisterDataSerializablePrimitiveOverrideNativeDeserialization(
            static_cast<int8_t>(native::internal::DSCode::CacheableStack),
            gcnew TypeFactoryMethod(CacheableStack::CreateDeserializable),
            nullptr);

        typeRegistry->RegisterDataSerializablePrimitiveOverrideNativeDeserialization(
            static_cast<int8_t>(native::internal::InternalId::CacheableManagedObject),
            gcnew TypeFactoryMethod(CacheableObject::CreateDeserializable),
            nullptr);

        typeRegistry->RegisterDataSerializablePrimitiveOverrideNativeDeserialization(
            static_cast<int8_t>(native::internal::InternalId::CacheableManagedObjectXml),
            gcnew TypeFactoryMethod(CacheableObjectXml::CreateDeserializable),
            nullptr);
      }

      void DistributedSystem::RegisterDataSerializableFixedIdsOverrideNativeDeserialization(Cache^ cache)
      {
        auto typeRegistry = cache->TypeRegistry;
 
        typeRegistry->RegisterDataSerializableFixedIdTypeOverrideNativeDeserialization(
            static_cast<int8_t>(native::DSFid::EnumInfo),
            gcnew TypeFactoryMethod(Internal::EnumInfo::CreateDeserializable));
      }

      void DistributedSystem::AppDomainInstanceInitialization(Cache^ cache)
      {
        RegisterDataSerializablePrimitives(cache);
        RegisterDataSerializableFixedIdsOverrideNativeDeserialization(cache);

        // Actually an internal type being registered as a primitive
        cache->TypeRegistry->RegisterDataSerializablePrimitiveOverrideNativeDeserialization(
            static_cast<int8_t>(native::internal::DSCode::PdxType),
            gcnew TypeFactoryMethod(Apache::Geode::Client::Internal::PdxType::CreateDeserializable),
            nullptr);

        _GF_MG_EXCEPTION_TRY2

          // Set the Generic ManagedCacheLoader/Listener/Writer factory functions.
          native::CacheXmlParser::managedCacheLoaderFn_ =
            native::ManagedCacheLoaderGeneric::create;
          native::CacheXmlParser::managedCacheListenerFn_ =
            native::ManagedCacheListenerGeneric::create;
          native::CacheXmlParser::managedCacheWriterFn_ =
            native::ManagedCacheWriterGeneric::create;

          // Set the Generic ManagedPartitionResolver factory function
          native::CacheXmlParser::managedPartitionResolverFn_ =
            native::ManagedFixedPartitionResolverGeneric::create;

          // Set the Generic ManagedPersistanceManager factory function
          native::CacheXmlParser::managedPersistenceManagerFn_ =
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
            static_cast<int8_t>(native::internal::DSCode::CacheableDate));
          cache->TypeRegistry->UnregisterTypeGeneric(
            static_cast<int8_t>(native::internal::DSCode::CacheableFileName));
          cache->TypeRegistry->UnregisterTypeGeneric(
            static_cast<int8_t>(native::internal::DSCode::CacheableHashMap));
          cache->TypeRegistry->UnregisterTypeGeneric(
            static_cast<int8_t>(native::internal::DSCode::CacheableHashTable));
          cache->TypeRegistry->UnregisterTypeGeneric(
            static_cast<int8_t>(native::internal::DSCode::CacheableIdentityHashMap));
          cache->TypeRegistry->UnregisterTypeGeneric(
            static_cast<int8_t>(native::internal::DSCode::CacheableVector));
          cache->TypeRegistry->UnregisterTypeGeneric(
            static_cast<int8_t>(native::internal::DSCode::CacheableObjectArray));
          cache->TypeRegistry->UnregisterTypeGeneric(
            static_cast<int8_t>(native::internal::DSCode::CacheableArrayList));
          cache->TypeRegistry->UnregisterTypeGeneric(
            static_cast<int8_t>(native::internal::DSCode::CacheableStack));
          cache->TypeRegistry->UnregisterTypeGeneric(
            static_cast<int8_t>(native::internal::InternalId::CacheableManagedObject));
          cache->TypeRegistry->UnregisterTypeGeneric(
            static_cast<int8_t>(native::internal::InternalId::CacheableManagedObjectXml));

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
