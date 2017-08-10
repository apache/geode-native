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
#include "Pool.hpp"
#include "QueryService.hpp"
#include "CacheableString.hpp"
#include "Cache.hpp"
//#include "Properties.hpp"
#include "impl/ManagedString.hpp"
#include "ExceptionTypes.hpp"
#include "impl/SafeConvert.hpp"


using namespace System;

namespace Apache
{
  namespace Geode
  {
    namespace Client
    {


      String^ Pool::Name::get( )
      {
        try
        {
          return ManagedString::Get( m_nativeptr->get()->getName( ) );
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }

      }


      Int32 Pool::FreeConnectionTimeout::get()
      {
        try
        {
          return m_nativeptr->get()->getFreeConnectionTimeout();
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }

      }


      Int32 Pool::LoadConditioningInterval::get()
      {
        try
        {
          return m_nativeptr->get()->getLoadConditioningInterval();
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }

      }


      Int32 Pool::SocketBufferSize::get()
      {
        try
        {
          return m_nativeptr->get()->getSocketBufferSize();
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }

      }


      Int32 Pool::ReadTimeout::get()
      {
        try
        {
          return m_nativeptr->get()->getReadTimeout();
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }

      }


      Int32 Pool::MinConnections::get()
      {
        try
        {
          return m_nativeptr->get()->getMinConnections();
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }

      }


      Int32 Pool::MaxConnections::get()
      {
        try
        {
          return m_nativeptr->get()->getMaxConnections();
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }

      }


      Int32 Pool::IdleTimeout::get()
      {
        try
        {
          return m_nativeptr->get()->getIdleTimeout();
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }

      }


      Int32 Pool::PingInterval::get()
      {
        try
        {
          return m_nativeptr->get()->getPingInterval();
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }

      }


      Int32 Pool::UpdateLocatorListInterval::get()
      {
        try
        {
          return m_nativeptr->get()->getUpdateLocatorListInterval();
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }

      }


      Int32 Pool::StatisticInterval::get()
      {
        try
        {
          return m_nativeptr->get()->getStatisticInterval();
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }

      }


      Int32 Pool::RetryAttempts::get()
      {
        try
        {
          return m_nativeptr->get()->getRetryAttempts();
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }

      }


      Boolean Pool::SubscriptionEnabled::get()
      {
        try
        {
          return m_nativeptr->get()->getSubscriptionEnabled();
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }

      }


      Boolean Pool::PRSingleHopEnabled::get()
      {
        try
        {
          return m_nativeptr->get()->getPRSingleHopEnabled();
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }

      }


      Int32 Pool::SubscriptionRedundancy::get()
      {
        try
        {
          return m_nativeptr->get()->getSubscriptionRedundancy();
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }

      }


      Int32 Pool::SubscriptionMessageTrackingTimeout::get()
      {
        try
        {
          return m_nativeptr->get()->getSubscriptionMessageTrackingTimeout();
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }

      }


      Int32 Pool::SubscriptionAckInterval::get()
      {
        try
        {
          return m_nativeptr->get()->getSubscriptionAckInterval();
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }

      }


      String^ Pool::ServerGroup::get( )
      {
        try
        {
          return ManagedString::Get( m_nativeptr->get()->getServerGroup( ) );
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }

      }


      array<String^>^ Pool::Locators::get()
      {
        try
        {
          auto locators = m_nativeptr->get()->getLocators();
          int length = locators->length();
          if (length > 0)
          {
            array<String^>^ result = gcnew array<String^>(length);
            for (int item = 0; item < length; item++)
            {
              result[item] = CacheableString::GetString((*locators)[item].get());
            }
            return result;
          }
          else
          {
            return nullptr;
          }
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }
      }


      array<String^>^ Pool::Servers::get()
      {
        try
        {
          auto servers = m_nativeptr->get()->getServers();
          int length = servers->length();
          if (length > 0)
          {
            array<String^>^ result = gcnew array<String^>(length);
            for (int item = 0; item < length; item++)
            {
              result[item] = CacheableString::GetString((*servers)[item].get());
            }
            return result;
          }
          else
          {
            return nullptr;
          }
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }
      }

	  //generic<class TKey, class TValue>
      Boolean Pool::ThreadLocalConnections::get()
      {
        try
        {
          return m_nativeptr->get()->getThreadLocalConnections();
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }

      }


      bool Pool::MultiuserAuthentication::get()
      {
        try
        {
          return m_nativeptr->get()->getMultiuserAuthentication();
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }
    
      }
      

      void Pool::Destroy(Boolean KeepAlive)
      {
        try
        {
          m_nativeptr->get()->destroy(KeepAlive);
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }

      }

      void Pool::ReleaseThreadLocalConnection()
      {
        try
        {
          m_nativeptr->get()->releaseThreadLocalConnection();
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }

      }


      void Pool::Destroy()
      {
        try
        {
          m_nativeptr->get()->destroy();
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }

      }


      Boolean Pool::Destroyed::get()
      {
        try
        {
          return m_nativeptr->get()->isDestroyed();
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }

      }

      generic<class TKey, class TResult>
      QueryService<TKey, TResult>^ Pool::GetQueryService()
      {
        _GF_MG_EXCEPTION_TRY2/* due to auto replace */

          try
          {
            return QueryService<TKey, TResult>::Create(m_nativeptr->get()->getQueryService());
          }
          finally
          {
            GC::KeepAlive(m_nativeptr);
          }


        _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */
      }
      
      Int32 Pool::PendingEventCount::get()
      {
        _GF_MG_EXCEPTION_TRY2

          try
          {
            return m_nativeptr->get()->getPendingEventCount();
          }
          finally
          {
            GC::KeepAlive(m_nativeptr);
          }


        _GF_MG_EXCEPTION_CATCH_ALL2
      }
    }  // namespace Client
  }  // namespace Geode
}  // namespace Apache
