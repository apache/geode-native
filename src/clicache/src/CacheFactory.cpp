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

//#include "geode_includes.hpp"

#include "ExceptionTypes.hpp"

#include "CacheFactory.hpp"
#include "Cache.hpp"
#include "DistributedSystem.hpp"
#include "SystemProperties.hpp"
#include "impl/SafeConvert.hpp"
#include "impl/PdxTypeRegistry.hpp"
//#pragma warning(disable:4091)
//#include <msclr/lock.h>
//#pragma warning(disable:4091)
#include "impl/AppDomainContext.hpp"

using namespace System;

namespace Apache
{
  namespace Geode
  {
    namespace Client
    {

      namespace native = apache::geode::client;

      CacheFactory^ CacheFactory::CreateCacheFactory()
      {
        return CacheFactory::CreateCacheFactory(Properties<String^, String^>::Create<String^, String^>());
      }

      CacheFactory^ CacheFactory::CreateCacheFactory(Properties<String^, String^>^ dsProps)
      {
        _GF_MG_EXCEPTION_TRY2

          auto nativeCacheFactory = native::CacheFactory::createCacheFactory(dsProps->GetNative());         
          if (nativeCacheFactory)
            return gcnew CacheFactory( nativeCacheFactory, dsProps );
            
          return nullptr;

        _GF_MG_EXCEPTION_CATCH_ALL2
      }

      Cache^ CacheFactory::Create()
      {
				bool pdxIgnoreUnreadFields = false;
        bool pdxReadSerialized = false;
				bool appDomainEnable = false; 
        _GF_MG_EXCEPTION_TRY2
          //msclr::lock lockInstance(m_singletonSync);
          DistributedSystem::acquireDisconnectLock();
    
          if(!m_connected)
          {
            DistributedSystem::AppDomainInstanceInitialization(m_dsProps->GetNative());                  
          }

          auto nativeCache = m_nativeptr->get()->create( );
					pdxIgnoreUnreadFields = nativeCache->getPdxIgnoreUnreadFields();
          pdxReadSerialized = nativeCache->getPdxReadSerialized();

          appDomainEnable = DistributedSystem::SystemProperties->AppDomainEnabled;
          Log::SetLogLevel(static_cast<LogLevel>(native::Log::logLevel( )));
					//TODO::split
          SafeConvertClassGeneric::SetAppDomainEnabled(appDomainEnable);

          if (appDomainEnable)
          {
            // Register managed AppDomain context with unmanaged.
            native::createAppDomainContext = &Apache::Geode::Client::createAppDomainContext;
          }

            Serializable::RegisterTypeGeneric(
              native::GeodeTypeIds::PdxType,
              gcnew TypeFactoryMethodGeneric(Apache::Geode::Client::Internal::PdxType::CreateDeserializable),
              nullptr);

           if(!m_connected)
           {
             //it registers types in unmanage layer, so should be once only 
             DistributedSystem::ManagedPostConnect();
             DistributedSystem::AppDomainInstancePostInitialization();
             DistributedSystem::connectInstance();
           }
          
           m_connected = true;
           
           return Cache::Create( nativeCache );
        _GF_MG_EXCEPTION_CATCH_ALL2
          finally {
            GC::KeepAlive(m_nativeptr);
            DistributedSystem::registerCliCallback();
						Serializable::RegisterPDXManagedCacheableKey(appDomainEnable);
					Apache::Geode::Client::Internal::PdxTypeRegistry::PdxIgnoreUnreadFields = pdxIgnoreUnreadFields; 
          Apache::Geode::Client::Internal::PdxTypeRegistry::PdxReadSerialized = pdxReadSerialized; 
          DistributedSystem::releaseDisconnectLock();
        }
      }

      Cache^ CacheFactory::GetInstance( DistributedSystem^ system )
      {
        _GF_MG_EXCEPTION_TRY2

         return Cache::Create( native::CacheFactory::getInstance( system->GetNative() ) );

        _GF_MG_EXCEPTION_CATCH_ALL2
      }

      Cache^ CacheFactory::GetInstanceCloseOk( DistributedSystem^ system )
      {
        _GF_MG_EXCEPTION_TRY2

          return Cache::Create( native::CacheFactory::getInstanceCloseOk( system->GetNative() ) );

        _GF_MG_EXCEPTION_CATCH_ALL2
      }

      Cache^ CacheFactory::GetAnyInstance( )
      {
        _GF_MG_EXCEPTION_TRY2

          return Cache::Create( native::CacheFactory::getAnyInstance( ) );

        _GF_MG_EXCEPTION_CATCH_ALL2
      }

      String^ CacheFactory::Version::get( )
      {
        return ManagedString::Get( native::CacheFactory::getVersion( ) );
      }

      String^ CacheFactory::ProductDescription::get( )
      {
        return ManagedString::Get(
          native::CacheFactory::getProductDescription( ) );
      }


