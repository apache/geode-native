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
#include "QueryService.hpp"
#include "Query.hpp"
#include "Log.hpp"
#include "CqAttributes.hpp"
#include "CqQuery.hpp"
#include "CqServiceStatistics.hpp"
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

      generic<class TKey, class TResult>
      //generic<class TResult>
      Query<TResult>^ QueryService<TKey, TResult>::NewQuery(String^ query)
      {
        ManagedString mg_queryStr(query);
        try
        {
          return Query<TResult>::Create(m_nativeptr->get()->newQuery(
            mg_queryStr.CharPtr));
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
      CqQuery<TKey, TResult>^ QueryService<TKey, TResult>::NewCq(String^ query, CqAttributes<TKey, TResult>^ cqAttr, bool isDurable)
      {
        ManagedString mg_queryStr(query);
        try
        {
          return CqQuery<TKey, TResult>::Create(m_nativeptr->get()->newCq(
            mg_queryStr.CharPtr, cqAttr->GetNative(), isDurable));
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
      CqQuery<TKey, TResult>^ QueryService<TKey, TResult>::NewCq(String^ name, String^ query, CqAttributes<TKey, TResult>^ cqAttr, bool isDurable)
      {
        ManagedString mg_queryStr(query);
        ManagedString mg_nameStr(name);
        try
        {
          return CqQuery<TKey, TResult>::Create(m_nativeptr->get()->newCq(
            mg_nameStr.CharPtr, mg_queryStr.CharPtr, cqAttr->GetNative(), isDurable));
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
      void QueryService<TKey, TResult>::CloseCqs()
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
      array<CqQuery<TKey, TResult>^>^ QueryService<TKey, TResult>::GetCqs()
      {
        try
        {
          apache::geode::client::QueryService::query_container_type vrr =
              m_nativeptr->get()->getCqs();
          auto cqs = gcnew array<CqQuery<TKey, TResult>^>(vrr.size());

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
      CqQuery<TKey, TResult>^ QueryService<TKey, TResult>::GetCq(String^ name)
      {
        ManagedString mg_queryStr(name);
        try
        {
          return CqQuery<TKey, TResult>::Create(m_nativeptr->get()->getCq(
            mg_queryStr.CharPtr));
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
      void QueryService<TKey, TResult>::ExecuteCqs()
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

      generic<class TKey, class TResult>
      void QueryService<TKey, TResult>::StopCqs()
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

      generic<class TKey, class TResult>
      CqServiceStatistics^ QueryService<TKey, TResult>::GetCqStatistics()
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

      generic<class TKey, class TResult>
      System::Collections::Generic::List<String^>^ QueryService<TKey, TResult>::GetAllDurableCqsFromServer()
      {
        try
        {
          auto durableCqsArrayListPtr = m_nativeptr->get()->getAllDurableCqsFromServer();
          auto durableCqsList = gcnew System::Collections::Generic::List<String^>();
          for (const auto& d : *durableCqsArrayListPtr)
          {
            durableCqsList->Add(CacheableString::GetString(std::static_pointer_cast<apache::geode::client::CacheableString>(d)));
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
