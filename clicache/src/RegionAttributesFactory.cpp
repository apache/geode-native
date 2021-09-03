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


#include "RegionAttributesFactory.hpp"
#include "Region.hpp"
#include "impl/ManagedCacheLoader.hpp"
#include "impl/ManagedPersistenceManager.hpp"
#include "impl/ManagedCacheWriter.hpp"
#include "impl/ManagedCacheListener.hpp"
#include "impl/ManagedPartitionResolver.hpp"
#include "impl/ManagedFixedPartitionResolver.hpp"
#include "impl/CacheLoader.hpp"
#include "impl/CacheWriter.hpp"
#include "impl/CacheListener.hpp"
#include "impl/PartitionResolver.hpp"
#include "impl/PersistenceManagerProxy.hpp"
#include "RegionAttributes.hpp"
#include "ICacheLoader.hpp"
#include "IPersistenceManager.hpp"
#include "ICacheWriter.hpp"
#include "IPartitionResolver.hpp"
#include "IFixedPartitionResolver.hpp"
#include "impl/SafeConvert.hpp"
#include "ExceptionTypes.hpp"
#include "TimeUtils.hpp"

#include "begin_native.hpp"
#include <memory>
#include "end_native.hpp"


namespace Apache
{
  namespace Geode
  {
    namespace Client
    {
      using namespace System;
      using namespace System::Collections::Generic;
      using namespace msclr::interop;

      namespace native = apache::geode::client;

      generic<class TKey, class TValue>
      RegionAttributesFactory<TKey, TValue>::RegionAttributesFactory( Apache::Geode::Client::RegionAttributes<TKey, TValue>^ regionAttributes )
      {
        auto attribptr = regionAttributes->GetNative();
        m_nativeptr = gcnew native_unique_ptr<native::RegionAttributesFactory>(std::make_unique<native::RegionAttributesFactory>(*attribptr));
      }

      // CALLBACKS

      generic<class TKey, class TValue>
      RegionAttributesFactory<TKey, TValue>^ RegionAttributesFactory<TKey, TValue>::SetCacheLoader( ICacheLoader<TKey, TValue>^ cacheLoader )
      {
        std::shared_ptr<native::CacheLoader> loaderptr;
        if ( cacheLoader != nullptr ) {
          CacheLoaderGeneric<TKey, TValue>^ clg = gcnew CacheLoaderGeneric<TKey, TValue>();
          clg->SetCacheLoader(cacheLoader);
          loaderptr = std::shared_ptr<native::ManagedCacheLoaderGeneric>(new native::ManagedCacheLoaderGeneric(cacheLoader));
          ((native::ManagedCacheLoaderGeneric*)loaderptr.get())->setptr(clg);
        }
        try
        {
          m_nativeptr->get()->setCacheLoader( loaderptr );
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }
        return this;
      }

      generic<class TKey, class TValue>
      RegionAttributesFactory<TKey, TValue>^ RegionAttributesFactory<TKey, TValue>::SetCacheWriter( ICacheWriter<TKey, TValue>^ cacheWriter )
      {
        std::shared_ptr<native::CacheWriter> writerptr;
        if ( cacheWriter != nullptr ) {
          CacheWriterGeneric<TKey, TValue>^ cwg = gcnew CacheWriterGeneric<TKey, TValue>();
          cwg->SetCacheWriter(cacheWriter);
          writerptr = std::shared_ptr<native::ManagedCacheWriterGeneric>(new native::ManagedCacheWriterGeneric(cacheWriter));
          ((native::ManagedCacheWriterGeneric*)writerptr.get())->setptr(cwg);
        }
        try
        {
          m_nativeptr->get()->setCacheWriter( writerptr );
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }
        return this;
      }

      generic<class TKey, class TValue>
      RegionAttributesFactory<TKey, TValue>^ RegionAttributesFactory<TKey, TValue>::SetCacheListener( ICacheListener<TKey, TValue>^ cacheListener )
      {
        std::shared_ptr<native::CacheListener> listenerptr;
        if ( cacheListener != nullptr ) {
          CacheListenerGeneric<TKey, TValue>^ clg = gcnew CacheListenerGeneric<TKey, TValue>();
          clg->SetCacheListener(cacheListener);
          listenerptr = std::shared_ptr<native::ManagedCacheListenerGeneric>(new native::ManagedCacheListenerGeneric(cacheListener));
          ((native::ManagedCacheListenerGeneric*)listenerptr.get())->setptr(clg);
        }
        try
        {
          m_nativeptr->get()->setCacheListener( listenerptr );
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }
        return this;
      }

