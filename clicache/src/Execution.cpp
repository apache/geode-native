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
#include "Execution.hpp"
#include "begin_native.hpp"
#include <geode/Execution.hpp>
#include "end_native.hpp"

#include "ResultCollector.hpp"
#include "impl/ManagedResultCollector.hpp"

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
      namespace native = apache::geode::client;

      generic<class TResult>
      generic<class TFilter>
      Execution<TResult>^ Execution<TResult>::WithFilter(System::Collections::Generic::ICollection<TFilter>^ routingObj)
      {
        if (routingObj != nullptr) {
          _GF_MG_EXCEPTION_TRY2/* due to auto replace */
          auto rsptr = native::CacheableVector::create();
        
          for each(TFilter item in routingObj)
          {
            rsptr->push_back(Serializable::GetUnmanagedValueGeneric<TFilter>( item, nullptr ));
          }
          
          try
          {
            return Execution<TResult>::Create(m_nativeptr->get()->withFilter(rsptr), this->m_rc);
          }
          finally
          {
            GC::KeepAlive(m_nativeptr);
          }
          _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */
        }
        else {
          throw gcnew IllegalArgumentException("Execution<TResult>::WithFilter: null TFilter provided");
        }
      }

      generic<class TResult>
      generic<class TArgs>
      Execution<TResult>^ Execution<TResult>::WithArgs( TArgs args )
      {
        _GF_MG_EXCEPTION_TRY2/* due to auto replace */
          try
          {
            auto argsptr = Serializable::GetUnmanagedValueGeneric<TArgs>( args, nullptr );
            return Execution<TResult>::Create(m_nativeptr->get()->withArgs(argsptr), this->m_rc);
          }
          finally
          {
            GC::KeepAlive(m_nativeptr);
          }
        _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */
      }

      generic<class TResult>
      Execution<TResult>^ Execution<TResult>::WithCollector(Client::IResultCollector<TResult>^ rc)
      {
        _GF_MG_EXCEPTION_TRY2/* due to auto replace */
          native::ResultCollectorPtr rcptr;
        if ( rc != nullptr ) {
          auto rcg = gcnew ResultCollectorGeneric<TResult>();
          rcg->SetResultCollector(rc); 
          rcptr = std::shared_ptr<native::ManagedResultCollectorGeneric>(new native::ManagedResultCollectorGeneric(rcg));
        }
        try
        {
          return Execution<TResult>::Create( m_nativeptr->get()->withCollector(rcptr), rc);
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }
        _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */
      }

      generic<class TResult>
      IResultCollector<TResult>^ Execution<TResult>::Execute(String^ func, UInt32 timeout)
      {
        _GF_MG_EXCEPTION_TRY2/* due to auto replace */
        try
        {
          ManagedString mg_function(func);
          auto rc = m_nativeptr->get()->execute(mg_function.CharPtr, timeout);
          if (m_rc == nullptr)
            return gcnew ResultCollector<TResult>(rc);
          else
            return m_rc;
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }
        _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */
      }

      generic<class TResult>
      IResultCollector<TResult>^ Execution<TResult>::Execute(String^ func)
      {
        return Execute(func, DEFAULT_QUERY_RESPONSE_TIMEOUT);
      }
    }  // namespace Client
  }  // namespace Geode
}  // namespace Apache
