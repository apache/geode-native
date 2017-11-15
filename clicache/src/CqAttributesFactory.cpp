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



#include "begin_native.hpp"
#include <geode/QueryService.hpp>
#include "end_native.hpp"


#include "CqAttributesFactory.hpp"
#include "impl/ManagedCqListener.hpp"
#include "ICqListener.hpp"
#include "impl/ManagedCqStatusListener.hpp"
#include "ICqStatusListener.hpp"
#include "CqAttributesMutator.hpp"

using namespace System;

namespace Apache
{
  namespace Geode
  {
    namespace Client
    {
      namespace native = apache::geode::client;

      generic<class TKey, class TResult>
      void CqAttributesFactory<TKey, TResult>::AddCqListener( Client::ICqListener<TKey, TResult>^ cqListener )
      {
        native_shared_ptr<native::CqListener>^ listenerptr;
        if ( cqListener != nullptr ) {
          if (auto cqStatusListener = dynamic_cast<ICqStatusListener<TKey, TResult>^>(cqListener)) {
            auto sLstr = gcnew CqStatusListenerGeneric<TKey, TResult>();
            sLstr->AddCqListener(cqListener);
            listenerptr = gcnew native_shared_ptr<native::CqListener>(std::shared_ptr<native::ManagedCqStatusListenerGeneric>(new native::ManagedCqStatusListenerGeneric(cqListener)));
            try {
              CqListenerHelper<TKey, TResult>::g_readerWriterLock->AcquireWriterLock(-1);
              if ( CqListenerHelper<TKey, TResult>::m_ManagedVsUnManagedCqLstrDict->ContainsKey(cqListener) ) {
                CqListenerHelper<TKey, TResult>::m_ManagedVsUnManagedCqLstrDict[cqListener] = listenerptr;
              }
              else {
                CqListenerHelper<TKey, TResult>::m_ManagedVsUnManagedCqLstrDict->Add(cqListener, listenerptr);
              }
            }
            finally {
              CqListenerHelper<TKey, TResult>::g_readerWriterLock->ReleaseWriterLock();
            }
            ((native::ManagedCqStatusListenerGeneric*)listenerptr->get())->setptr(sLstr);
          }
          else {
            //TODO::split
            auto cqlg = gcnew CqListenerGeneric<TKey, TResult>();
            cqlg->AddCqListener(cqListener);
            listenerptr = gcnew native_shared_ptr<native::CqListener>(std::shared_ptr<native::ManagedCqListenerGeneric>(new native::ManagedCqListenerGeneric(cqListener)));
            try {
              CqListenerHelper<TKey, TResult>::g_readerWriterLock->AcquireWriterLock(-1);
              if ( CqListenerHelper<TKey, TResult>::m_ManagedVsUnManagedCqLstrDict->ContainsKey(cqListener) ) {
                CqListenerHelper<TKey, TResult>::m_ManagedVsUnManagedCqLstrDict[cqListener] = listenerptr; 
              }
              else {
                CqListenerHelper<TKey, TResult>::m_ManagedVsUnManagedCqLstrDict->Add(cqListener, listenerptr);
              }
            } finally {
                CqListenerHelper<TKey, TResult>::g_readerWriterLock->ReleaseWriterLock();
            }
            ((native::ManagedCqListenerGeneric*)listenerptr->get())->setptr(cqlg);            
          }
        }
        try
        {
          m_nativeptr->get()->addCqListener( listenerptr->get_shared_ptr() );
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }
      }

      generic<class TKey, class TResult>
      void CqAttributesFactory<TKey, TResult>::InitCqListeners(array<Client::ICqListener<TKey, TResult>^>^ newListeners)
      {
        native::CqAttributes::listener_container_type vrr;
        for( int i = 0; i < newListeners->Length; i++ )
        {
          if (auto lister = dynamic_cast<Client::ICqStatusListener<TKey, TResult>^>(newListeners[i])) {
            auto cptr = gcnew native_shared_ptr<native::CqListener>(std::shared_ptr<native::ManagedCqStatusListenerGeneric>(new native::ManagedCqStatusListenerGeneric(lister)));
            vrr.push_back(cptr->get_shared_ptr());
            auto cqlg = gcnew CqStatusListenerGeneric<TKey, TResult>();
            cqlg->AddCqListener(newListeners[i]);
            try {
              CqListenerHelper<TKey, TResult>::g_readerWriterLock->AcquireWriterLock(-1);
              if ( CqListenerHelper<TKey, TResult>::m_ManagedVsUnManagedCqLstrDict->ContainsKey( newListeners[i]) ) {
                CqListenerHelper<TKey, TResult>::m_ManagedVsUnManagedCqLstrDict[newListeners[i]] = cptr;
              }
              else {
                CqListenerHelper<TKey, TResult>::m_ManagedVsUnManagedCqLstrDict->Add(newListeners[i], cptr);
              }
            } finally {
                CqListenerHelper<TKey, TResult>::g_readerWriterLock->ReleaseWriterLock();
            }
            ((native::ManagedCqStatusListenerGeneric*)cptr->get())->setptr(cqlg);
          }
          else {
            auto cptr = gcnew native_shared_ptr<native::CqListener>(std::shared_ptr<native::ManagedCqListenerGeneric>(new native::ManagedCqListenerGeneric(newListeners[i])));
            vrr.push_back(cptr->get_shared_ptr());
            auto cqlg = gcnew CqListenerGeneric<TKey, TResult>();
            cqlg->AddCqListener(newListeners[i]);
            try {
              CqListenerHelper<TKey, TResult>::g_readerWriterLock->AcquireWriterLock(-1);
              if ( CqListenerHelper<TKey, TResult>::m_ManagedVsUnManagedCqLstrDict->ContainsKey( newListeners[i]) ) {
                CqListenerHelper<TKey, TResult>::m_ManagedVsUnManagedCqLstrDict[newListeners[i]] = cptr;
              }
              else {
                CqListenerHelper<TKey, TResult>::m_ManagedVsUnManagedCqLstrDict->Add(newListeners[i], cptr);
              }
            } finally {
                CqListenerHelper<TKey, TResult>::g_readerWriterLock->ReleaseWriterLock();
            }
            ((native::ManagedCqListenerGeneric*)cptr->get())->setptr(cqlg);
          }
        }

        try
        {
          m_nativeptr->get()->initCqListeners( vrr );
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }
      }

      generic<class TKey, class TResult>
      Client::CqAttributes<TKey, TResult>^ CqAttributesFactory<TKey, TResult>::Create( )
      {
        try
        {
          return Client::CqAttributes<TKey, TResult>::Create(m_nativeptr->get()->create());
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }
      }
    }  // namespace Client
  }  // namespace Geode
}  // namespace Apache