      generic<class TKey, class TValue>
      RegionAttributesFactory<TKey, TValue>^ RegionAttributesFactory<TKey, TValue>::SetPartitionResolver( IPartitionResolver<TKey, TValue>^ partitionresolver )
      {
        std::shared_ptr<native::PartitionResolver> resolverptr;
        if ( partitionresolver != nullptr ) {
          Client::IFixedPartitionResolver<TKey, TValue>^ resolver =
            dynamic_cast<Client::IFixedPartitionResolver<TKey, TValue>^>(partitionresolver);
          if (resolver != nullptr) {
            FixedPartitionResolverGeneric<TKey, TValue>^ prg = gcnew FixedPartitionResolverGeneric<TKey, TValue>();
            prg->SetPartitionResolver(partitionresolver);
            resolverptr = std::shared_ptr<native::ManagedFixedPartitionResolverGeneric>(new native::ManagedFixedPartitionResolverGeneric(partitionresolver));
            ((native::ManagedFixedPartitionResolverGeneric*)resolverptr.get())->setptr(prg);
          }
          else {
            PartitionResolverGeneric<TKey, TValue>^ prg = gcnew PartitionResolverGeneric<TKey, TValue>();
            prg->SetPartitionResolver(partitionresolver);
            resolverptr = std::shared_ptr<native::ManagedPartitionResolverGeneric>(new native::ManagedPartitionResolverGeneric(partitionresolver));
            ((native::ManagedPartitionResolverGeneric*)resolverptr.get())->setptr(prg);
          }
        }
        try
        {
          m_nativeptr->get()->setPartitionResolver( resolverptr );
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }
        return this;
      }

      generic<class TKey, class TValue>
      RegionAttributesFactory<TKey, TValue>^ RegionAttributesFactory<TKey, TValue>::SetCacheLoader( String^ libPath, String^ factoryFunctionName )
      {
        throw gcnew System::NotSupportedException;

        try
        {
          m_nativeptr->get()->setCacheLoader( marshal_as<std::string>(libPath), marshal_as<std::string>(factoryFunctionName) );
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }
        return this;
      }

      generic<class TKey, class TValue>
      RegionAttributesFactory<TKey, TValue>^ RegionAttributesFactory<TKey, TValue>::SetCacheWriter( String^ libPath, String^ factoryFunctionName )
      {
        throw gcnew System::NotSupportedException;

        try
        {
          m_nativeptr->get()->setCacheWriter( marshal_as<std::string>(libPath), marshal_as<std::string>(factoryFunctionName) );
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }
        return this;
      }

      generic<class TKey, class TValue>
      RegionAttributesFactory<TKey, TValue>^ RegionAttributesFactory<TKey, TValue>::SetCacheListener( String^ libPath, String^ factoryFunctionName )
      {
        throw gcnew System::NotSupportedException;

        try
        {
          m_nativeptr->get()->setCacheListener( marshal_as<std::string>(libPath), marshal_as<std::string>(factoryFunctionName) );
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }
        return this;
      }

      generic<class TKey, class TValue>
      RegionAttributesFactory<TKey, TValue>^ RegionAttributesFactory<TKey, TValue>::SetPartitionResolver( String^ libPath, String^ factoryFunctionName )
      {
        throw gcnew System::NotSupportedException;

        try
        {
          m_nativeptr->get()->setPartitionResolver( marshal_as<std::string>(libPath), marshal_as<std::string>(factoryFunctionName) );
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }
        return this;
      }

      // EXPIRATION ATTRIBUTES

      generic<class TKey, class TValue>
      RegionAttributesFactory<TKey, TValue>^ RegionAttributesFactory<TKey, TValue>::SetEntryIdleTimeout( ExpirationAction action, TimeSpan idleTimeout )
      {
        try
        {
          m_nativeptr->get()->setEntryIdleTimeout(static_cast<native::ExpirationAction>( action ), TimeUtils::TimeSpanToDurationCeil<std::chrono::seconds>(idleTimeout) );
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }
        return this;
      }

