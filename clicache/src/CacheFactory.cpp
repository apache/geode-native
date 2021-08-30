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
#include "end_native.hpp"

#include "ExceptionTypes.hpp"
#include "CacheFactory.hpp"
#include "Cache.hpp"
#include "DistributedSystem.hpp"
#include "String.hpp"
#include "SystemProperties.hpp"
#include "impl/SafeConvert.hpp"
#include "impl/PdxTypeRegistry.hpp"
#include "impl/AppDomainContext.hpp"
#include "impl/CacheResolver.hpp"
#include "impl/ManagedAuthInitialize.hpp"
#include "ManagedPdxTypeHandler.hpp"
#include "ManagedDataSerializableHandler.hpp"

using namespace System;

namespace Apache
{
  namespace Geode
  {
    namespace Client
    {
      using namespace msclr::interop;
      namespace native = apache::geode::client;

      CacheFactory::CacheFactory() :
          CacheFactory(Properties<String^, String^>::Create())
      {
      }

      CacheFactory::CacheFactory(Properties<String^, String^>^ dsProps) :       
          CacheFactory(native::CacheFactory(dsProps->GetNative()), dsProps)
      {
      }

      Cache^ CacheFactory::Create()
      {
        native::createAppDomainContext = &Apache::Geode::Client::createAppDomainContext;
				bool pdxIgnoreUnreadFields = false;
        bool pdxReadSerialized = false;
        try {
          //msclr::lock lockInstance(m_singletonSync);
          auto nativeCache = std::make_shared<native::Cache>(m_nativeptr->get()->create( ));

          auto cache = Cache::Create( nativeCache );
          CacheResolver::Add(nativeCache.get(), cache);

          DistributedSystem::AppDomainInstanceInitialization(cache);                  

					pdxIgnoreUnreadFields = nativeCache->getPdxIgnoreUnreadFields();
          pdxReadSerialized = nativeCache->getPdxReadSerialized();

          Log::SetLogLevel(static_cast<LogLevel>(native::Log::logLevel( )));

          DistributedSystem::ManagedPostConnect(cache);
          DistributedSystem::registerCliCallback();
          auto&& cacheImpl = CacheRegionHelper::getCacheImpl(nativeCache.get());
          cacheImpl->getSerializationRegistry()->setPdxTypeHandler(new ManagedPdxTypeHandler());
          cacheImpl->getSerializationRegistry()->setDataSerializableHandler(new ManagedDataSerializableHandler());

          return cache;
        }
        catch (const apache::geode::client::Exception& ex) {
          throw Apache::Geode::Client::GeodeException::Get(ex);
        }
        catch (System::AccessViolationException^ ex) {
          throw ex;
        }
        finally {
          GC::KeepAlive(m_nativeptr);
        }
      }
   

      String^ CacheFactory::Version::get( )
      {
        return marshal_as<String^>(native::CacheFactory::getVersion());
      }

      String^ CacheFactory::ProductDescription::get( )
      {
        return marshal_as<String^>(native::CacheFactory::getProductDescription());
      }


			CacheFactory^ CacheFactory::SetPdxIgnoreUnreadFields(bool ignore)
			{
				try {

          try
          {
            m_nativeptr->get()->setPdxIgnoreUnreadFields( ignore );
          }
          finally
          {
            GC::KeepAlive(m_nativeptr);
          }
          return this;

			  }
        catch (const apache::geode::client::Exception& ex) {
          throw Apache::Geode::Client::GeodeException::Get(ex);
        }
        catch (System::AccessViolationException^ ex) {
          throw ex;
        }
			}

      CacheFactory^ CacheFactory::SetPdxReadSerialized(bool pdxReadSerialized)
      {
        try {

          try
          {
            m_nativeptr->get()->setPdxReadSerialized( pdxReadSerialized );
          }
          finally
          {
            GC::KeepAlive(m_nativeptr);
          }
          return this;

			  }
        catch (const apache::geode::client::Exception& ex) {
          throw Apache::Geode::Client::GeodeException::Get(ex);
        }
        catch (System::AccessViolationException^ ex) {
          throw ex;
        }
      }

      CacheFactory^ CacheFactory::Set(String^ name, String^ value)
      {
        try {
          try
          {
            m_nativeptr->get()->set(marshal_as<std::string>(name), to_utf8(value));
          }
          finally
          {
            GC::KeepAlive(m_nativeptr);
          }
          return this;

			  }
        catch (const apache::geode::client::Exception& ex) {
          throw Apache::Geode::Client::GeodeException::Get(ex);
        }
        catch (System::AccessViolationException^ ex) {
          throw ex;
        }
      }

      CacheFactory^ CacheFactory::SetAuthInitialize(IAuthInitialize^ authInitialize)
      {
        try {

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

