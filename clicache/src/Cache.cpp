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
#include <CacheRegionHelper.hpp>
#include "end_native.hpp"

#include "Cache.hpp"
#include "ExceptionTypes.hpp"
#include "DistributedSystem.hpp"
#include "PoolFactory.hpp"
#include "Region.hpp"
#include "RegionAttributes.hpp"
#include "QueryService.hpp"
#include "CacheFactory.hpp"
#include "impl/AuthenticatedView.hpp"
#include "impl/SafeConvert.hpp"
#include "impl/PdxTypeRegistry.hpp"
#include "impl/PdxInstanceFactoryImpl.hpp"
#include "CacheTransactionManager.hpp"
#include "PoolManager.hpp"
#include "TypeRegistry.hpp"

#pragma warning(disable:4091)

using namespace System;
using namespace msclr::interop;

namespace Apache
{
  namespace Geode
  {
    namespace Client
    {
      namespace native = apache::geode::client;

      Cache::Cache(std::shared_ptr<native::Cache> nativeptr)
      {
        m_nativeptr = gcnew native_shared_ptr<native::Cache>(nativeptr);
        m_pdxTypeRegistry = gcnew Apache::Geode::Client::Internal::PdxTypeRegistry(this);
        m_typeRegistry = gcnew Apache::Geode::Client::TypeRegistry(this);
      }

      Cache::~Cache() { //Destructor - deterministic
        if (_disposed) return;
        //Clean-up managed resources
        this->!Cache();
        _disposed = true;
        //GC.SuppressFinalize(this) is automatically added here
        //Base destructor is automatically called too if needed
      }

      Cache::!Cache() { //Finalizer - non-deterministic when called by GC
        //Clean-up unmanaged resources
        m_nativeptr->get()->~Cache();
      }

      String^ Cache::Name::get( )
      {
        try
        {
          return marshal_as<String^>( m_nativeptr->get()->getName( ) );
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

      SystemProperties^ Cache::SystemProperties::get( )
      {
        try
        {
          auto&& systemProperties = m_nativeptr->get()->getSystemProperties();
          return Apache::Geode::Client::SystemProperties::Create(&systemProperties);
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
          auto nativeptr = m_nativeptr->get()->getCacheTransactionManager();
          return Apache::Geode::Client::CacheTransactionManager::Create(nativeptr.get());
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

          m_nativeptr->get()->close( keepalive );
          Apache::Geode::Client::DistributedSystem::UnregisterBuiltinManagedTypes(this);

        _GF_MG_EXCEPTION_CATCH_ALL2
        finally
        {
					CacheRegionHelper::getCacheImpl(m_nativeptr->get())->getPdxTypeRegistry()->clear();
          m_typeRegistry->Clear();
          Apache::Geode::Client::DistributedSystem::unregisterCliCallback();
          GC::KeepAlive(m_nativeptr);

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

          try
          {
            return Client::Region<TKey, TValue>::Create(m_nativeptr->get()->getRegion(marshal_as<std::string>(path)));
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
        std::vector<std::shared_ptr<apache::geode::client::Region>> vrr;
        try
        {
			vrr = m_nativeptr->get()->rootRegions( );
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }

        auto rootRegions = gcnew array<Client::IRegion<TKey, TValue>^>(static_cast<int>(vrr.size()));

        for( System::Int32 index = 0; index < vrr.size( ); index++ )
        {
          std::shared_ptr<apache::geode::client::Region>& nativeptr( vrr[ index ] );
          rootRegions[ index ] = Client::Region<TKey, TValue>::Create( nativeptr );
        }
        return rootRegions;
      }

      Client::QueryService^ Cache::GetQueryService( )
      {
        _GF_MG_EXCEPTION_TRY2

          try
          {
            return Client::QueryService::Create(m_nativeptr->get()->getQueryService());
          }
          finally
          {
            GC::KeepAlive(m_nativeptr);
          }

        _GF_MG_EXCEPTION_CATCH_ALL2
      }

      Client::QueryService^ Cache::GetQueryService(String^ poolName )
      {
        _GF_MG_EXCEPTION_TRY2

          try
          {
            return QueryService::Create(m_nativeptr->get()->getQueryService(marshal_as<std::string>(poolName)));
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

          auto preDefineRegionAttr = apache::geode::client::RegionShortcut(preDefinedRegionAttributes);

          try
          {
            return RegionFactory::Create(std::unique_ptr<native::RegionFactory>(
                new native::RegionFactory(
                    m_nativeptr->get()->createRegionFactory(preDefineRegionAttr))));
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
            return AuthenticatedView::Create((m_nativeptr->get()->createAuthenticatedView(credentials->GetNative(), "")));
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

        _GF_MG_EXCEPTION_TRY2

          try
          {
            return AuthenticatedView::Create( (m_nativeptr->get()->createAuthenticatedView(credentials->GetNative(), marshal_as<std::string>(poolName))));
          }
          finally
          {
            GC::KeepAlive(m_nativeptr);
          }

        _GF_MG_EXCEPTION_CATCH_ALL2   
      }

			 void Cache::InitializeDeclarativeCache( String^ cacheXml )
      {
        try
        {
          m_nativeptr->get()->initializeDeclarativeCache( marshal_as<std::string>(cacheXml));
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }
      }

       IPdxInstanceFactory^ Cache::CreatePdxInstanceFactory(String^ className)
       {
    
         return gcnew Internal::PdxInstanceFactoryImpl(className, this);

       }

       DataInput^ Cache::CreateDataInput(array<Byte>^ buffer, System::Int32 len)
       {
         return gcnew DataInput(buffer, len,  this);
       }

       
       DataInput^ Cache::CreateDataInput(array<Byte>^ buffer)
       {
         return gcnew DataInput(buffer, this);
       }

        DataOutput^ Cache::CreateDataOutput()
       {
         return gcnew DataOutput(this);
       }

        PoolFactory^ Cache::GetPoolFactory()
        {
          return PoolFactory::Create(std::unique_ptr<native::PoolFactory>(new native::PoolFactory(
            m_nativeptr->get_shared_ptr()->getPoolManager().createFactory())));
        }

        PoolManager^ Cache::GetPoolManager()
        {
          return gcnew PoolManager(m_nativeptr->get_shared_ptr()->getPoolManager());
        }
    }  // namespace Client
  }  // namespace Geode
}  // namespace Apache

