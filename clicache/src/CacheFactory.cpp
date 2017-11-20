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

#include "ExceptionTypes.hpp"
#include "CacheFactory.hpp"
#include "Cache.hpp"
#include "DistributedSystem.hpp"
#include "SystemProperties.hpp"
#include "impl/SafeConvert.hpp"
#include "impl/PdxTypeRegistry.hpp"
#include "impl/AppDomainContext.hpp"
#include "impl/CacheResolver.hpp"
#include "impl/ManagedAuthInitialize.hpp"

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
          auto nativeCache = std::make_shared<native::Cache>(m_nativeptr->get()->create( ));

          auto cache = Cache::Create( nativeCache );
          CacheResolver::Add(nativeCache.get(), cache);

          if(!m_connected)
          {
            DistributedSystem::AppDomainInstanceInitialization(cache);                  
          }

					pdxIgnoreUnreadFields = nativeCache->getPdxIgnoreUnreadFields();
          pdxReadSerialized = nativeCache->getPdxReadSerialized();

          appDomainEnable = cache->DistributedSystem->SystemProperties->AppDomainEnabled;
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
            nullptr, cache);

          if(!m_connected)
          {
            //it registers types in unmanage layer, so should be once only 
            DistributedSystem::ManagedPostConnect(cache);
            DistributedSystem::AppDomainInstancePostInitialization();
            DistributedSystem::connectInstance();
          }
          
          m_connected = true;
 
          DistributedSystem::registerCliCallback();
          Serializable::RegisterPDXManagedCacheableKey(cache);

          return cache;
        _GF_MG_EXCEPTION_CATCH_ALL2
        finally {
          GC::KeepAlive(m_nativeptr);
           DistributedSystem::releaseDisconnectLock();
        }
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
      }

      CacheFactory^ CacheFactory::SetAuthInitialize(IAuthInitialize^ authInitialize)
      {
        	_GF_MG_EXCEPTION_TRY2

          try
          {
            std::shared_ptr<ManagedAuthInitializeGeneric> nativeAuthInitialize;
            if (authInitialize != nullptr)
            {
              nativeAuthInitialize.reset(new ManagedAuthInitializeGeneric(authInitialize));
            }
            m_nativeptr->get()->setAuthInitialize( nativeAuthInitialize);
          }
          finally
          {
            GC::KeepAlive(m_nativeptr);
          }
          return this;

			  _GF_MG_EXCEPTION_CATCH_ALL2
      }

    }  // namespace Client
  }  // namespace Geode
}  // namespace Apache

