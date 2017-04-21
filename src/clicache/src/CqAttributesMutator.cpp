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

#include <memory>

#include "begin_native.hpp"
#include <geode/QueryService.hpp>
#include "end_native.hpp"

#include "CqAttributesMutator.hpp"
#include "impl/ManagedCqListener.hpp"
#include "impl/ManagedCqStatusListener.hpp"

using namespace System;


namespace Apache
{
  namespace Geode
  {
    namespace Client
    {
      namespace native = apache::geode::client;

      generic<class TKey, class TResult>
      void CqAttributesMutator<TKey, TResult>::AddCqListener( Client::ICqListener<TKey, TResult>^ cqListener )
      {
        native::CqListenerPtr listenerptr;
        if ( cqListener != nullptr ) {
          auto cqStatusListener = dynamic_cast<ICqStatusListener<TKey, TResult>^>(cqListener);
          if (cqStatusListener != nullptr) {
            auto sLstr = gcnew CqStatusListenerGeneric<TKey, TResult>();
            sLstr->AddCqListener(cqListener);
            listenerptr = std::shared_ptr<native::ManagedCqStatusListenerGeneric>(new native::ManagedCqStatusListenerGeneric(cqListener));
            try {
              CqListenerHelper<TKey, TResult>::g_readerWriterLock->AcquireWriterLock(-1);
              if ( CqListenerHelper<TKey, TResult>::m_ManagedVsUnManagedCqLstrDict->ContainsKey(cqListener) ) {
                CqListenerHelper<TKey, TResult>::m_ManagedVsUnManagedCqLstrDict[cqListener] = (IntPtr)listenerptr.get();
              }
              else {
                CqListenerHelper<TKey, TResult>::m_ManagedVsUnManagedCqLstrDict->Add(cqListener, (IntPtr)listenerptr.get());
              }
            }
            finally {
              CqListenerHelper<TKey, TResult>::g_readerWriterLock->ReleaseWriterLock();
            }
            ((native::ManagedCqStatusListenerGeneric*)listenerptr.get())->setptr(sLstr);
          }
          else {
            //TODO::split
            auto cqlg = gcnew CqListenerGeneric<TKey, TResult>();
            cqlg->AddCqListener(cqListener);
            listenerptr = std::shared_ptr<native::ManagedCqListenerGeneric>(new native::ManagedCqListenerGeneric(cqListener));
            try {
              CqListenerHelper<TKey, TResult>::g_readerWriterLock->AcquireWriterLock(-1);
              if ( CqListenerHelper<TKey, TResult>::m_ManagedVsUnManagedCqLstrDict->ContainsKey(cqListener) ) {
                CqListenerHelper<TKey, TResult>::m_ManagedVsUnManagedCqLstrDict[cqListener] = (IntPtr)listenerptr.get(); 
              }
              else {
                CqListenerHelper<TKey, TResult>::m_ManagedVsUnManagedCqLstrDict->Add(cqListener, (IntPtr)listenerptr.get());
              }
            } finally {
                CqListenerHelper<TKey, TResult>::g_readerWriterLock->ReleaseWriterLock();
            }
            ((native::ManagedCqListenerGeneric*)listenerptr.get())->setptr(cqlg);            
          }
        }
        try
        {
          m_nativeptr->get()->addCqListener( listenerptr );
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }
      }

