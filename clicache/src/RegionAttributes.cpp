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


#include "RegionAttributes.hpp"
#include "impl/ManagedCacheLoader.hpp"
#include "impl/ManagedCacheWriter.hpp"
#include "impl/ManagedCacheListener.hpp"
#include "impl/ManagedPartitionResolver.hpp"
#include "impl/ManagedFixedPartitionResolver.hpp"
#include "impl/CacheLoader.hpp"
#include "impl/CacheWriter.hpp"
#include "impl/CacheListener.hpp"
#include "impl/PartitionResolver.hpp"
#include "Properties.hpp"
#include "ICacheLoader.hpp"
#include "ICacheWriter.hpp"
#include "ICacheListener.hpp"
#include "IPartitionResolver.hpp"
#include "CacheListenerAdapter.hpp"
#include "CacheWriterAdapter.hpp"
#include "impl/SafeConvert.hpp"
#include "TimeUtils.hpp"


namespace Apache
{
  namespace Geode
  {
    namespace Client
    {
      using namespace System;
      using namespace msclr::interop;

      namespace native = apache::geode::client;

      generic <class TKey, class TValue>
      void Client::RegionAttributes<TKey, TValue>::ToData(
        Apache::Geode::Client::DataOutput^ output )
      {
        auto nativeOutput = output->GetNative();
        if (nativeOutput != nullptr)
        {
          try
          {
            m_nativeptr->get()->toData(*nativeOutput);
          }
          finally
          {
            GC::KeepAlive(m_nativeptr);
          }
        }
      }

      generic <class TKey, class TValue>
      void Client::RegionAttributes<TKey, TValue>::FromData(
        Apache::Geode::Client::DataInput^ input )
      {
        auto nativeInput = input->GetNative();
        if (nativeInput != nullptr)
        {
          try
          {
            m_nativeptr->get()->fromData(*nativeInput);
          }
          finally
          {
            GC::KeepAlive(m_nativeptr);
          }
        }
      }

      generic <class TKey, class TValue>
      ICacheLoader<TKey, TValue>^ Client::RegionAttributes<TKey, TValue>::CacheLoader::get()
      {
        try
        {
          auto loaderptr = m_nativeptr->get()->getCacheLoader();
          if (auto mg_loader = std::dynamic_pointer_cast<native::ManagedCacheLoaderGeneric>(loaderptr))
          {
            return (ICacheLoader<TKey, TValue>^) mg_loader->userptr();
          }
          return nullptr;
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }
      }

      generic <class TKey, class TValue>
      ICacheWriter<TKey, TValue>^ Client::RegionAttributes<TKey, TValue>::CacheWriter::get()
      {
        try
        {
          auto writerptr = m_nativeptr->get()->getCacheWriter();
          if (auto mg_writer = std::dynamic_pointer_cast<native::ManagedCacheWriterGeneric>(writerptr))
          {
            return (ICacheWriter<TKey, TValue>^)mg_writer->userptr();
          }
          return nullptr;
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }
      }

      generic <class TKey, class TValue>
      ICacheListener<TKey, TValue>^ Client::RegionAttributes<TKey, TValue>::CacheListener::get()
      {
        try
        {
          auto listenerptr = m_nativeptr->get()->getCacheListener();
          if (auto mg_listener = std::dynamic_pointer_cast<native::ManagedCacheListenerGeneric>(listenerptr))
          {
            return (ICacheListener<TKey, TValue>^)mg_listener->userptr();
          }
          return nullptr;
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }
      }

      generic <class TKey, class TValue>
      IPartitionResolver<TKey, TValue>^ Client::RegionAttributes<TKey, TValue>::PartitionResolver::get()
      {
        try
        {
          auto resolverptr = m_nativeptr->get()->getPartitionResolver();
          if (auto mg_resolver = std::dynamic_pointer_cast<native::ManagedPartitionResolverGeneric>(resolverptr))
          {
            return (IPartitionResolver<TKey, TValue>^)mg_resolver->userptr();
          }

          if (auto mg_fixedResolver = std::dynamic_pointer_cast<native::ManagedFixedPartitionResolverGeneric>(resolverptr))
          {
            return (IPartitionResolver<TKey, TValue>^)mg_fixedResolver->userptr();
          }

          return nullptr;
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }
      }

      generic <class TKey, class TValue>
      TimeSpan Client::RegionAttributes<TKey, TValue>::RegionTimeToLive::get()
      {
        try
        {
          return TimeUtils::DurationToTimeSpan(m_nativeptr->get()->getRegionTimeToLive( ));
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }
      }

