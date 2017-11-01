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

#pragma once

#include "RegionFactory.hpp"
#include "RegionAttributes.hpp"
#include "impl/SafeConvert.hpp"

#include "impl/ManagedCacheLoader.hpp"
#include "impl/ManagedCacheWriter.hpp"
#include "impl/ManagedCacheListener.hpp"
#include "impl/ManagedPartitionResolver.hpp"
#include "impl/ManagedFixedPartitionResolver.hpp"
#include "impl/ManagedFixedPartitionResolver.hpp"
#include "impl/ManagedPersistenceManager.hpp"

#include "impl/CacheLoader.hpp"
#include "impl/CacheWriter.hpp"
#include "impl/CacheListener.hpp"
#include "impl/PartitionResolver.hpp"
#include "impl/FixedPartitionResolver.hpp"
#include "impl/FixedPartitionResolver.hpp"
#include "impl/PersistenceManagerProxy.hpp"

using namespace System;
using namespace System::Collections::Generic;


namespace Apache
{
  namespace Geode
  {
    namespace Client
    {
      namespace native = apache::geode::client;

      RegionFactory^ RegionFactory::SetCacheLoader( String^ libPath, String^ factoryFunctionName )
      {
        throw gcnew System::NotSupportedException;
        ManagedString mg_libpath( libPath );
        ManagedString mg_factoryFunctionName( factoryFunctionName );

        try
        {
          m_nativeptr->get()->setCacheLoader( mg_libpath.CharPtr, mg_factoryFunctionName.CharPtr );
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }
        return this;
      }

      RegionFactory^ RegionFactory::SetCacheWriter( String^ libPath, String^ factoryFunctionName )
      {
        throw gcnew System::NotSupportedException;
        ManagedString mg_libpath( libPath );
        ManagedString mg_factoryFunctionName( factoryFunctionName );

        try
        {
          m_nativeptr->get()->setCacheWriter( mg_libpath.CharPtr, mg_factoryFunctionName.CharPtr );
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }
        return this;
      }

      RegionFactory^ RegionFactory::SetCacheListener( String^ libPath, String^ factoryFunctionName )
      {
        throw gcnew System::NotSupportedException;
        ManagedString mg_libpath( libPath );
        ManagedString mg_factoryFunctionName( factoryFunctionName );

        try
        {
          m_nativeptr->get()->setCacheListener( mg_libpath.CharPtr, mg_factoryFunctionName.CharPtr );
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }
        return this;
      }

      RegionFactory^ RegionFactory::SetPartitionResolver( String^ libPath, String^ factoryFunctionName )
      {
        throw gcnew System::NotSupportedException;
        ManagedString mg_libpath( libPath );
        ManagedString mg_factoryFunctionName( factoryFunctionName );

        try
        {
          m_nativeptr->get()->setPartitionResolver( mg_libpath.CharPtr, mg_factoryFunctionName.CharPtr );
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }
        return this;
      }

      // EXPIRATION ATTRIBUTES

      RegionFactory^ RegionFactory::SetEntryIdleTimeout( ExpirationAction action, System::UInt32 idleTimeout )
      {
        try
        {
          m_nativeptr->get()->setEntryIdleTimeout( static_cast<native::ExpirationAction::Action>( action ), idleTimeout );
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }
        return this;
      }

      RegionFactory^ RegionFactory::SetEntryTimeToLive( ExpirationAction action, System::UInt32 timeToLive )
      {
        try
        {
          m_nativeptr->get()->setEntryTimeToLive(static_cast<native::ExpirationAction::Action>( action ), timeToLive );
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }
        return this;
      }

      RegionFactory^ RegionFactory::SetRegionIdleTimeout( ExpirationAction action, System::UInt32 idleTimeout )
      {
        try
        {
          m_nativeptr->get()->setRegionIdleTimeout(static_cast<native::ExpirationAction::Action>( action ), idleTimeout );
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }
        return this;
      }

      RegionFactory^ RegionFactory::SetRegionTimeToLive( ExpirationAction action, System::UInt32 timeToLive )
      {
        try
        {
          m_nativeptr->get()->setRegionTimeToLive(static_cast<native::ExpirationAction::Action>( action ), timeToLive );
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }
        return this;
      }

      // PERSISTENCE

       generic <class TKey, class TValue>
      RegionFactory^ RegionFactory::SetPersistenceManager( Client::IPersistenceManager<TKey, TValue>^ persistenceManager, 
          Properties<String^, String^>^ config)
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

      generic <class TKey, class TValue>
      RegionFactory^ RegionFactory::SetPersistenceManager( Client::IPersistenceManager<TKey, TValue>^ persistenceManager )
      {
        return SetPersistenceManager(persistenceManager, nullptr);
      }

      RegionFactory^ RegionFactory::SetPersistenceManager( String^ libPath,
        String^ factoryFunctionName )
      {        
        SetPersistenceManager( libPath, factoryFunctionName, nullptr );
        return this;
      }

      RegionFactory^ RegionFactory::SetPersistenceManager( String^ libPath,
        String^ factoryFunctionName, /*Dictionary<Object^, Object^>*/Properties<String^, String^>^ config )
      {        
        ManagedString mg_libpath( libPath );
        ManagedString mg_factoryFunctionName( factoryFunctionName );

        try
        {
          m_nativeptr->get()->setPersistenceManager( mg_libpath.CharPtr, mg_factoryFunctionName.CharPtr, config->GetNative() );
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }
        return this;
      }

