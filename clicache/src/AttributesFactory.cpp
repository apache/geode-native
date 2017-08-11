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

#include "AttributesFactory.hpp"
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

      namespace native = apache::geode::client;

      generic<class TKey, class TValue>
      AttributesFactory<TKey, TValue>::AttributesFactory( Apache::Geode::Client::RegionAttributes<TKey, TValue>^ regionAttributes )
      {
        auto attribptr = regionAttributes->GetNative();
        m_nativeptr = gcnew native_unique_ptr<native::AttributesFactory>(std::make_unique<native::AttributesFactory>(attribptr));
      }

      // CALLBACKS

      generic<class TKey, class TValue>
      void AttributesFactory<TKey, TValue>::SetCacheLoader( ICacheLoader<TKey, TValue>^ cacheLoader )
      {
        native::CacheLoaderPtr loaderptr;
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
      }

      generic<class TKey, class TValue>
      void AttributesFactory<TKey, TValue>::SetCacheWriter( ICacheWriter<TKey, TValue>^ cacheWriter )
      {
        native::CacheWriterPtr writerptr;
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
      }

      generic<class TKey, class TValue>
      void AttributesFactory<TKey, TValue>::SetCacheListener( ICacheListener<TKey, TValue>^ cacheListener )
      {
        native::CacheListenerPtr listenerptr;
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
      }

      generic<class TKey, class TValue>
      void AttributesFactory<TKey, TValue>::SetPartitionResolver( IPartitionResolver<TKey, TValue>^ partitionresolver )
      {
        native::PartitionResolverPtr resolverptr;
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
      }

      generic<class TKey, class TValue>
      void AttributesFactory<TKey, TValue>::SetCacheLoader( String^ libPath, String^ factoryFunctionName )
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
      }

      generic<class TKey, class TValue>
      void AttributesFactory<TKey, TValue>::SetCacheWriter( String^ libPath, String^ factoryFunctionName )
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
      }

      generic<class TKey, class TValue>
      void AttributesFactory<TKey, TValue>::SetCacheListener( String^ libPath, String^ factoryFunctionName )
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
      }

      generic<class TKey, class TValue>
      void AttributesFactory<TKey, TValue>::SetPartitionResolver( String^ libPath, String^ factoryFunctionName )
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
      }

      // EXPIRATION ATTRIBUTES

      generic<class TKey, class TValue>
      void AttributesFactory<TKey, TValue>::SetEntryIdleTimeout( ExpirationAction action, System::UInt32 idleTimeout )
      {
        try
        {
          m_nativeptr->get()->setEntryIdleTimeout(static_cast<native::ExpirationAction::Action>( action ), idleTimeout );
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }
      }

      generic<class TKey, class TValue>
      void AttributesFactory<TKey, TValue>::SetEntryTimeToLive( ExpirationAction action, System::UInt32 timeToLive )
      {
        try
        {
          m_nativeptr->get()->setEntryTimeToLive( static_cast<native::ExpirationAction::Action>( action ), timeToLive );
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }
      }

      generic<class TKey, class TValue>
      void AttributesFactory<TKey, TValue>::SetRegionIdleTimeout( ExpirationAction action, System::UInt32 idleTimeout )
      {
        try
        {
          m_nativeptr->get()->setRegionIdleTimeout( static_cast<native::ExpirationAction::Action>( action ), idleTimeout );
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }
      }

      generic<class TKey, class TValue>
      void AttributesFactory<TKey, TValue>::SetRegionTimeToLive( ExpirationAction action, System::UInt32 timeToLive )
      {
        try
        {
          m_nativeptr->get()->setRegionTimeToLive( static_cast<native::ExpirationAction::Action>( action ), timeToLive );
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }
      }

      // PERSISTENCE
      generic<class TKey, class TValue>
      void AttributesFactory<TKey, TValue>::SetPersistenceManager(IPersistenceManager<TKey, TValue>^ persistenceManager, Properties<String^, String^>^ config )
      {
        native::PersistenceManagerPtr persistenceManagerptr;
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
      }
      
      generic<class TKey, class TValue>
      void AttributesFactory<TKey, TValue>::SetPersistenceManager(IPersistenceManager<TKey, TValue>^ persistenceManager )
      {
        SetPersistenceManager(persistenceManager, nullptr);
      }
        
      generic<class TKey, class TValue>
      void AttributesFactory<TKey, TValue>::SetPersistenceManager( String^ libPath,
        String^ factoryFunctionName )
      {        
        SetPersistenceManager( libPath, factoryFunctionName, nullptr );
      }

      generic<class TKey, class TValue>
      void AttributesFactory<TKey, TValue>::SetPersistenceManager( String^ libPath,
        String^ factoryFunctionName, Properties<String^, String^>^ config )
      {        
        ManagedString mg_libpath( libPath );
        ManagedString mg_factoryFunctionName( factoryFunctionName );

        try
        {
          m_nativeptr->get()->setPersistenceManager(mg_libpath.CharPtr, mg_factoryFunctionName.CharPtr, config->GetNative());
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }
          
      }

      // STORAGE ATTRIBUTES

      generic<class TKey, class TValue>
      void AttributesFactory<TKey, TValue>::SetPoolName( String^ poolName )
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
      }

      // MAP ATTRIBUTES

      generic<class TKey, class TValue>
      void AttributesFactory<TKey, TValue>::SetInitialCapacity( System::Int32 initialCapacity )
      {
        _GF_MG_EXCEPTION_TRY2/* due to auto replace */

          try
          {
            m_nativeptr->get()->setInitialCapacity( initialCapacity );
          }
          finally
          {
            GC::KeepAlive(m_nativeptr);
          }

        _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */
      }

      generic<class TKey, class TValue>
      void AttributesFactory<TKey, TValue>::SetLoadFactor( Single loadFactor )
      {
        _GF_MG_EXCEPTION_TRY2/* due to auto replace */

          try
          {
            m_nativeptr->get()->setLoadFactor( loadFactor );
          }
          finally
          {
            GC::KeepAlive(m_nativeptr);
          }

        _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */
      }

      generic<class TKey, class TValue>
      void AttributesFactory<TKey, TValue>::SetConcurrencyLevel( System::Int32 concurrencyLevel )
      {
        _GF_MG_EXCEPTION_TRY2/* due to auto replace */

          try
          {
            m_nativeptr->get()->setConcurrencyLevel( concurrencyLevel );
          }
          finally
          {
            GC::KeepAlive(m_nativeptr);
          }

        _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */
      }

      generic<class TKey, class TValue>
      void AttributesFactory<TKey, TValue>::SetLruEntriesLimit( System::UInt32 entriesLimit )
      {
        try
        {
          m_nativeptr->get()->setLruEntriesLimit( entriesLimit );
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }
      }

      generic<class TKey, class TValue>
      void AttributesFactory<TKey, TValue>::SetDiskPolicy( DiskPolicyType diskPolicy )
      {
        try
        {
          m_nativeptr->get()->setDiskPolicy(static_cast<native::DiskPolicyType::PolicyType>( diskPolicy ) );
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }
      }

      generic<class TKey, class TValue>
      void AttributesFactory<TKey, TValue>::SetCachingEnabled( bool cachingEnabled )
      {
        try
        {
          m_nativeptr->get()->setCachingEnabled( cachingEnabled );
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }
      }

      generic<class TKey, class TValue>
      void AttributesFactory<TKey, TValue>::SetCloningEnabled( bool cloningEnabled )
      {
        try
        {
          m_nativeptr->get()->setCloningEnabled( cloningEnabled );
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }
      }

      generic<class TKey, class TValue>
      void  AttributesFactory<TKey, TValue>::SetConcurrencyChecksEnabled( bool concurrencyChecksEnabled )
      {
        try
        {
          m_nativeptr->get()->setConcurrencyChecksEnabled( concurrencyChecksEnabled );
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }
      }
      // FACTORY METHOD

      generic<class TKey, class TValue>
      Apache::Geode::Client::RegionAttributes<TKey, TValue>^ AttributesFactory<TKey, TValue>::CreateRegionAttributes()
      {
        _GF_MG_EXCEPTION_TRY2/* due to auto replace */

          try
          {
            native::RegionAttributesPtr nativeptr = m_nativeptr->get()->createRegionAttributes();
            return Apache::Geode::Client::RegionAttributes<TKey, TValue>::Create(nativeptr);
          }
          finally
          {
            GC::KeepAlive(m_nativeptr);
          }

        _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */
      }
    }  // namespace Client
  }  // namespace Geode
}  // namespace Apache