      generic<class TKey, class TValue>
      RegionAttributesFactory<TKey, TValue>^ RegionAttributesFactory<TKey, TValue>::SetEntryTimeToLive( ExpirationAction action, TimeSpan timeToLive )
      {
        try
        {
          m_nativeptr->get()->setEntryTimeToLive( static_cast<native::ExpirationAction>( action ), TimeUtils::TimeSpanToDurationCeil<std::chrono::seconds>(timeToLive) );
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }
        return this;
      }

      generic<class TKey, class TValue>
      RegionAttributesFactory<TKey, TValue>^ RegionAttributesFactory<TKey, TValue>::SetRegionIdleTimeout( ExpirationAction action, TimeSpan idleTimeout )
      {
        try
        {
          m_nativeptr->get()->setRegionIdleTimeout( static_cast<native::ExpirationAction>( action ), TimeUtils::TimeSpanToDurationCeil<std::chrono::seconds>(idleTimeout) );
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }
        return this;
      }

      generic<class TKey, class TValue>
      RegionAttributesFactory<TKey, TValue>^ RegionAttributesFactory<TKey, TValue>::SetRegionTimeToLive( ExpirationAction action, TimeSpan timeToLive )
      {
        try
        {
          m_nativeptr->get()->setRegionTimeToLive( static_cast<native::ExpirationAction>( action ), TimeUtils::TimeSpanToDurationCeil<std::chrono::seconds>(timeToLive) );
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }
        return this;
      }

      // PERSISTENCE
      generic<class TKey, class TValue>
      RegionAttributesFactory<TKey, TValue>^ RegionAttributesFactory<TKey, TValue>::SetPersistenceManager(IPersistenceManager<TKey, TValue>^ persistenceManager, Properties<String^, String^>^ config )
      {
        std::shared_ptr<native::PersistenceManager> persistenceManagerptr;
        if ( persistenceManager != nullptr ) {
          PersistenceManagerGeneric<TKey, TValue>^ clg = gcnew PersistenceManagerGeneric<TKey, TValue>();
          clg->SetPersistenceManager(persistenceManager);
          persistenceManagerptr = std::shared_ptr<native::ManagedPersistenceManagerGeneric>(new native::ManagedPersistenceManagerGeneric(persistenceManager));
          ((native::ManagedPersistenceManagerGeneric*)persistenceManagerptr.get())->setptr(clg);
        }
         try
         {
           m_nativeptr->get()->setPersistenceManager( persistenceManagerptr, config->GetNative() );
         }
         finally
         {
           GC::KeepAlive(m_nativeptr);
         }
        return this;
      }

      generic<class TKey, class TValue>
      RegionAttributesFactory<TKey, TValue>^ RegionAttributesFactory<TKey, TValue>::SetPersistenceManager(IPersistenceManager<TKey, TValue>^ persistenceManager )
      {
        SetPersistenceManager(persistenceManager, nullptr);
        return this;
      }

      generic<class TKey, class TValue>
      RegionAttributesFactory<TKey, TValue>^ RegionAttributesFactory<TKey, TValue>::SetPersistenceManager( String^ libPath,
        String^ factoryFunctionName )
      {
        SetPersistenceManager( libPath, factoryFunctionName, nullptr );
        return this;
      }

      generic<class TKey, class TValue>
      RegionAttributesFactory<TKey, TValue>^ RegionAttributesFactory<TKey, TValue>::SetPersistenceManager( String^ libPath,
        String^ factoryFunctionName, Properties<String^, String^>^ config )
      {

        try
        {
          m_nativeptr->get()->setPersistenceManager(marshal_as<std::string>(libPath), marshal_as<std::string>(factoryFunctionName), config->GetNative());
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }
        return this;

      }

      // STORAGE ATTRIBUTES

      generic<class TKey, class TValue>
      RegionAttributesFactory<TKey, TValue>^ RegionAttributesFactory<TKey, TValue>::SetPoolName( String^ poolName )
      {

        try
        {
          m_nativeptr->get()->setPoolName( marshal_as<std::string>(poolName) );
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }
        return this;
      }

      // MAP ATTRIBUTES