      CacheFactory^ CacheFactory::SetFreeConnectionTimeout( Int32 connectionTimeout )
		  {
			  _GF_MG_EXCEPTION_TRY2

			  try
			  {
			    m_nativeptr->get()->setFreeConnectionTimeout( connectionTimeout );
			  }
			  finally
			  {
			    GC::KeepAlive(m_nativeptr);
			  }

        return this;

			  _GF_MG_EXCEPTION_CATCH_ALL2
		  }

		  CacheFactory^ CacheFactory::SetLoadConditioningInterval( Int32 loadConditioningInterval )
		  {
			  _GF_MG_EXCEPTION_TRY2

			  try
			  {
			    m_nativeptr->get()->setLoadConditioningInterval( loadConditioningInterval );
			  }
			  finally
			  {
			    GC::KeepAlive(m_nativeptr);
			  }
        return this;

			  _GF_MG_EXCEPTION_CATCH_ALL2
		  }

		  CacheFactory^ CacheFactory::SetSocketBufferSize( Int32 bufferSize )
      {
			  _GF_MG_EXCEPTION_TRY2

          try
          {
            m_nativeptr->get()->setSocketBufferSize( bufferSize );
          }
          finally
          {
            GC::KeepAlive(m_nativeptr);
          }
          return this;

			  _GF_MG_EXCEPTION_CATCH_ALL2
		  }

		  CacheFactory^ CacheFactory::SetReadTimeout( Int32 timeout )
      {
			  _GF_MG_EXCEPTION_TRY2

          try
          {
            m_nativeptr->get()->setReadTimeout( timeout );
          }
          finally
          {
            GC::KeepAlive(m_nativeptr);
          }
          return this;

			  _GF_MG_EXCEPTION_CATCH_ALL2
		  }

		  CacheFactory^ CacheFactory::SetMinConnections( Int32 minConnections )
      {
			  _GF_MG_EXCEPTION_TRY2

          try
          {
            m_nativeptr->get()->setMinConnections( minConnections );
          }
          finally
          {
            GC::KeepAlive(m_nativeptr);
          }
          return this;

			  _GF_MG_EXCEPTION_CATCH_ALL2
		  }

		  CacheFactory^ CacheFactory::SetMaxConnections( Int32 maxConnections )
      {
			  _GF_MG_EXCEPTION_TRY2

          try
          {
            m_nativeptr->get()->setMaxConnections( maxConnections );
          }
          finally
          {
            GC::KeepAlive(m_nativeptr);
          }
          return this;

			  _GF_MG_EXCEPTION_CATCH_ALL2
		  }

		  CacheFactory^ CacheFactory::SetIdleTimeout( Int32 idleTimeout )
      {
			  _GF_MG_EXCEPTION_TRY2

          try
          {
            m_nativeptr->get()->setIdleTimeout( idleTimeout );
          }
          finally
          {
            GC::KeepAlive(m_nativeptr);
          }
          return this;

			  _GF_MG_EXCEPTION_CATCH_ALL2
		  }

		  CacheFactory^ CacheFactory::SetRetryAttempts( Int32 retryAttempts )
      {
			  _GF_MG_EXCEPTION_TRY2

			  try
			  {
			    m_nativeptr->get()->setRetryAttempts( retryAttempts );
			  }
			  finally
			  {
			    GC::KeepAlive(m_nativeptr);
			  }
        return this;

			  _GF_MG_EXCEPTION_CATCH_ALL2
		  }

		  CacheFactory^ CacheFactory::SetPingInterval( Int32 pingInterval )
      {
			  _GF_MG_EXCEPTION_TRY2

          try
          {
            m_nativeptr->get()->setPingInterval( pingInterval );
          }
          finally
          {
            GC::KeepAlive(m_nativeptr);
          }
          return this;

			  _GF_MG_EXCEPTION_CATCH_ALL2
		  }

      CacheFactory^ CacheFactory::SetUpdateLocatorListInterval( Int32 updateLocatorListInterval )
      {
			  _GF_MG_EXCEPTION_TRY2

          try
          {
            m_nativeptr->get()->setUpdateLocatorListInterval( updateLocatorListInterval );
          }
          finally
          {
            GC::KeepAlive(m_nativeptr);
          }
          return this;

			  _GF_MG_EXCEPTION_CATCH_ALL2
		  }

      CacheFactory^ CacheFactory::SetStatisticInterval( Int32 statisticInterval )
      {
			  _GF_MG_EXCEPTION_TRY2

          try
          {
            m_nativeptr->get()->setStatisticInterval( statisticInterval );
          }
          finally
          {
            GC::KeepAlive(m_nativeptr);
          }
          return this;

			  _GF_MG_EXCEPTION_CATCH_ALL2
		  }

      CacheFactory^ CacheFactory::SetServerGroup( String^ group )
      {
			  _GF_MG_EXCEPTION_TRY2

        ManagedString mg_servergroup( group );
        try
        {
          m_nativeptr->get()->setServerGroup( mg_servergroup.CharPtr );
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }
        return this;

			  _GF_MG_EXCEPTION_CATCH_ALL2
		  }

