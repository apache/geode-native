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



#include "QueryService.hpp"
#include "Query.hpp"
#include "Log.hpp"
#include "CqAttributes.hpp"
#include "CqQuery.hpp"
#include "CqServiceStatistics.hpp"
#include "ExceptionTypes.hpp"
#include "String.hpp"
#include "impl/ManagedString.hpp"
#include "impl/SafeConvert.hpp"

using namespace System;

namespace Apache
{
  namespace Geode
  {
    namespace Client
    {

      generic<class TResult>
      Query<TResult>^ QueryService::NewQuery(String^ query)
      {
        try
        {
          return Query<TResult>::Create(m_nativeptr->get()->newQuery(
            Apache::Geode::Client::to_utf8(query)));
        }
        catch (const apache::geode::client::Exception& ex)
        {
          throw GeodeException::Get(ex);
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }
      }

      generic<class TKey, class TResult>
        CqQuery<TKey, TResult>^ QueryService::NewCq(String^ query, CqAttributes<TKey, TResult>^ cqAttr, bool isDurable)
        {
          try
          {
            return CqQuery<TKey, TResult>::Create(m_nativeptr->get()->newCq(
              Apache::Geode::Client::to_utf8(query), cqAttr->GetNative(), isDurable));
          }
          catch (const apache::geode::client::Exception& ex)
          {
            throw GeodeException::Get(ex);
          }
          finally
          {
            GC::KeepAlive(m_nativeptr);
          }
        }

    
      generic<class TKey, class TResult>
      CqQuery<TKey, TResult>^ QueryService::NewCq(String^ name, String^ query, CqAttributes<TKey, TResult>^ cqAttr, bool isDurable)
      {
        try
        {
          return CqQuery<TKey, TResult>::Create(m_nativeptr->get()->newCq(
            Apache::Geode::Client::to_utf8(name), Apache::Geode::Client::to_utf8(query), cqAttr->GetNative(), isDurable));
        }
        catch (const apache::geode::client::Exception& ex)
        {
          throw GeodeException::Get(ex);
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }
      }

      void QueryService::CloseCqs()
      {
        try
        {
          m_nativeptr->get()->closeCqs();
        }
        catch (const apache::geode::client::Exception& ex)
        {
          throw GeodeException::Get(ex);
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }
      }

      generic<class TKey, class TResult>
      array<CqQuery<TKey, TResult>^>^ QueryService::GetCqs()
      {
        try
        {
          apache::geode::client::QueryService::query_container_type vrr =
              m_nativeptr->get()->getCqs();
          auto cqs = gcnew array<CqQuery<TKey, TResult>^>(static_cast<int>(vrr.size()));

          for (System::Int32 index = 0; index < vrr.size(); index++)
          {
            cqs[index] = CqQuery<TKey, TResult>::Create(vrr[index]);
          }
          return cqs;
        }
        catch (const apache::geode::client::Exception& ex)
        {
          throw GeodeException::Get(ex);
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }
      }

      generic<class TKey, class TResult>
      CqQuery<TKey, TResult>^ QueryService::GetCq(String^ name)
      {
        try
        {
          return CqQuery<TKey, TResult>::Create(m_nativeptr->get()->getCq(
            Apache::Geode::Client::to_utf8(name)));
        }
        catch (const apache::geode::client::Exception& ex)
        {
          throw GeodeException::Get(ex);
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }
      }

      void QueryService::ExecuteCqs()
      {
        try
        {
          m_nativeptr->get()->executeCqs();
        }
        catch (const apache::geode::client::Exception& ex)
        {
          throw GeodeException::Get(ex);
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }
      }

      void QueryService::StopCqs()
      {
        try
        {
          m_nativeptr->get()->stopCqs();
        }
        catch (const apache::geode::client::Exception& ex)
        {
          throw GeodeException::Get(ex);
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }
      }

      CqServiceStatistics^ QueryService::GetCqStatistics()
      {
        try
        {
          return CqServiceStatistics::Create(m_nativeptr->get()->getCqServiceStatistics());
        }
        catch (const apache::geode::client::Exception& ex)
        {
          throw GeodeException::Get(ex);
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }
      }

      System::Collections::Generic::List<String^>^ QueryService::GetAllDurableCqsFromServer()
      {
        try
        {
          auto durableCqsArrayListPtr = m_nativeptr->get()->getAllDurableCqsFromServer();
          auto durableCqsList = gcnew System::Collections::Generic::List<String^>();
          for (const auto& d : *durableCqsArrayListPtr)
          {
            durableCqsList->Add(to_String(std::dynamic_pointer_cast<apache::geode::client::CacheableString>(d)->value()));
          }
          return durableCqsList;
        }
        catch (const apache::geode::client::Exception& ex)
        {
          throw GeodeException::Get(ex);
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }
      }
    }  // namespace Client
  }  // namespace Geode
}  // namespace Apache
