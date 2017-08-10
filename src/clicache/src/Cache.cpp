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
#include "CacheRegionHelper.hpp"
#include "CacheImpl.hpp"
#include "end_native.hpp"

#include "Cache.hpp"
#include "ExceptionTypes.hpp"
#include "DistributedSystem.hpp"
#include "PoolFactory.hpp"
#include "Region.hpp"
#include "RegionAttributes.hpp"
#include "QueryService.hpp"
#include "CacheFactory.hpp"
#include "impl/AuthenticatedCache.hpp"
#include "impl/ManagedString.hpp"
#include "impl/SafeConvert.hpp"
#include "impl/PdxTypeRegistry.hpp"
#include "impl/PdxInstanceFactoryImpl.hpp"

#pragma warning(disable:4091)

using namespace System;

namespace Apache
{
  namespace Geode
  {
    namespace Client
    {
      namespace native = apache::geode::client;

      String^ Cache::Name::get( )
      {
        try
        {
          return ManagedString::Get( m_nativeptr->get()->getName( ).c_str() );
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }
      }

      bool Cache::IsClosed::get( )
      {
        try
        {
          return m_nativeptr->get()->isClosed( );
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }
      }

      DistributedSystem^ Cache::DistributedSystem::get( )
      {
        try
        {
          return Client::DistributedSystem::Create(&(m_nativeptr->get()->getDistributedSystem()));
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }
      }

      CacheTransactionManager^ Cache::CacheTransactionManager::get( )
      {
        // TODO shared_ptr this should be checking the return type for which tx mgr
        try
        {
          auto nativeptr = std::dynamic_pointer_cast<InternalCacheTransactionManager2PC>(
            m_nativeptr->get()->getCacheTransactionManager());
          return Apache::Geode::Client::CacheTransactionManager::Create(nativeptr);
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }
      }

      void Cache::Close( )
      {
        Close( false );
      }

      void Cache::Close( bool keepalive )
      {
        _GF_MG_EXCEPTION_TRY2

          Apache::Geode::Client::DistributedSystem::acquireDisconnectLock();

          Apache::Geode::Client::DistributedSystem::disconnectInstance();
          CacheFactory::m_connected = false;

          try
          {
            m_nativeptr->get()->close( keepalive );
          }
          finally
          {
            GC::KeepAlive(m_nativeptr);
          }

          // If DS automatically disconnected due to the new bootstrap API, then cleanup the C++/CLI side
          //if (!apache::geode::client::DistributedSystem::isConnected())
          {
            Apache::Geode::Client::DistributedSystem::UnregisterBuiltinManagedTypes(this);
          }

        _GF_MG_EXCEPTION_CATCH_ALL2
        finally
        {
					CacheRegionHelper::getCacheImpl(m_nativeptr->get())->getPdxTypeRegistry()->clear();
          Serializable::Clear();
          Apache::Geode::Client::DistributedSystem::releaseDisconnectLock();
          Apache::Geode::Client::DistributedSystem::unregisterCliCallback();
        }
      }

      void Cache::ReadyForEvents( )
      {
        _GF_MG_EXCEPTION_TRY2

          try
          {
            m_nativeptr->get()->readyForEvents( );
          }
          finally
          {
            GC::KeepAlive(m_nativeptr);
          }

        _GF_MG_EXCEPTION_CATCH_ALL2
      }

      generic<class TKey, class TValue>
      Client::IRegion<TKey,TValue>^ Cache::GetRegion( String^ path )
      {
        _GF_MG_EXCEPTION_TRY2/* due to auto replace */

          ManagedString mg_path( path );
          try
          {
            return Client::Region<TKey, TValue>::Create(m_nativeptr->get()->getRegion(mg_path.CharPtr));
          }
          finally
          {
            GC::KeepAlive(m_nativeptr);
          }

        _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */
      }

      generic<class TKey, class TValue>
      array<Client::IRegion<TKey, TValue>^>^ Cache::RootRegions( )
      {
        apache::geode::client::VectorOfRegion vrr;
        try
        {
          m_nativeptr->get()->rootRegions( vrr );
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }
        array<Client::IRegion<TKey, TValue>^>^ rootRegions =
          gcnew array<Client::IRegion<TKey, TValue>^>( vrr.size( ) );

        for( System::Int32 index = 0; index < vrr.size( ); index++ )
        {
          apache::geode::client::RegionPtr& nativeptr( vrr[ index ] );
          rootRegions[ index ] = Client::Region<TKey, TValue>::Create( nativeptr );
        }
        return rootRegions;
      }

      generic<class TKey, class TResult>
      Client::QueryService<TKey, TResult>^ Cache::GetQueryService( )
      {
        _GF_MG_EXCEPTION_TRY2

          try
          {
            return Client::QueryService<TKey, TResult>::Create(m_nativeptr->get()->getQueryService());
          }
          finally
          {
            GC::KeepAlive(m_nativeptr);
          }

        _GF_MG_EXCEPTION_CATCH_ALL2
      }

