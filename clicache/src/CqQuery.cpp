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



#include "CqQuery.hpp"
#include "Query.hpp"
#include "CqAttributes.hpp"
#include "CqAttributesMutator.hpp"
#include "CqStatistics.hpp"
#include "ISelectResults.hpp"
#include "ResultSet.hpp"
#include "StructSet.hpp"
#include "ExceptionTypes.hpp"
#include "TimeUtils.hpp"

namespace Apache
{
  namespace Geode
  {
    namespace Client
    {
      using namespace msclr::interop;
      using namespace System;

      namespace native = apache::geode::client;

      generic<class TKey, class TResult>
      ICqResults<TResult>^ CqQuery<TKey, TResult>::ExecuteWithInitialResults()
      {
        return ExecuteWithInitialResults(TimeUtils::DurationToTimeSpan(native::DEFAULT_QUERY_RESPONSE_TIMEOUT));
      }

      generic<class TKey, class TResult>
      ICqResults<TResult>^ CqQuery<TKey, TResult>::ExecuteWithInitialResults(TimeSpan timeout)
      {
        _GF_MG_EXCEPTION_TRY2/* due to auto replace */
          try
          {
            auto nativeptr = m_nativeptr->get()->executeWithInitialResults(TimeUtils::TimeSpanToDurationCeil<std::chrono::milliseconds>(timeout));
 
            if (auto structptr = std::dynamic_pointer_cast<native::StructSet>(nativeptr))
            {
              return StructSet<TResult>::Create(structptr);
            }

            return nullptr;
          }
          finally
          {
            GC::KeepAlive(m_nativeptr);
          }
        _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */
      }

      generic<class TKey, class TResult>
      void CqQuery<TKey, TResult>::Execute()
      {
        _GF_MG_EXCEPTION_TRY2/* due to auto replace */

          try
          {
            m_nativeptr->get()->execute();
          }
          finally
          {
            GC::KeepAlive(m_nativeptr);
          }

        _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */
      }

      generic<class TKey, class TResult>
      String^ CqQuery<TKey, TResult>::QueryString::get( )
      {
        try
        {
          return marshal_as<String^>(m_nativeptr->get()->getQueryString());
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }
      }

      generic<class TKey, class TResult>
      String^ CqQuery<TKey, TResult>::Name::get( )
      {
        try
        {
          return marshal_as<String^>(m_nativeptr->get()->getName());
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }
      }

      generic<class TKey, class TResult>
      Query<TResult>^ CqQuery<TKey, TResult>::GetQuery( )
      {
        try
        {
          return Query<TResult>::Create(m_nativeptr->get()->getQuery());
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }
      }

      generic<class TKey, class TResult>
      CqAttributes<TKey, TResult>^ CqQuery<TKey, TResult>::GetCqAttributes( )
      {
        try
        {
          return CqAttributes<TKey, TResult>::Create(m_nativeptr->get()->getCqAttributes( ));
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }
      }

      generic<class TKey, class TResult>
      CqAttributesMutator<TKey, TResult>^ CqQuery<TKey, TResult>::GetCqAttributesMutator( )
      {
        try
        {
          return CqAttributesMutator<TKey, TResult>::Create(
            new native::CqAttributesMutator(m_nativeptr->get()->getCqAttributesMutator()));
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }
      }

      generic<class TKey, class TResult>
      CqStatistics^ CqQuery<TKey, TResult>::GetStatistics( )
      {
        try
        {
          return CqStatistics::Create(m_nativeptr->get()->getStatistics());
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }
      }

      generic<class TKey, class TResult>
      CqState CqQuery<TKey, TResult>::GetState( )
      {
        try
        {
          return CqState(m_nativeptr->get()->getState());
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }
      }

      generic<class TKey, class TResult>
      void CqQuery<TKey, TResult>::Stop( )
      {
        _GF_MG_EXCEPTION_TRY2/* due to auto replace */

          try
          {
            m_nativeptr->get()->stop( );
          }
          finally
          {
            GC::KeepAlive(m_nativeptr);
          }

        _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */
      }

      generic<class TKey, class TResult>
      void CqQuery<TKey, TResult>::Close( )
      {
        _GF_MG_EXCEPTION_TRY2/* due to auto replace */

          try
          {
            m_nativeptr->get()->close( );
          }
          finally
          {
            GC::KeepAlive(m_nativeptr);
          }

        _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */
      }

      generic<class TKey, class TResult>
      bool CqQuery<TKey, TResult>::IsRunning( )
      {
        _GF_MG_EXCEPTION_TRY2/* due to auto replace */

          try
          {
            return m_nativeptr->get()->isRunning( );
          }
          finally
          {
            GC::KeepAlive(m_nativeptr);
          }

        _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */
      }

      generic<class TKey, class TResult>
      bool CqQuery<TKey, TResult>::IsStopped( )
      {
        _GF_MG_EXCEPTION_TRY2/* due to auto replace */

          try
          {
            return m_nativeptr->get()->isStopped( );
          }
          finally
          {
            GC::KeepAlive(m_nativeptr);
          }

        _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */
      }

      generic<class TKey, class TResult>
        bool CqQuery<TKey, TResult>::IsClosed()
        {
          _GF_MG_EXCEPTION_TRY2/* due to auto replace */

            try
          {
            return m_nativeptr->get()->isClosed();
          }
          finally
          {
            GC::KeepAlive(m_nativeptr);
          }

          _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */
      }
    }  // namespace Client
  }  // namespace Geode
}  // namespace Apache