      generic<class TKey, class TResult>
      void CqAttributesMutator<TKey, TResult>::RemoveCqListener( Client::ICqListener<TKey, TResult>^ cqListener )
      {
        auto lister = dynamic_cast<Client::ICqStatusListener<TKey, TResult>^>(cqListener);
        if (lister != nullptr) {
          auto cqlg = gcnew CqStatusListenerGeneric<TKey, TResult>();
          cqlg->AddCqListener(cqListener);
          native::CqStatusListenerPtr lptr = std::shared_ptr<native::ManagedCqStatusListenerGeneric>(
            new native::ManagedCqStatusListenerGeneric(lister));
          ((native::ManagedCqStatusListenerGeneric*)lptr.get())->setptr(cqlg);
          try {
            IntPtr value;
            CqListenerHelper<TKey, TResult>::g_readerWriterLock->AcquireWriterLock(-1);
            if ( CqListenerHelper<TKey, TResult>::m_ManagedVsUnManagedCqLstrDict->TryGetValue(cqListener, value) ) {
              // TODO shared_ptr this will break, need to keep shared_ptr
              native::CqStatusListenerPtr lptr((native::CqStatusListener*)value.ToPointer());
              try
              {
                m_nativeptr->get()->removeCqListener(lptr);
              }
              finally
              {
                GC::KeepAlive(m_nativeptr);
              }
            }
          } finally {
              CqListenerHelper<TKey, TResult>::g_readerWriterLock->ReleaseWriterLock();
          }
        }
        else {
          auto cqlg = gcnew CqListenerGeneric<TKey, TResult>();
          cqlg->AddCqListener(cqListener);
          native::CqListenerPtr lptr = std::shared_ptr<native::ManagedCqListenerGeneric>(
            new native::ManagedCqListenerGeneric(cqListener));
          ((native::ManagedCqListenerGeneric*)lptr.get())->setptr(cqlg);
          try {
            IntPtr value;
            CqListenerHelper<TKey, TResult>::g_readerWriterLock->AcquireWriterLock(-1);
            if ( CqListenerHelper<TKey, TResult>::m_ManagedVsUnManagedCqLstrDict->TryGetValue(cqListener, value) ) {
              // TODO shared_ptr this will break, need to keep shared_ptr
              native::CqListenerPtr lptr((native::CqListener*)value.ToPointer());
              try
              {
                m_nativeptr->get()->removeCqListener(lptr);
              }
              finally
              {
                GC::KeepAlive(m_nativeptr);
              }
            } 
          } finally {
              CqListenerHelper<TKey, TResult>::g_readerWriterLock->ReleaseWriterLock();            
          }
        }
      }

      generic<class TKey, class TResult>
      void CqAttributesMutator<TKey, TResult>::SetCqListeners(array<Client::ICqListener<TKey, TResult>^>^ newListeners)
      {
        native::CqAttributes::listener_container_type vrr;
        for( int i = 0; i < newListeners->Length; i++ )
        {
          auto lister = dynamic_cast<Client::ICqStatusListener<TKey, TResult>^>(newListeners[i]);
          if (lister != nullptr) {
            auto cptr = std::shared_ptr<native::ManagedCqStatusListenerGeneric>(new native::ManagedCqStatusListenerGeneric(lister));
            vrr.push_back(cptr);
            auto cqlg = gcnew CqStatusListenerGeneric<TKey, TResult>();
            cqlg->AddCqListener(newListeners[i]);
            try {
              CqListenerHelper<TKey, TResult>::g_readerWriterLock->AcquireWriterLock(-1);
              if ( CqListenerHelper<TKey, TResult>::m_ManagedVsUnManagedCqLstrDict->ContainsKey( newListeners[i]) ) {
                CqListenerHelper<TKey, TResult>::m_ManagedVsUnManagedCqLstrDict[newListeners[i]] = (IntPtr)cptr.get();
              }
              else {
                CqListenerHelper<TKey, TResult>::m_ManagedVsUnManagedCqLstrDict->Add(newListeners[i], (IntPtr)cptr.get());
              }
            } finally {
                CqListenerHelper<TKey, TResult>::g_readerWriterLock->ReleaseWriterLock();
            }
            ((native::ManagedCqStatusListenerGeneric*)vrr[i].get())->setptr(cqlg);
          }
          else {
            auto lister = newListeners[i];
            auto cptr = std::shared_ptr<native::ManagedCqListenerGeneric>(new native::ManagedCqListenerGeneric(lister));
            vrr.push_back(cptr);
            auto cqlg = gcnew CqListenerGeneric<TKey, TResult>();
            cqlg->AddCqListener(newListeners[i]);
            try {
              CqListenerHelper<TKey, TResult>::g_readerWriterLock->AcquireWriterLock(-1);
              if ( CqListenerHelper<TKey, TResult>::m_ManagedVsUnManagedCqLstrDict->ContainsKey( newListeners[i]) ) {
                CqListenerHelper<TKey, TResult>::m_ManagedVsUnManagedCqLstrDict[newListeners[i]] = (IntPtr)cptr.get();
              }
              else {
                CqListenerHelper<TKey, TResult>::m_ManagedVsUnManagedCqLstrDict->Add(newListeners[i], (IntPtr)cptr.get());
              }
            } finally {
                CqListenerHelper<TKey, TResult>::g_readerWriterLock->ReleaseWriterLock();
            }
            ((native::ManagedCqListenerGeneric*)vrr[i].get())->setptr(cqlg);
          }
        }

        try
        {
          m_nativeptr->get()->setCqListeners( vrr );
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }
      }
    }  // namespace Client
  }  // namespace Geode
}  // namespace Apache
