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
#include <CacheImpl.hpp>
#include <CacheXmlParser.hpp>
#include <DistributedSystemImpl.hpp>
#include <ace/Process.h> // Added to get rid of unresolved token warning
#include "end_native.hpp"

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

      class ManagedAuthInitializeGeneric
        : public AuthInitialize
      {
      public:

        static AuthInitialize* create(const char* assemblyPath,
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

      DistributedSystem^ DistributedSystem::Connect(String^ name)
      {
        return DistributedSystem::Connect(name, nullptr);
      }

      DistributedSystem^ DistributedSystem::Connect(String^ name, Properties<String^, String^>^ config)
      {
        native::DistributedSystemImpl::acquireDisconnectLock();

        _GF_MG_EXCEPTION_TRY2

          ManagedString mg_name(name);

        DistributedSystem::AppDomainInstanceInitialization(config->GetNative());

        // this we are calling after all .NET initialization required in
        // each AppDomain
        auto nativeptr = native::DistributedSystem::connect(mg_name.CharPtr,
                                                            config->GetNative());

        ManagedPostConnect();

        return Create(nativeptr);

        _GF_MG_EXCEPTION_CATCH_ALL2

          finally {
          native::DistributedSystemImpl::releaseDisconnectLock();
        }
      }

      void DistributedSystem::Disconnect()
      {
        native::DistributedSystemImpl::acquireDisconnectLock();

        _GF_MG_EXCEPTION_TRY2

          if (native::DistributedSystem::isConnected()) {
            // native::CacheImpl::expiryTaskManager->cancelTask(
            // s_memoryPressureTaskID);
            Serializable::UnregisterNativesGeneric();
            DistributedSystem::UnregisterBuiltinManagedTypes();
          }
        native::DistributedSystem::disconnect();

        _GF_MG_EXCEPTION_CATCH_ALL2

          finally {
          native::DistributedSystemImpl::releaseDisconnectLock();
        }
      }

      void DistributedSystem::AppDomainInstanceInitialization(
        const native::PropertiesPtr& nativepropsptr)
      {
        _GF_MG_EXCEPTION_TRY2

          // Register wrapper types for built-in types, this are still cpp wrapper

          /*
            Serializable::RegisterWrapperGeneric(
            gcnew WrapperDelegateGeneric(Apache::Geode::Client::CacheableHashSet::Create),
            native::GeodeTypeIds::CacheableHashSet);

            Serializable::RegisterWrapperGeneric(
            gcnew WrapperDelegateGeneric(Apache::Geode::Client::CacheableLinkedHashSet::Create),
            native::GeodeTypeIds::CacheableLinkedHashSet);

            Serializable::RegisterWrapperGeneric(
            gcnew WrapperDelegateGeneric(Apache::Geode::Client::Struct::Create),
            native::GeodeTypeIds::Struct);

            Serializable::RegisterWrapperGeneric(
            gcnew WrapperDelegateGeneric(Apache::Geode::Client::Properties::CreateDeserializable),
            native::GeodeTypeIds::Properties);

            // End register wrapper types for built-in types

            // Register with cpp using unmanaged Cacheablekey wrapper
            Serializable::RegisterTypeGeneric(
            native::GeodeTypeIds::CacheableByte,
            gcnew TypeFactoryMethodGeneric(Apache::Geode::Client::CacheableByte::CreateDeserializable));

            Serializable::RegisterTypeGeneric(
            native::GeodeTypeIds::CacheableBoolean,
            gcnew TypeFactoryMethodGeneric(Apache::Geode::Client::CacheableBoolean::CreateDeserializable));

            Serializable::RegisterTypeGeneric(
            native::GeodeTypeIds::CacheableBytes,
            gcnew TypeFactoryMethodGeneric(Apache::Geode::Client::CacheableBytes::CreateDeserializable));

            Serializable::RegisterTypeGeneric(
            native::GeodeTypeIds::BooleanArray,
            gcnew TypeFactoryMethodGeneric(Apache::Geode::Client::BooleanArray::CreateDeserializable));

            Serializable::RegisterTypeGeneric(
            native::GeodeTypeIds::CacheableWideChar,
            gcnew TypeFactoryMethodGeneric(Apache::Geode::Client::CacheableCharacter::CreateDeserializable));

            Serializable::RegisterTypeGeneric(
            native::GeodeTypeIds::CharArray,
            gcnew TypeFactoryMethodGeneric(Apache::Geode::Client::CharArray::CreateDeserializable));

            Serializable::RegisterTypeGeneric(
            native::GeodeTypeIds::CacheableDouble,
            gcnew TypeFactoryMethodGeneric(Apache::Geode::Client::CacheableDouble::CreateDeserializable));

            Serializable::RegisterTypeGeneric(
            native::GeodeTypeIds::CacheableDoubleArray,
            gcnew TypeFactoryMethodGeneric(Apache::Geode::Client::CacheableDoubleArray::CreateDeserializable));

            Serializable::RegisterTypeGeneric(
            native::GeodeTypeIds::CacheableFloat,
            gcnew TypeFactoryMethodGeneric(Apache::Geode::Client::CacheableFloat::CreateDeserializable));

            Serializable::RegisterTypeGeneric(
            native::GeodeTypeIds::CacheableFloatArray,
            gcnew TypeFactoryMethodGeneric(Apache::Geode::Client::CacheableFloatArray::CreateDeserializable));


            Serializable::RegisterTypeGeneric(
            native::GeodeTypeIds::CacheableHashSet,
            gcnew TypeFactoryMethodGeneric(Apache::Geode::Client::CacheableHashSet::CreateDeserializable));

            Serializable::RegisterTypeGeneric(
            native::GeodeTypeIds::CacheableLinkedHashSet,
            gcnew TypeFactoryMethodGeneric(Apache::Geode::Client::CacheableLinkedHashSet::CreateDeserializable));

            Serializable::RegisterTypeGeneric(
            native::GeodeTypeIds::CacheableInt16,
            gcnew TypeFactoryMethodGeneric(Apache::Geode::Client::CacheableInt16::CreateDeserializable));

            Serializable::RegisterTypeGeneric(
            native::GeodeTypeIds::CacheableInt16Array,
            gcnew TypeFactoryMethodGeneric(Apache::Geode::Client::CacheableInt16Array::CreateDeserializable));

            Serializable::RegisterTypeGeneric(
            native::GeodeTypeIds::CacheableInt32,
            gcnew TypeFactoryMethodGeneric(Apache::Geode::Client::CacheableInt32::CreateDeserializable));

            Serializable::RegisterTypeGeneric(
            native::GeodeTypeIds::CacheableInt32Array,
            gcnew TypeFactoryMethodGeneric(Apache::Geode::Client::CacheableInt32Array::CreateDeserializable));

            Serializable::RegisterTypeGeneric(
            native::GeodeTypeIds::CacheableInt64,
            gcnew TypeFactoryMethodGeneric(Apache::Geode::Client::CacheableInt64::CreateDeserializable));

            Serializable::RegisterTypeGeneric(
            native::GeodeTypeIds::CacheableInt64Array,
            gcnew TypeFactoryMethodGeneric(Apache::Geode::Client::CacheableInt64Array::CreateDeserializable));
            */

            /*Serializable::RegisterTypeGeneric(
              native::GeodeTypeIds::CacheableASCIIString,
              gcnew TypeFactoryMethodGeneric(Apache::Geode::Client::CacheableString::CreateDeserializable));

              Serializable::RegisterTypeGeneric(
              native::GeodeTypeIds::CacheableASCIIStringHuge,
              gcnew TypeFactoryMethodGeneric(Apache::Geode::Client::CacheableString::createDeserializableHuge));

              Serializable::RegisterTypeGeneric(
              native::GeodeTypeIds::CacheableString,
              gcnew TypeFactoryMethodGeneric(Apache::Geode::Client::CacheableString::createUTFDeserializable));

              Serializable::RegisterTypeGeneric(
              native::GeodeTypeIds::CacheableStringHuge,
              gcnew TypeFactoryMethodGeneric(Apache::Geode::Client::CacheableString::createUTFDeserializableHuge));*/

              /*
              Serializable::RegisterTypeGeneric(
              native::GeodeTypeIds::CacheableNullString,
              gcnew TypeFactoryMethodGeneric(Apache::Geode::Client::CacheableString::CreateDeserializable));

              Serializable::RegisterTypeGeneric(
              native::GeodeTypeIds::CacheableStringArray,
              gcnew TypeFactoryMethodGeneric(Apache::Geode::Client::CacheableStringArray::CreateDeserializable));

              Serializable::RegisterTypeGeneric(
              native::GeodeTypeIds::Struct,
              gcnew TypeFactoryMethodGeneric(Apache::Geode::Client::Struct::CreateDeserializable));

              Serializable::RegisterTypeGeneric(
              native::GeodeTypeIds::Properties,
              gcnew TypeFactoryMethodGeneric(Apache::Geode::Client::Properties::CreateDeserializable));
              */

              // End register other built-in types

              //primitive types are still C++, as these are used as keys mostly
              // Register generic wrapper types for built-in types
              //byte

              /* Serializable::RegisterWrapperGeneric(
                 gcnew WrapperDelegateGeneric(CacheableByte::Create),
                 native::GeodeTypeIds::CacheableByte, Byte::typeid);*/

                 Serializable::RegisterWrapperGeneric(
                 gcnew WrapperDelegateGeneric(CacheableByte::Create),
                 native::GeodeTypeIds::CacheableByte, SByte::typeid);

        //boolean
        Serializable::RegisterWrapperGeneric(
          gcnew WrapperDelegateGeneric(CacheableBoolean::Create),
          native::GeodeTypeIds::CacheableBoolean, Boolean::typeid);
        //wide char
        Serializable::RegisterWrapperGeneric(
          gcnew WrapperDelegateGeneric(CacheableCharacter::Create),
          native::GeodeTypeIds::CacheableWideChar, Char::typeid);
        //double
        Serializable::RegisterWrapperGeneric(
          gcnew WrapperDelegateGeneric(CacheableDouble::Create),
          native::GeodeTypeIds::CacheableDouble, Double::typeid);
        //ascii string
        Serializable::RegisterWrapperGeneric(
          gcnew WrapperDelegateGeneric(CacheableString::Create),
          native::GeodeTypeIds::CacheableASCIIString, String::typeid);

        //TODO:
        ////ascii string huge
        //Serializable::RegisterWrapperGeneric(
        //  gcnew WrapperDelegateGeneric(CacheableString::Create),
        //  native::GeodeTypeIds::CacheableASCIIStringHuge, String::typeid);
        ////string
        //Serializable::RegisterWrapperGeneric(
        //  gcnew WrapperDelegateGeneric(CacheableString::Create),
        //  native::GeodeTypeIds::CacheableString, String::typeid);
        ////string huge
        //Serializable::RegisterWrapperGeneric(
        //  gcnew WrapperDelegateGeneric(CacheableString::Create),
        //  native::GeodeTypeIds::CacheableStringHuge, String::typeid);
        //float

        Serializable::RegisterWrapperGeneric(
          gcnew WrapperDelegateGeneric(CacheableFloat::Create),
          native::GeodeTypeIds::CacheableFloat, float::typeid);
        //int 16
        Serializable::RegisterWrapperGeneric(
          gcnew WrapperDelegateGeneric(CacheableInt16::Create),
          native::GeodeTypeIds::CacheableInt16, Int16::typeid);
        //int32
        Serializable::RegisterWrapperGeneric(
          gcnew WrapperDelegateGeneric(CacheableInt32::Create),
          native::GeodeTypeIds::CacheableInt32, Int32::typeid);
        //int64
        Serializable::RegisterWrapperGeneric(
          gcnew WrapperDelegateGeneric(CacheableInt64::Create),
          native::GeodeTypeIds::CacheableInt64, Int64::typeid);

        ////uint16
        //Serializable::RegisterWrapperGeneric(
        //  gcnew WrapperDelegateGeneric(CacheableInt16::Create),
        //  native::GeodeTypeIds::CacheableInt16, UInt16::typeid);
        ////uint32
        //Serializable::RegisterWrapperGeneric(
        //  gcnew WrapperDelegateGeneric(CacheableInt32::Create),
        //  native::GeodeTypeIds::CacheableInt32, UInt32::typeid);
        ////uint64
        //Serializable::RegisterWrapperGeneric(
        //  gcnew WrapperDelegateGeneric(CacheableInt64::Create),
        //  native::GeodeTypeIds::CacheableInt64, UInt64::typeid);
        //=======================================================================

        //Now onwards all will be wrap in managed cacheable key..

        Serializable::RegisterTypeGeneric(
          native::GeodeTypeIds::CacheableBytes,
          gcnew TypeFactoryMethodGeneric(CacheableBytes::CreateDeserializable),
          Type::GetType("System.Byte[]"));

        /* Serializable::RegisterTypeGeneric(
           native::GeodeTypeIds::CacheableBytes,
           gcnew TypeFactoryMethodGeneric(CacheableBytes::CreateDeserializable),
           Type::GetType("System.SByte[]"));*/

        Serializable::RegisterTypeGeneric(
          native::GeodeTypeIds::CacheableDoubleArray,
          gcnew TypeFactoryMethodGeneric(CacheableDoubleArray::CreateDeserializable),
          Type::GetType("System.Double[]"));

        Serializable::RegisterTypeGeneric(
          native::GeodeTypeIds::CacheableFloatArray,
          gcnew TypeFactoryMethodGeneric(CacheableFloatArray::CreateDeserializable),
          Type::GetType("System.Single[]"));

        //TODO:
        //as it is
        Serializable::RegisterTypeGeneric(
          native::GeodeTypeIds::CacheableHashSet,
          gcnew TypeFactoryMethodGeneric(CacheableHashSet::CreateDeserializable),
          nullptr);

        //as it is
        Serializable::RegisterTypeGeneric(
          native::GeodeTypeIds::CacheableLinkedHashSet,
          gcnew TypeFactoryMethodGeneric(CacheableLinkedHashSet::CreateDeserializable),
          nullptr);


        Serializable::RegisterTypeGeneric(
          native::GeodeTypeIds::CacheableInt16Array,
          gcnew TypeFactoryMethodGeneric(CacheableInt16Array::CreateDeserializable),
          Type::GetType("System.Int16[]"));

        /*  Serializable::RegisterTypeGeneric(
            native::GeodeTypeIds::CacheableInt16Array,
            gcnew TypeFactoryMethodGeneric(CacheableInt16Array::CreateDeserializable),
            Type::GetType("System.UInt16[]"));*/


        Serializable::RegisterTypeGeneric(
          native::GeodeTypeIds::CacheableInt32Array,
          gcnew TypeFactoryMethodGeneric(CacheableInt32Array::CreateDeserializable),
          Type::GetType("System.Int32[]"));

        /* Serializable::RegisterTypeGeneric(
           native::GeodeTypeIds::CacheableInt32Array,
           gcnew TypeFactoryMethodGeneric(CacheableInt32Array::CreateDeserializable),
           Type::GetType("System.UInt32[]"));*/


        Serializable::RegisterTypeGeneric(
          native::GeodeTypeIds::CacheableInt64Array,
          gcnew TypeFactoryMethodGeneric(CacheableInt64Array::CreateDeserializable),
          Type::GetType("System.Int64[]"));

        /* Serializable::RegisterTypeGeneric(
           native::GeodeTypeIds::CacheableInt64Array,
           gcnew TypeFactoryMethodGeneric(CacheableInt64Array::CreateDeserializable),
           Type::GetType("System.UInt64[]"));*/
        //TODO:;split

        Serializable::RegisterTypeGeneric(
          native::GeodeTypeIds::BooleanArray,
          gcnew TypeFactoryMethodGeneric(BooleanArray::CreateDeserializable),
          Type::GetType("System.Boolean[]"));

        Serializable::RegisterTypeGeneric(
          native::GeodeTypeIds::CharArray,
          gcnew TypeFactoryMethodGeneric(CharArray::CreateDeserializable),
          Type::GetType("System.Char[]"));

        //TODO::

        //Serializable::RegisterTypeGeneric(
        //  native::GeodeTypeIds::CacheableNullString,
        //  gcnew TypeFactoryMethodNew(Apache::Geode::Client::CacheableString::CreateDeserializable));

        Serializable::RegisterTypeGeneric(
          native::GeodeTypeIds::CacheableStringArray,
          gcnew TypeFactoryMethodGeneric(CacheableStringArray::CreateDeserializable),
          Type::GetType("System.String[]"));

        //as it is
        Serializable::RegisterTypeGeneric(
          native::GeodeTypeIds::Struct,
          gcnew TypeFactoryMethodGeneric(Struct::CreateDeserializable),
          nullptr);

        //as it is
        Serializable::RegisterTypeGeneric(
          native::GeodeTypeIds::Properties,
          gcnew TypeFactoryMethodGeneric(Properties<String^, String^>::CreateDeserializable),
          nullptr);

        /*  Serializable::RegisterTypeGeneric(
            native::GeodeTypeIds::PdxType,
            gcnew TypeFactoryMethodGeneric(Apache::Geode::Client::Internal::PdxType::CreateDeserializable),
            nullptr);*/

        Serializable::RegisterTypeGeneric(
          native::GeodeTypeIds::EnumInfo,
          gcnew TypeFactoryMethodGeneric(Apache::Geode::Client::Internal::EnumInfo::CreateDeserializable),
          nullptr);

        // End register generic wrapper types for built-in types

        if (!native::DistributedSystem::isConnected())
        {
          // Set the Generic ManagedAuthInitialize factory function
          native::SystemProperties::managedAuthInitializeFn =
            native::ManagedAuthInitializeGeneric::create;

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
        }

        _GF_MG_EXCEPTION_CATCH_ALL2
      }

      void DistributedSystem::ManagedPostConnect()
      {
        //  The registration into the native map should be after
        // native connect since it can be invoked only once

        // Register other built-in types
        /*
        Serializable::RegisterTypeGeneric(
        native::GeodeTypeIds::CacheableDate,
        gcnew TypeFactoryMethodGeneric(Apache::Geode::Client::CacheableDate::CreateDeserializable));
        Serializable::RegisterTypeGeneric(
        native::GeodeTypeIds::CacheableFileName,
        gcnew TypeFactoryMethodGeneric(Apache::Geode::Client::CacheableFileName::CreateDeserializable));
        Serializable::RegisterTypeGeneric(
        native::GeodeTypeIds::CacheableHashMap,
        gcnew TypeFactoryMethodGeneric(Apache::Geode::Client::CacheableHashMap::CreateDeserializable));
        Serializable::RegisterTypeGeneric(
        native::GeodeTypeIds::CacheableHashTable,
        gcnew TypeFactoryMethodGeneric(Apache::Geode::Client::CacheableHashTable::CreateDeserializable));
        Serializable::RegisterTypeGeneric(
        native::GeodeTypeIds::CacheableIdentityHashMap,
        gcnew TypeFactoryMethodGeneric(
        Apache::Geode::Client::CacheableIdentityHashMap::CreateDeserializable));
        Serializable::RegisterTypeGeneric(
        native::GeodeTypeIds::CacheableUndefined,
        gcnew TypeFactoryMethodGeneric(Apache::Geode::Client::CacheableUndefined::CreateDeserializable));
        Serializable::RegisterTypeGeneric(
        native::GeodeTypeIds::CacheableVector,
        gcnew TypeFactoryMethodGeneric(Apache::Geode::Client::CacheableVector::CreateDeserializable));
        Serializable::RegisterTypeGeneric(
        native::GeodeTypeIds::CacheableObjectArray,
        gcnew TypeFactoryMethodGeneric(
        Apache::Geode::Client::CacheableObjectArray::CreateDeserializable));
        Serializable::RegisterTypeGeneric(
        native::GeodeTypeIds::CacheableArrayList,
        gcnew TypeFactoryMethodGeneric(Apache::Geode::Client::CacheableArrayList::CreateDeserializable));
        Serializable::RegisterTypeGeneric(
        native::GeodeTypeIds::CacheableStack,
        gcnew TypeFactoryMethodGeneric(Apache::Geode::Client::CacheableStack::CreateDeserializable));
        Serializable::RegisterTypeGeneric(
        GeodeClassIds::CacheableManagedObject - 0x80000000,
        gcnew TypeFactoryMethodGeneric(Apache::Geode::Client::CacheableObject::CreateDeserializable));
        Serializable::RegisterTypeGeneric(
        GeodeClassIds::CacheableManagedObjectXml - 0x80000000,
        gcnew TypeFactoryMethodGeneric(Apache::Geode::Client::CacheableObjectXml::CreateDeserializable));
        */
        // End register other built-in types

        // Register other built-in types for generics
        //c# datatime

        Serializable::RegisterTypeGeneric(
          native::GeodeTypeIds::CacheableDate,
          gcnew TypeFactoryMethodGeneric(CacheableDate::CreateDeserializable),
          Type::GetType("System.DateTime"));

        //as it is
        Serializable::RegisterTypeGeneric(
          native::GeodeTypeIds::CacheableFileName,
          gcnew TypeFactoryMethodGeneric(CacheableFileName::CreateDeserializable),
          nullptr);

        //for generic dictionary define its type in static constructor of Serializable.hpp
        Serializable::RegisterTypeGeneric(
          native::GeodeTypeIds::CacheableHashMap,
          gcnew TypeFactoryMethodGeneric(CacheableHashMap::CreateDeserializable),
          nullptr);

        //c# hashtable
        Serializable::RegisterTypeGeneric(
          native::GeodeTypeIds::CacheableHashTable,
          gcnew TypeFactoryMethodGeneric(CacheableHashTable::CreateDeserializable),
          Type::GetType("System.Collections.Hashtable"));

        //Need to keep public as no counterpart in c#
        Serializable::RegisterTypeGeneric(
          native::GeodeTypeIds::CacheableIdentityHashMap,
          gcnew TypeFactoryMethodGeneric(
          CacheableIdentityHashMap::CreateDeserializable),
          nullptr);

        //keep as it is
        Serializable::RegisterTypeGeneric(
          native::GeodeTypeIds::CacheableUndefined,
          gcnew TypeFactoryMethodGeneric(CacheableUndefined::CreateDeserializable),
          nullptr);

        //c# arraylist
        Serializable::RegisterTypeGeneric(
          native::GeodeTypeIds::CacheableVector,
          gcnew TypeFactoryMethodGeneric(CacheableVector::CreateDeserializable),
          nullptr);

        //as it is
        Serializable::RegisterTypeGeneric(
          native::GeodeTypeIds::CacheableObjectArray,
          gcnew TypeFactoryMethodGeneric(
          CacheableObjectArray::CreateDeserializable),
          nullptr);

        //Generic::List
        Serializable::RegisterTypeGeneric(
          native::GeodeTypeIds::CacheableArrayList,
          gcnew TypeFactoryMethodGeneric(CacheableArrayList::CreateDeserializable),
          nullptr);

        //c# generic stack 
        Serializable::RegisterTypeGeneric(
          native::GeodeTypeIds::CacheableStack,
          gcnew TypeFactoryMethodGeneric(CacheableStack::CreateDeserializable),
          nullptr);

        //as it is
        Serializable::RegisterTypeGeneric(
          GeodeClassIds::CacheableManagedObject - 0x80000000,
          gcnew TypeFactoryMethodGeneric(CacheableObject::CreateDeserializable),
          nullptr);

        //as it is
        Serializable::RegisterTypeGeneric(
          GeodeClassIds::CacheableManagedObjectXml - 0x80000000,
          gcnew TypeFactoryMethodGeneric(CacheableObjectXml::CreateDeserializable),
          nullptr);

        // End register other built-in types

        // TODO: what will happen for following if first appDomain unload ??
        // Testing shows that there are problems; need to discuss -- maybe
        // maintain per AppDomainID functions in C++ layer.

        // Log the version of the C# layer being used
        Log::Config(".NET layer assembly version: {0}({1})", System::Reflection::
                    Assembly::GetExecutingAssembly()->GetName()->Version->ToString(),
                    System::Reflection::Assembly::GetExecutingAssembly()->ImageRuntimeVersion);

        Log::Config(".NET runtime version: {0} ", System::Environment::Version);
        Log::Config(".NET layer source repository (revision): {0} ({1})",
                    PRODUCT_SOURCE_REPOSITORY, PRODUCT_SOURCE_REVISION);
      }

      void DistributedSystem::AppDomainInstancePostInitialization()
      {
        //to create .net memory pressure handler 
        Create(native::DistributedSystem::getInstance());
      }

      void DistributedSystem::UnregisterBuiltinManagedTypes()
      {
        _GF_MG_EXCEPTION_TRY2

          native::DistributedSystemImpl::acquireDisconnectLock();

        Serializable::UnregisterNativesGeneric();

        int remainingInstances =
          native::DistributedSystemImpl::currentInstances();

        if (remainingInstances == 0) { // last instance


          Serializable::UnregisterTypeGeneric(
            native::GeodeTypeIds::CacheableDate);
          Serializable::UnregisterTypeGeneric(
            native::GeodeTypeIds::CacheableFileName);
          Serializable::UnregisterTypeGeneric(
            native::GeodeTypeIds::CacheableHashMap);
          Serializable::UnregisterTypeGeneric(
            native::GeodeTypeIds::CacheableHashTable);
          Serializable::UnregisterTypeGeneric(
            native::GeodeTypeIds::CacheableIdentityHashMap);
          Serializable::UnregisterTypeGeneric(
            native::GeodeTypeIds::CacheableVector);
          Serializable::UnregisterTypeGeneric(
            native::GeodeTypeIds::CacheableObjectArray);
          Serializable::UnregisterTypeGeneric(
            native::GeodeTypeIds::CacheableArrayList);
          Serializable::UnregisterTypeGeneric(
            native::GeodeTypeIds::CacheableStack);
          Serializable::UnregisterTypeGeneric(
            GeodeClassIds::CacheableManagedObject - 0x80000000);
          Serializable::UnregisterTypeGeneric(
            GeodeClassIds::CacheableManagedObjectXml - 0x80000000);

        }

        _GF_MG_EXCEPTION_CATCH_ALL2

          finally {
          native::DistributedSystemImpl::releaseDisconnectLock();
        }
      }

      Apache::Geode::Client::SystemProperties^ DistributedSystem::SystemProperties::get()
      {
        _GF_MG_EXCEPTION_TRY2

          return Apache::Geode::Client::SystemProperties::Create(
          native::DistributedSystem::getSystemProperties());

        _GF_MG_EXCEPTION_CATCH_ALL2
      }

      String^ DistributedSystem::Name::get()
      {
        try
        {
          return ManagedString::Get(m_nativeptr->get()->getName());
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }
      }

      bool DistributedSystem::IsConnected::get()
      {
        return native::DistributedSystem::isConnected();
      }

      DistributedSystem^ DistributedSystem::GetInstance()
      {
        return Create(native::DistributedSystem::getInstance());
      }

      void DistributedSystem::HandleMemoryPressure(System::Object^ state)
      {
        ACE_Time_Value dummy(1);
        MemoryPressureHandler handler;
        handler.handle_timeout(dummy, nullptr);
      }

      DistributedSystem^ DistributedSystem::Create(native::DistributedSystemPtr nativeptr)
      {
        if (m_instance == nullptr) {
          msclr::lock lockInstance(m_singletonSync);
          if (m_instance == nullptr) {
            m_instance = __nullptr == nativeptr ? nullptr :
              gcnew DistributedSystem(nativeptr);
          }
        }
        auto instance = (DistributedSystem^)m_instance;
        return instance;
      }

      DistributedSystem::DistributedSystem(native::DistributedSystemPtr nativeptr)
      {
        m_nativeptr = gcnew native_shared_ptr<native::DistributedSystem>(nativeptr);
        auto timerCallback = gcnew System::Threading::TimerCallback(&DistributedSystem::HandleMemoryPressure);
        m_memoryPressureHandler = gcnew System::Threading::Timer(
          timerCallback, "MemoryPressureHandler", 3 * 60000, 3 * 60000);
      }

      DistributedSystem::~DistributedSystem()
      {
        m_memoryPressureHandler->Dispose(nullptr);
      }

      void DistributedSystem::acquireDisconnectLock()
      {
        native::DistributedSystemImpl::acquireDisconnectLock();
      }

      void DistributedSystem::disconnectInstance()
      {
        native::DistributedSystemImpl::disconnectInstance();
      }

      void DistributedSystem::releaseDisconnectLock()
      {
        native::DistributedSystemImpl::releaseDisconnectLock();
      }

      void DistributedSystem::connectInstance()
      {
        native::DistributedSystemImpl::connectInstance();
      }

      void DistributedSystem::registerCliCallback()
      {
        m_cliCallBackObj = gcnew CliCallbackDelegate();
        auto nativeCallback =
          gcnew cliCallback(m_cliCallBackObj,
          &CliCallbackDelegate::Callback);

        native::DistributedSystemImpl::registerCliCallback(System::Threading::Thread::GetDomainID(),
                                                                          (native::CliCallbackMethod)System::Runtime::InteropServices::
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
