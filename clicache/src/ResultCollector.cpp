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



#include "ResultCollector.hpp"
#include "ExceptionTypes.hpp"
#include "impl/SafeConvert.hpp"
#include "TimeUtils.hpp"

using namespace System;

namespace Apache
{
  namespace Geode
  {
    namespace Client
    {

      namespace native = apache::geode::client;

      generic<class TResult>
      void ResultCollector<TResult>::AddResult( const TResult rs )
      {
        throw gcnew UnsupportedOperationException();
      }

      generic<class TResult>
      System::Collections::Generic::ICollection<TResult>^  ResultCollector<TResult>::GetResult()
      {
        return GetResult( TimeUtils::DurationToTimeSpan(DEFAULT_QUERY_RESPONSE_TIMEOUT) );
      }

      generic<class TResult>
      System::Collections::Generic::ICollection<TResult>^  ResultCollector<TResult>::GetResult(TimeSpan timeout)
      {
        try {
          try
          {
            auto results = m_nativeptr->get()->getResult(TimeUtils::TimeSpanToDurationCeil<std::chrono::milliseconds>(timeout));
            auto rs = gcnew array<TResult>(static_cast<int>(results->size()));
            for (System::Int32 index = 0; index < results->size(); index++)
            {
              auto nativeptr = results->operator[](index);
              rs[index] = TypeRegistry::GetManagedValueGeneric<TResult>(nativeptr);
            }
            auto collectionlist = (ICollection<TResult>^)rs;
            return collectionlist;
          }
          finally
          {
            GC::KeepAlive(m_nativeptr);
          }

        }
        catch (const apache::geode::client::Exception& ex) {
          throw Apache::Geode::Client::GeodeException::Get(ex);
        }
        catch (System::AccessViolationException^ ex) {
          throw ex;
        }
      }

      generic<class TResult>
      void ResultCollector<TResult>::EndResults()
      {
        try {
          try
          {
            m_nativeptr->get()->endResults();
          }
          finally
          {
            GC::KeepAlive(m_nativeptr);
          }
        }
        catch (const apache::geode::client::Exception& ex) {
          throw Apache::Geode::Client::GeodeException::Get(ex);
        }
        catch (System::AccessViolationException^ ex) {
          throw ex;
        }
      }

      generic<class TResult>
      void ResultCollector<TResult>::ClearResults(/*bool*/)
      {
        try {
          try
          {
            m_nativeptr->get()->clearResults();
          }
          finally
          {
            GC::KeepAlive(m_nativeptr);
          }
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
