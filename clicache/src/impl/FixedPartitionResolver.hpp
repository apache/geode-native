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

#include <msclr/marshal_cppstd.h>

#include "../IFixedPartitionResolver.hpp"
#include "../Region.hpp"
#include "SafeConvert.hpp"
#include "../native_shared_ptr.hpp"

using namespace System;
using namespace System::Collections::Concurrent;
using namespace System::Threading;

namespace Apache
{
  namespace Geode
  {
    namespace Client
    {
      using namespace msclr::interop;

      public interface class IFixedPartitionResolverProxy
      {
      public:
        std::shared_ptr<apache::geode::client::CacheableKey> getRoutingObject(const apache::geode::client::EntryEvent& ev);
        const std::string& getName();
        const std::string& getPartitionName(const apache::geode::client::EntryEvent& opDetails);       
      };

      generic<class TKey, class TValue>
      public ref class FixedPartitionResolverGeneric : IFixedPartitionResolverProxy
      {
        private:

          IPartitionResolver<TKey, TValue>^ m_resolver;
          IFixedPartitionResolver<TKey, TValue>^ m_fixedResolver;
          ConcurrentDictionary<String^, native_shared_ptr<std::string>^>^ m_partitionNames;
        public:

          void SetPartitionResolver(IPartitionResolver<TKey, TValue>^ resolver)
          {            
            m_resolver = resolver;
            m_fixedResolver = dynamic_cast<IFixedPartitionResolver<TKey, TValue>^>(resolver);
            m_partitionNames = gcnew ConcurrentDictionary<String^, native_shared_ptr<std::string>^>();
          }

          virtual std::shared_ptr<apache::geode::client::CacheableKey> getRoutingObject(const apache::geode::client::EntryEvent& ev)
          {
            EntryEvent<TKey, TValue> gevent(&ev);
			      Object^ groutingobject = m_resolver->GetRoutingObject(%gevent);
            return Serializable::GetUnmanagedValueGeneric<Object^>(groutingobject);
          }

          virtual const std::string& getName()
          {
            static const std::string name = marshal_as<std::string>(m_resolver->GetName());
            return name;
          }

          virtual const std::string& getPartitionName(const apache::geode::client::EntryEvent& opDetails)
          {
            if (m_fixedResolver == nullptr)
            {
              throw apache::geode::client::IllegalStateException("GetPartitionName() called on non fixed partition resolver.");
            }

            EntryEvent<TKey, TValue> gevent(&opDetails);                        
            String^ managedString = m_fixedResolver->GetPartitionName(%gevent);

            native_shared_ptr<std::string>^ unmanagedString = nullptr;
            if(!m_partitionNames->TryGetValue(managedString, unmanagedString))
            {
              unmanagedString = gcnew native_shared_ptr<std::string>(std::shared_ptr<std::string>(
                new std::string(marshal_as<std::string>(managedString))));
              m_partitionNames->TryAdd(managedString, unmanagedString);
            }
            
            return *(unmanagedString->get());
          }
      };
    }  // namespace Client
  }  // namespace Geode
}  // namespace Apache