		  CacheFactory^ CacheFactory::AddLocator( String^ host, Int32 port )
      {
			  _GF_MG_EXCEPTION_TRY2

        ManagedString mg_host( host );
        try
        {
          m_nativeptr->get()->addLocator( mg_host.CharPtr, port );
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }
        return this;

			  _GF_MG_EXCEPTION_CATCH_ALL2
		  }

      CacheFactory^ CacheFactory::AddServer( String^ host, Int32 port )
      {
			  _GF_MG_EXCEPTION_TRY2

			  ManagedString mg_host( host );
        try
        {
          m_nativeptr->get()->addServer( mg_host.CharPtr, port );
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }
        return this;

			  _GF_MG_EXCEPTION_CATCH_ALL2
		  }

		  CacheFactory^ CacheFactory::SetSubscriptionEnabled( Boolean enabled )
      {
			  _GF_MG_EXCEPTION_TRY2

			  try
			  {
			    m_nativeptr->get()->setSubscriptionEnabled( enabled );
			  }
			  finally
			  {
			    GC::KeepAlive(m_nativeptr);
			  }
        return this;

			  _GF_MG_EXCEPTION_CATCH_ALL2
		  }

      CacheFactory^ CacheFactory::SetPRSingleHopEnabled( Boolean enabled )
      {
        _GF_MG_EXCEPTION_TRY2

          try
          {
            m_nativeptr->get()->setPRSingleHopEnabled(enabled);
          }
          finally
          {
            GC::KeepAlive(m_nativeptr);
          }
          return this;

         _GF_MG_EXCEPTION_CATCH_ALL2
      }

		  CacheFactory^ CacheFactory::SetSubscriptionRedundancy( Int32 redundancy )
      {
			  _GF_MG_EXCEPTION_TRY2

          try
          {
            m_nativeptr->get()->setSubscriptionRedundancy( redundancy );
          }
          finally
          {
            GC::KeepAlive(m_nativeptr);
          }
          return this;

			  _GF_MG_EXCEPTION_CATCH_ALL2
		  }

		  CacheFactory^ CacheFactory::SetSubscriptionMessageTrackingTimeout( Int32 messageTrackingTimeout )
      {
			  _GF_MG_EXCEPTION_TRY2

          try
          {
            m_nativeptr->get()->setSubscriptionMessageTrackingTimeout( messageTrackingTimeout );
          }
          finally
          {
            GC::KeepAlive(m_nativeptr);
          }
          return this;

			  _GF_MG_EXCEPTION_CATCH_ALL2
		  }

		  CacheFactory^ CacheFactory::SetSubscriptionAckInterval( Int32 ackInterval )
      {
			  _GF_MG_EXCEPTION_TRY2

          try
          {
            m_nativeptr->get()->setSubscriptionAckInterval( ackInterval );
          }
          finally
          {
            GC::KeepAlive(m_nativeptr);
          }
          return this;

			  _GF_MG_EXCEPTION_CATCH_ALL2
		  }

      CacheFactory^ CacheFactory::SetThreadLocalConnections( bool enabled )
      {
        _GF_MG_EXCEPTION_TRY2

        try
        {
          m_nativeptr->get()->setThreadLocalConnections( enabled );
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }

        _GF_MG_EXCEPTION_CATCH_ALL2

        return this;
      }

      CacheFactory^ CacheFactory::SetMultiuserAuthentication( bool multiuserAuthentication )
      {
			  _GF_MG_EXCEPTION_TRY2

          try
          {
            m_nativeptr->get()->setMultiuserAuthentication( multiuserAuthentication );
          }
          finally
          {
            GC::KeepAlive(m_nativeptr);
          }
          return this;

			  _GF_MG_EXCEPTION_CATCH_ALL2
	   }

			CacheFactory^ CacheFactory::SetPdxIgnoreUnreadFields(bool ignore)
			{
				_GF_MG_EXCEPTION_TRY2

          try
          {
            m_nativeptr->get()->setPdxIgnoreUnreadFields( ignore );
          }
          finally
          {
            GC::KeepAlive(m_nativeptr);
          }
          return this;

			  _GF_MG_EXCEPTION_CATCH_ALL2
			}

      CacheFactory^ CacheFactory::SetPdxReadSerialized(bool pdxReadSerialized)
      {
        	_GF_MG_EXCEPTION_TRY2

          try
          {
            m_nativeptr->get()->setPdxReadSerialized( pdxReadSerialized );
          }
          finally
          {
            GC::KeepAlive(m_nativeptr);
          }
          return this;

			  _GF_MG_EXCEPTION_CATCH_ALL2
      }

      CacheFactory^ CacheFactory::Set(String^ name, String^ value)
      {
        _GF_MG_EXCEPTION_TRY2
          ManagedString mg_name( name );
          ManagedString mg_value( value );
          try
          {
            m_nativeptr->get()->set( mg_name.CharPtr, mg_value.CharPtr );
          }
          finally
          {
            GC::KeepAlive(m_nativeptr);
          }
          return this;

			  _GF_MG_EXCEPTION_CATCH_ALL2
    }  // namespace Client
  }  // namespace Geode
}  // namespace Apache

}
