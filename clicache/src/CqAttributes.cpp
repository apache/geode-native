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




#include "CqAttributes.hpp"
#include "impl/ManagedCqListener.hpp"
#include "ICqListener.hpp"
#include "impl/ManagedCqStatusListener.hpp"
#include "ICqStatusListener.hpp"

#include "begin_native.hpp"
#include <geode/CqAttributes.hpp>
#include "end_native.hpp"

namespace Apache
{
  namespace Geode
  {
    namespace Client
    {
      using namespace System;

      namespace native = apache::geode::client;

      generic<class TKey, class TResult>
      array<ICqListener<TKey, TResult>^>^ CqAttributes<TKey, TResult>::getCqListeners( )
      {
        native::CqAttributes::listener_container_type vrr;
        try
        {
          vrr = m_nativeptr->get()->getCqListeners();
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }
        auto listners = gcnew array<ICqListener<TKey, TResult>^>(static_cast<int>(vrr.size()));

        for (System::Int32 index = 0; index < vrr.size(); index++)
        {
          auto nativeptr = vrr[index];
          if (auto mg_listener = std::dynamic_pointer_cast<native::ManagedCqListenerGeneric>(nativeptr))
          {
            listners[index] = (ICqListener<TKey, TResult>^) mg_listener->userptr();
          }
          else  if (auto mg_statuslistener = std::dynamic_pointer_cast<native::ManagedCqStatusListenerGeneric>(nativeptr))
          {
            listners[index] = (ICqStatusListener<TKey, TResult>^) mg_statuslistener->userptr();
          }
          else
          {
            listners[index] = nullptr;
          }
        }
        return listners;
      }
    }  // namespace Client
  }  // namespace Geode
}  // namespace Apache