      generic <class TKey, class TValue>
      ExpirationAction Client::RegionAttributes<TKey, TValue>::RegionTimeToLiveAction::get()
      {
        try
        {
          return static_cast<ExpirationAction>( m_nativeptr->get()->getRegionTimeToLiveAction( ) );
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }
      }

      generic <class TKey, class TValue>
      TimeSpan Client::RegionAttributes<TKey, TValue>::RegionIdleTimeout::get()
      {
        try
        {
          return TimeUtils::DurationToTimeSpan(m_nativeptr->get()->getRegionIdleTimeout( ));
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }
      }

      generic <class TKey, class TValue>
      ExpirationAction Client::RegionAttributes<TKey, TValue>::RegionIdleTimeoutAction::get()
      {
        try
        {
          return static_cast<ExpirationAction>( m_nativeptr->get()->getRegionIdleTimeoutAction( ) );
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }
      }

      generic <class TKey, class TValue>
      TimeSpan Client::RegionAttributes<TKey, TValue>::EntryTimeToLive::get()
      {
        try
        {
          return TimeUtils::DurationToTimeSpan(m_nativeptr->get()->getEntryTimeToLive( ));
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }
      }

      generic <class TKey, class TValue>
      ExpirationAction Client::RegionAttributes<TKey, TValue>::EntryTimeToLiveAction::get()
      {
        try
        {
          return static_cast<ExpirationAction>( m_nativeptr->get()->getEntryTimeToLiveAction( ) );
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }
      }

      generic <class TKey, class TValue>
      TimeSpan Client::RegionAttributes<TKey, TValue>::EntryIdleTimeout::get()
      {
        try
        {
          return TimeUtils::DurationToTimeSpan(m_nativeptr->get()->getEntryIdleTimeout( ));
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }
      }

      generic <class TKey, class TValue>
      ExpirationAction Client::RegionAttributes<TKey, TValue>::EntryIdleTimeoutAction::get()
      {
        try
        {
          return static_cast<ExpirationAction>( m_nativeptr->get()->getEntryIdleTimeoutAction( ) );
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }
      }

      generic <class TKey, class TValue>
      bool Client::RegionAttributes<TKey, TValue>::CachingEnabled::get()
      {
        try
        {
          return m_nativeptr->get()->getCachingEnabled( );
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }
      }

      generic <class TKey, class TValue>
      bool Client::RegionAttributes<TKey, TValue>::CloningEnabled::get()
      {
        try
        {
          return m_nativeptr->get()->getCloningEnabled( );
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }
      }

      generic <class TKey, class TValue>
      System::Int32 Client::RegionAttributes<TKey, TValue>::InitialCapacity::get()
      {
        try
        {
          return m_nativeptr->get()->getInitialCapacity( );
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }
      }

      generic <class TKey, class TValue>
      Single Client::RegionAttributes<TKey, TValue>::LoadFactor::get()
      {
        try
        {
          return m_nativeptr->get()->getLoadFactor( );
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }
      }

      generic <class TKey, class TValue>
        System::Int32 Client::RegionAttributes<TKey, TValue>::ConcurrencyLevel::get()
      {
        try
        {
          return m_nativeptr->get()->getConcurrencyLevel( );
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }
      }

      generic <class TKey, class TValue>
      System::UInt32 Client::RegionAttributes<TKey, TValue>::LruEntriesLimit::get()
      {
        try
        {
          return m_nativeptr->get()->getLruEntriesLimit( );
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }
      }

      generic <class TKey, class TValue>
      DiskPolicyType Client::RegionAttributes<TKey, TValue>::DiskPolicy::get()
      {
        try
        {
          return static_cast<DiskPolicyType>( m_nativeptr->get()->getDiskPolicy( ) );
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }
      }

      generic <class TKey, class TValue>
      ExpirationAction Client::RegionAttributes<TKey, TValue>::LruEvictionAction::get()
      {
        try
        {
          return static_cast<ExpirationAction>( m_nativeptr->get()->getLruEvictionAction( ) );
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }
      }

      generic <class TKey, class TValue>
      String^ Client::RegionAttributes<TKey, TValue>::CacheLoaderLibrary::get()
      {
        try
        {
          return marshal_as<String^>( m_nativeptr->get()->getCacheLoaderLibrary( ) );
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }
      }

      generic <class TKey, class TValue>
      String^ Client::RegionAttributes<TKey, TValue>::CacheLoaderFactory::get()
      {
        try
        {
          return marshal_as<String^>( m_nativeptr->get()->getCacheLoaderFactory( ) );
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }
      }