      generic<class TKey, class TResult>
      Client::QueryService<TKey, TResult>^ Cache::GetQueryService(String^ poolName )
      {
        _GF_MG_EXCEPTION_TRY2

          ManagedString mg_poolName( poolName );
          try
          {
            return QueryService<TKey, TResult>::Create(m_nativeptr->get()->getQueryService(mg_poolName.CharPtr));
          }
          finally
          {
            GC::KeepAlive(m_nativeptr);
          }

        _GF_MG_EXCEPTION_CATCH_ALL2
      }

      RegionFactory^ Cache::CreateRegionFactory(RegionShortcut preDefinedRegionAttributes)
      {
        _GF_MG_EXCEPTION_TRY2

          apache::geode::client::RegionShortcut preDefineRegionAttr = apache::geode::client::CACHING_PROXY;

          switch(preDefinedRegionAttributes)
          {
          case RegionShortcut::PROXY:
              preDefineRegionAttr = apache::geode::client::PROXY;
              break;
          case RegionShortcut::CACHING_PROXY:
              preDefineRegionAttr = apache::geode::client::CACHING_PROXY;
              break;
          case RegionShortcut::CACHING_PROXY_ENTRY_LRU:
              preDefineRegionAttr = apache::geode::client::CACHING_PROXY_ENTRY_LRU;
              break;
          case RegionShortcut::LOCAL:
              preDefineRegionAttr = apache::geode::client::LOCAL;
              break;
          case RegionShortcut::LOCAL_ENTRY_LRU:
              preDefineRegionAttr = apache::geode::client::LOCAL_ENTRY_LRU;
              break;          
          }

          try
          {
            return RegionFactory::Create(m_nativeptr->get()->createRegionFactory(preDefineRegionAttr));
          }
          finally
          {
            GC::KeepAlive(m_nativeptr);
          }
          
        _GF_MG_EXCEPTION_CATCH_ALL2
      }

      IRegionService^ Cache::CreateAuthenticatedView(Properties<String^, Object^>^ credentials)
      {        
        _GF_MG_EXCEPTION_TRY2

          try
          {
            return AuthenticatedCache::Create((m_nativeptr->get()->createAuthenticatedView(credentials->GetNative())));
          }
          finally
          {
            GC::KeepAlive(m_nativeptr);
          }

        _GF_MG_EXCEPTION_CATCH_ALL2   
      }

			bool Cache::GetPdxIgnoreUnreadFields()
			{
				_GF_MG_EXCEPTION_TRY2

					try
					{
					  return	m_nativeptr->get()->getPdxIgnoreUnreadFields();
					}
					finally
					{
					  GC::KeepAlive(m_nativeptr);
					}

				_GF_MG_EXCEPTION_CATCH_ALL2   
			}

      bool Cache::GetPdxReadSerialized()
			{
				_GF_MG_EXCEPTION_TRY2

					try
					{
					  return	m_nativeptr->get()->getPdxReadSerialized();
					}
					finally
					{
					  GC::KeepAlive(m_nativeptr);
					}

				_GF_MG_EXCEPTION_CATCH_ALL2   
			}

      IRegionService^ Cache::CreateAuthenticatedView(Properties<String^, Object^>^ credentials, String^ poolName)
      {
        ManagedString mg_poolName( poolName );

        _GF_MG_EXCEPTION_TRY2

          try
          {
            return AuthenticatedCache::Create( (m_nativeptr->get()->createAuthenticatedView(credentials->GetNative(), mg_poolName.CharPtr)));
          }
          finally
          {
            GC::KeepAlive(m_nativeptr);
          }

        _GF_MG_EXCEPTION_CATCH_ALL2   
      }

			 void Cache::InitializeDeclarativeCache( String^ cacheXml )
      {
        ManagedString mg_cacheXml( cacheXml );
        try
        {
          m_nativeptr->get()->initializeDeclarativeCache( mg_cacheXml.CharPtr );
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }
      }

       IPdxInstanceFactory^ Cache::CreatePdxInstanceFactory(String^ className)
       {
    
         return gcnew Internal::PdxInstanceFactoryImpl(className, (m_nativeptr->get()));

       }

       DataInput^ Cache::CreateDataInput(array<Byte>^ buffer, System::Int32 len)
       {
         return gcnew DataInput(buffer, len,  m_nativeptr->get());
       }

       
       DataInput^ Cache::CreateDataInput(array<Byte>^ buffer)
       {
         return gcnew DataInput(buffer, m_nativeptr->get());
       }

        DataOutput^ Cache::CreateDataOutput()
       {
         return gcnew DataOutput( m_nativeptr->get());
       }

        PoolFactory^ Cache::GetPoolFactory()
        {
          return PoolFactory::Create(m_nativeptr->get_shared_ptr()->getPoolManager().createFactory());
        }

        PoolManager^ Cache::GetPoolManager()
        {
          return gcnew PoolManager(m_nativeptr->get_shared_ptr()->getPoolManager());
        }
    }  // namespace Client
  }  // namespace Geode
}  // namespace Apache

