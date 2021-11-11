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


#include "Query.hpp"
#include "ISelectResults.hpp"
#include "ResultSet.hpp"
#include "String.hpp"
#include "StructSet.hpp"
#include "ExceptionTypes.hpp"
#include "impl/SafeConvert.hpp"
#include "TimeUtils.hpp"


namespace Apache
{
  namespace Geode
  {
    namespace Client
    {

      using namespace System;
      using namespace msclr::interop;

      namespace native = apache::geode::client;

      generic<class TResult>
      ISelectResults<TResult>^ Query<TResult>::Execute(  )
      {
        return Execute( TimeUtils::DurationToTimeSpan(native::DEFAULT_QUERY_RESPONSE_TIMEOUT) );
      }

      generic<class TResult>
      ISelectResults<TResult>^ Query<TResult>::Execute( TimeSpan timeout )
      {
        _GF_MG_EXCEPTION_TRY2/* due to auto replace */

          try
          {
            return WrapResults( m_nativeptr->get()->execute( TimeUtils::TimeSpanToDurationCeil<std::chrono::milliseconds>(timeout) ));
          }
          finally
          {
            GC::KeepAlive(m_nativeptr);
          }        

        _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */
      }
	
      generic<class TResult>
      ISelectResults<TResult>^ Query<TResult>::Execute( array<Object^>^ paramList)
      {
        return Execute(paramList, TimeUtils::DurationToTimeSpan(native::DEFAULT_QUERY_RESPONSE_TIMEOUT));
      }

      generic<class TResult>
      ISelectResults<TResult>^ Query<TResult>::Execute( array<Object^>^ paramList, TimeSpan timeout )
      {
        _GF_MG_EXCEPTION_TRY2/* due to auto replace */

          auto rsptr = apache::geode::client::CacheableVector::create();
          for( int index = 0; index < paramList->Length; index++ )
          {
            auto valueptr = Serializable::GetUnmanagedValueGeneric<Object^>(paramList[index]->GetType(), (Object^)paramList[index]);
            rsptr->push_back(valueptr);
		      }

          try
          {
            return WrapResults( m_nativeptr->get()->execute(rsptr, TimeUtils::TimeSpanToDurationCeil<std::chrono::milliseconds>(timeout) ));
          }
          finally
          {
            GC::KeepAlive(m_nativeptr);
          }

        _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */
      }

      generic<class TResult>
      ISelectResults<TResult>^ Query<TResult>::WrapResults(const std::shared_ptr<apache::geode::client::SelectResults>& selectResults)
      {
        if ( __nullptr == selectResults ) return nullptr;

        if (auto resultptr = std::dynamic_pointer_cast<apache::geode::client::ResultSet>(selectResults))
        {
          return ResultSet<TResult>::Create(resultptr);
        }
        else if (auto structptr = std::dynamic_pointer_cast<apache::geode::client::StructSet>(selectResults))
        {
          return StructSet<TResult>::Create(structptr);
        }

        return nullptr;
      }

      generic<class TResult>
      String^ Query<TResult>::QueryString::get( )
      {
        try
        {
          return to_String(m_nativeptr->get()->getQueryString());
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }
      }

      generic<class TResult>
      void Query<TResult>::Compile( )
      {
        _GF_MG_EXCEPTION_TRY2/* due to auto replace */

          try
          {
            m_nativeptr->get()->compile( );
          }
          finally
          {
            GC::KeepAlive(m_nativeptr);
          }

        _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */
      }

      generic<class TResult>
      bool Query<TResult>::IsCompiled::get()
      {
        _GF_MG_EXCEPTION_TRY2/* due to auto replace */

          try
          {
            return m_nativeptr->get()->isCompiled();
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
