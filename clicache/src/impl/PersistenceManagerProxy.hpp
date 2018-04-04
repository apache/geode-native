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


#include "../IPersistenceManager.hpp"
#include "SafeConvert.hpp"
#include "../Region.hpp"
#include "../Properties.hpp"
#include "../begin_native.hpp"
#include <memory>
#include "../end_native.hpp"
using namespace System;
namespace Apache
{
  namespace Geode
  {
    namespace Client
    {
		namespace native = apache::geode::client;
        public interface class IPersistenceManagerProxy
        {
          public:
            void write(const std::shared_ptr<native::CacheableKey>&  key, const std::shared_ptr<native::Cacheable>&  value/*, void *& PersistenceInfo*/);
            bool writeAll();
            void init(const std::shared_ptr<native::Region>& region, const std::shared_ptr<native::Properties>& diskProperties);
            std::shared_ptr<native::Cacheable> read(const std::shared_ptr<native::CacheableKey>& key/*, void *& PersistenceInfo*/);
            bool readAll();
            void destroy(const std::shared_ptr<native::CacheableKey>& key/*, void *& PersistenceInfo*/);
            void close();
        };

        generic<class TKey, class TValue>
        public ref class PersistenceManagerGeneric : IPersistenceManagerProxy 
        {
          private:
            IPersistenceManager<TKey, TValue>^ m_persistenceManager;
          public:
            virtual void SetPersistenceManager(IPersistenceManager<TKey, TValue>^ persistenceManager)
            {
              m_persistenceManager = persistenceManager;
            }
            virtual void write(const std::shared_ptr<native::CacheableKey>&  key, const std::shared_ptr<native::Cacheable>&  value/*, void *& PersistenceInfo*/)
            {
               TKey gKey = TypeRegistry::GetManagedValueGeneric<TKey>(key);
               TValue gValue = TypeRegistry::GetManagedValueGeneric<TValue>(value);
               m_persistenceManager->Write(gKey, gValue);
            }
            
            virtual bool writeAll()
            {
              throw gcnew System::NotSupportedException;
            }

            virtual void init(const std::shared_ptr<native::Region>& region, const std::shared_ptr<native::Properties>& diskProperties)
            {
              auto gRegion = Region<TKey, TValue>::Create(region);
              auto gProps = Properties<String^, String^>::Create(diskProperties);
              m_persistenceManager->Init(gRegion, gProps);
            }
            
            virtual std::shared_ptr<native::Cacheable> read(const std::shared_ptr<native::CacheableKey>& key/*, void *& PersistenceInfo*/)
            {
              TKey gKey = TypeRegistry::GetManagedValueGeneric<TKey>(key);
              return Serializable::GetUnmanagedValueGeneric<TValue>(m_persistenceManager->Read(gKey));
            }
            
            virtual bool readAll()
            {
              throw gcnew System::NotSupportedException;
            }
            
            virtual void destroy(const std::shared_ptr<native::CacheableKey>& key/*, void *& PersistenceInfo*/)
            {
              TKey gKey = TypeRegistry::GetManagedValueGeneric<TKey>(key);
              m_persistenceManager->Destroy(gKey);
            }
            
            virtual void close()
            {
              m_persistenceManager->Close();
            }
        };
    }  // namespace Client
  }  // namespace Geode
}  // namespace Apache