      generic<class TKey, class TValue>
      RegionAttributesFactory<TKey, TValue>^ RegionAttributesFactory<TKey, TValue>::SetInitialCapacity( System::Int32 initialCapacity )
      {
        try {

          try
          {
            m_nativeptr->get()->setInitialCapacity( initialCapacity );
          }
          finally
          {
            GC::KeepAlive(m_nativeptr);
          }

        }
        catch (const apache::geode::client::Exception& ex) {
          throw Apache::Geode::Client::GeodeException::Get(ex);
        }
        catch (System::AccessViolationException^ ex) {
          throw ex;
        }

        return this;
      }

      generic<class TKey, class TValue>
      RegionAttributesFactory<TKey, TValue>^ RegionAttributesFactory<TKey, TValue>::SetLoadFactor( Single loadFactor )
      {
        try {

          try
          {
            m_nativeptr->get()->setLoadFactor( loadFactor );
          }
          finally
          {
            GC::KeepAlive(m_nativeptr);
          }

        }
        catch (const apache::geode::client::Exception& ex) {
          throw Apache::Geode::Client::GeodeException::Get(ex);
        }
        catch (System::AccessViolationException^ ex) {
          throw ex;
        }

        return this;
      }

      generic<class TKey, class TValue>
      RegionAttributesFactory<TKey, TValue>^ RegionAttributesFactory<TKey, TValue>::SetConcurrencyLevel( System::Int32 concurrencyLevel )
      {
        try {

          try
          {
            m_nativeptr->get()->setConcurrencyLevel( concurrencyLevel );
          }
          finally
          {
            GC::KeepAlive(m_nativeptr);
          }

        }
        catch (const apache::geode::client::Exception& ex) {
          throw Apache::Geode::Client::GeodeException::Get(ex);
        }
        catch (System::AccessViolationException^ ex) {
          throw ex;
        }

        return this;
      }

      generic<class TKey, class TValue>
      RegionAttributesFactory<TKey, TValue>^ RegionAttributesFactory<TKey, TValue>::SetLruEntriesLimit( System::UInt32 entriesLimit )
      {
        try
        {
          m_nativeptr->get()->setLruEntriesLimit( entriesLimit );
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }
        return this;
      }

      generic<class TKey, class TValue>
      RegionAttributesFactory<TKey, TValue>^ RegionAttributesFactory<TKey, TValue>::SetDiskPolicy( DiskPolicyType diskPolicy )
      {
        try
        {
          m_nativeptr->get()->setDiskPolicy(static_cast<native::DiskPolicyType>( diskPolicy ) );
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }
        return this;
      }

      generic<class TKey, class TValue>
      RegionAttributesFactory<TKey, TValue>^ RegionAttributesFactory<TKey, TValue>::SetCachingEnabled( bool cachingEnabled )
      {
        try
        {
          m_nativeptr->get()->setCachingEnabled( cachingEnabled );
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }
        return this;
      }

      generic<class TKey, class TValue>
      RegionAttributesFactory<TKey, TValue>^ RegionAttributesFactory<TKey, TValue>::SetCloningEnabled( bool cloningEnabled )
      {
        try
        {
          m_nativeptr->get()->setCloningEnabled( cloningEnabled );
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }
        return this;
      }

      generic<class TKey, class TValue>
      RegionAttributesFactory<TKey, TValue>^  RegionAttributesFactory<TKey, TValue>::SetConcurrencyChecksEnabled( bool concurrencyChecksEnabled )
      {
        try
        {
          m_nativeptr->get()->setConcurrencyChecksEnabled( concurrencyChecksEnabled );
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }
        return this;
      }
      // FACTORY METHOD

      generic<class TKey, class TValue>
      Apache::Geode::Client::RegionAttributes<TKey, TValue>^ RegionAttributesFactory<TKey, TValue>::Create()
      {
        try {

          try
          {
            auto nativeRegionAttributes = m_nativeptr->get()->create();
            return Apache::Geode::Client::RegionAttributes<TKey, TValue>::Create(nativeRegionAttributes);
          }
          finally
          {
            GC::KeepAlive(m_nativeptr);
          }

        }
        catch (const apache::geode::client::Exception& ex) {
          throw Apache::Geode::Client::GeodeException::Get(ex);
        }
        catch (System::AccessViolationException^ ex) {
          throw ex;
        }
      }
    }  // namespace Client
  }  // namespace Geode
}  // namespace Apache