      generic <class TKey, class TValue>
      String^ Client::RegionAttributes<TKey, TValue>::CacheListenerLibrary::get()
      {
        try
        {
          return marshal_as<String^>( m_nativeptr->get()->getCacheListenerLibrary( ) );
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }
      }

      generic <class TKey, class TValue>
      String^ Client::RegionAttributes<TKey, TValue>::PartitionResolverLibrary::get()
      {
        try
        {
          return marshal_as<String^>( m_nativeptr->get()->getPartitionResolverLibrary( ) );
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }
      }

      generic <class TKey, class TValue>
      String^ Client::RegionAttributes<TKey, TValue>::PartitionResolverFactory::get()
      {
        try
        {
          return marshal_as<String^>( m_nativeptr->get()->getPartitionResolverFactory( ) );
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }
      }

      generic <class TKey, class TValue>
      String^ Client::RegionAttributes<TKey, TValue>::CacheListenerFactory::get()
      {
        try
        {
          return marshal_as<String^>( m_nativeptr->get()->getCacheListenerFactory( ) );
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }
      }

      generic <class TKey, class TValue>
      String^ Client::RegionAttributes<TKey, TValue>::CacheWriterLibrary::get()
      {
        try
        {
          return marshal_as<String^>( m_nativeptr->get()->getCacheWriterLibrary( ) );
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }
      }

      generic <class TKey, class TValue>
      String^ Client::RegionAttributes<TKey, TValue>::CacheWriterFactory::get()
      {
        try
        {
          return marshal_as<System::String^>(m_nativeptr->get()->getCacheWriterFactory());
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }
      }

      generic <class TKey, class TValue>
      bool Client::RegionAttributes<TKey, TValue>::Equals(Client::RegionAttributes<TKey, TValue>^ other)
      {
        auto otherPtr = other->GetNative();
        try
        {
          if (GetNative() != __nullptr && otherPtr != __nullptr) {
            return m_nativeptr->get()->operator==(*otherPtr);
          }
          return (GetNative() == otherPtr);
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }
      }

      generic <class TKey, class TValue>
      bool Client::RegionAttributes<TKey, TValue>::Equals(Object^ other)
      {
        return Equals(dynamic_cast<Client::RegionAttributes<TKey, TValue>^>(other));
      }

      generic <class TKey, class TValue>
      void Client::RegionAttributes<TKey, TValue>::ValidateSerializableAttributes()
      {
        try
        {
          m_nativeptr->get()->validateSerializableAttributes( );
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }
      }

      generic <class TKey, class TValue>
      String^ Client::RegionAttributes<TKey, TValue>::Endpoints::get()
      {
        try
        {
          return marshal_as<String^>( m_nativeptr->get()->getEndpoints( ) );
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }
      }

      generic <class TKey, class TValue>
      String^ Client::RegionAttributes<TKey, TValue>::PoolName::get()
      {
        try
        {
          return marshal_as<String^>( m_nativeptr->get()->getPoolName( ) );
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }
      }

      generic <class TKey, class TValue>
      Boolean Client::RegionAttributes<TKey, TValue>::ClientNotificationEnabled::get()
      {
        try
        {
          return m_nativeptr->get()->getClientNotificationEnabled( );
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }
      }

      generic <class TKey, class TValue>
      String^ Client::RegionAttributes<TKey, TValue>::PersistenceLibrary::get()
      {
        try
        {
          return marshal_as<String^>( m_nativeptr->get()->getPersistenceLibrary( ) );
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }
      }

      generic <class TKey, class TValue>
      String^ Client::RegionAttributes<TKey, TValue>::PersistenceFactory::get()
      {
        try
        {
          return marshal_as<String^>( m_nativeptr->get()->getPersistenceFactory( ) );
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }
      }
      generic <class TKey, class TValue>
      bool Client::RegionAttributes<TKey, TValue>::ConcurrencyChecksEnabled::get()
      {
        try
        {
          return m_nativeptr->get()->getConcurrencyChecksEnabled( );
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }
      }

      generic <class TKey, class TValue>
      Properties<String^, String^>^Client::RegionAttributes<TKey, TValue>::PersistenceProperties::get()
      {
        try
        {
          return Properties<String^, String^>::Create(m_nativeptr->get()->getPersistenceProperties());
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }
      }
    }  // namespace Client
  }  // namespace Geode
}  // namespace Apache