      RegionFactory^ RegionFactory::SetPoolName( String^ poolName )
      {
        ManagedString mg_poolName( poolName );

        try
        {
          m_nativeptr->get()->setPoolName( mg_poolName.CharPtr );
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }
        return this;
      }

      // MAP ATTRIBUTES

      RegionFactory^ RegionFactory::SetInitialCapacity( System::Int32 initialCapacity )
      {
        _GF_MG_EXCEPTION_TRY2

          try
          {
            m_nativeptr->get()->setInitialCapacity( initialCapacity );
          }
          finally
          {
            GC::KeepAlive(m_nativeptr);
          }
          return this;

        _GF_MG_EXCEPTION_CATCH_ALL2
      }

      RegionFactory^ RegionFactory::SetLoadFactor( Single loadFactor )
      {
        _GF_MG_EXCEPTION_TRY2

          try
          {
            m_nativeptr->get()->setLoadFactor( loadFactor );
          }
          finally
          {
            GC::KeepAlive(m_nativeptr);
          }
          return this;

        _GF_MG_EXCEPTION_CATCH_ALL2
      }

      RegionFactory^ RegionFactory::SetConcurrencyLevel( System::Int32 concurrencyLevel )
      {
        _GF_MG_EXCEPTION_TRY2

          try
          {
            m_nativeptr->get()->setConcurrencyLevel( concurrencyLevel );
          }
          finally
          {
            GC::KeepAlive(m_nativeptr);
          }
          return this;

        _GF_MG_EXCEPTION_CATCH_ALL2
      }

      RegionFactory^ RegionFactory::SetLruEntriesLimit( System::UInt32 entriesLimit )
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

      RegionFactory^ RegionFactory::SetDiskPolicy( DiskPolicyType diskPolicy )
      {
        try
        {
          m_nativeptr->get()->setDiskPolicy(static_cast<native::DiskPolicyType::PolicyType>( diskPolicy ) );
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }
        return this;
      }

      RegionFactory^ RegionFactory::SetCachingEnabled( bool cachingEnabled )
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

      RegionFactory^ RegionFactory::SetCloningEnabled( bool cloningEnabled )
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

      RegionFactory^ RegionFactory::SetConcurrencyChecksEnabled( bool concurrencyChecksEnabled )
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
      // NEW GENERIC APIs:

      generic <class TKey, class TValue>
      IRegion<TKey,TValue>^ RegionFactory::Create(String^ regionName)
      {
        _GF_MG_EXCEPTION_TRY2/* due to auto replace */

          try
          {
            ManagedString mg_name( regionName );
            auto nativeptr = m_nativeptr->get()->create( mg_name.CharPtr );
            return Client::Region<TKey,TValue>::Create( nativeptr );
          }
          finally
          {
            GC::KeepAlive(m_nativeptr);
          }

        _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */
      }

      generic <class TKey, class TValue>
      RegionFactory^ RegionFactory::SetCacheLoader( Client::ICacheLoader<TKey, TValue>^ cacheLoader )
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

      generic <class TKey, class TValue>
      RegionFactory^ RegionFactory::SetCacheWriter( Client::ICacheWriter<TKey, TValue>^ cacheWriter )
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

      generic <class TKey, class TValue>
      RegionFactory^ RegionFactory::SetCacheListener( Client::ICacheListener<TKey, TValue>^ cacheListener )
      {
        std::shared_ptr<native::CacheListener> listenerptr;
        if ( cacheListener != nullptr ) {
          CacheListenerGeneric<TKey, TValue>^ clg = gcnew CacheListenerGeneric<TKey, TValue>();
          clg->SetCacheListener(cacheListener);
          listenerptr = std::shared_ptr<native::ManagedCacheListenerGeneric>(new native::ManagedCacheListenerGeneric(cacheListener));
          ((native::ManagedCacheListenerGeneric*)listenerptr.get())->setptr(clg);
          /*
          listenerptr = new native::ManagedCacheListenerGeneric(
            (Client::ICacheListener<Object^, Object^>^)cacheListener);
            */
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

      generic <class TKey, class TValue>
      RegionFactory^ RegionFactory::SetPartitionResolver(Client::IPartitionResolver<TKey, TValue>^ partitionresolver)
      {
        std::shared_ptr<native::PartitionResolver> resolverptr;
        if (partitionresolver != nullptr) {
          Client::IFixedPartitionResolver<TKey, TValue>^ resolver =
            dynamic_cast<Client::IFixedPartitionResolver<TKey, TValue>^>(partitionresolver);
          if (resolver != nullptr) {
            FixedPartitionResolverGeneric<TKey, TValue>^ prg = gcnew FixedPartitionResolverGeneric<TKey, TValue>();
            prg->SetPartitionResolver(resolver);
            resolverptr = std::shared_ptr<native::ManagedFixedPartitionResolverGeneric>(new native::ManagedFixedPartitionResolverGeneric(resolver));
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
          m_nativeptr->get()->setPartitionResolver(resolverptr);
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }
        return this;
      }
    }  // namespace Client
  }  // namespace Geode
}  // namespace Apache

