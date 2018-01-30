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


#include "impl/SafeConvert.hpp"
#include "impl/ManagedTransactionListener.hpp"
#include "impl/ManagedTransactionWriter.hpp"
#include "CacheTransactionManager.hpp"
#include "TimeUtils.hpp"

namespace Apache
{
  namespace Geode
  {
    namespace Client
    {

      using namespace System;

      void CacheTransactionManager::Begin( )
      {
        _GF_MG_EXCEPTION_TRY2

          m_nativeptr->begin( );

        _GF_MG_EXCEPTION_CATCH_ALL2
      }

      void CacheTransactionManager::Prepare( )
      {
        _GF_MG_EXCEPTION_TRY2

          m_nativeptr->prepare( );

        _GF_MG_EXCEPTION_CATCH_ALL2
      }

      void CacheTransactionManager::Commit( )
      {
        _GF_MG_EXCEPTION_TRY2

            m_nativeptr->commit( );

        _GF_MG_EXCEPTION_CATCH_ALL2
      }

      void CacheTransactionManager::Rollback( )
      {
        _GF_MG_EXCEPTION_TRY2

            m_nativeptr->rollback( );

        _GF_MG_EXCEPTION_CATCH_ALL2
      }

      bool CacheTransactionManager::Exists( )
      {
        _GF_MG_EXCEPTION_TRY2

        return m_nativeptr->exists( );

        _GF_MG_EXCEPTION_CATCH_ALL2
      }

      Apache::Geode::Client::TransactionId^ CacheTransactionManager::Suspend( )
      {
        _GF_MG_EXCEPTION_TRY2
       
          return Apache::Geode::Client::TransactionId::Create(&m_nativeptr->suspend());
       
        _GF_MG_EXCEPTION_CATCH_ALL2
      }
			Apache::Geode::Client::TransactionId^ CacheTransactionManager::TransactionId::get( )
      {
        _GF_MG_EXCEPTION_TRY2

          return Apache::Geode::Client::TransactionId::Create(&m_nativeptr->getTransactionId());

        _GF_MG_EXCEPTION_CATCH_ALL2
      }
      void CacheTransactionManager::Resume(Apache::Geode::Client::TransactionId^ transactionId)
      {
        _GF_MG_EXCEPTION_TRY2

          return m_nativeptr->resume(transactionId->GetNative());

        _GF_MG_EXCEPTION_CATCH_ALL2
      }
      bool CacheTransactionManager::IsSuspended(Apache::Geode::Client::TransactionId^ transactionId)
      {
        _GF_MG_EXCEPTION_TRY2

          return m_nativeptr->isSuspended(transactionId->GetNative());

        _GF_MG_EXCEPTION_CATCH_ALL2
      }
      bool CacheTransactionManager::TryResume(Apache::Geode::Client::TransactionId^ transactionId)
      {
        _GF_MG_EXCEPTION_TRY2

          return m_nativeptr->tryResume(transactionId->GetNative());

        _GF_MG_EXCEPTION_CATCH_ALL2
      }
      bool CacheTransactionManager::TryResume(Apache::Geode::Client::TransactionId^ transactionId, TimeSpan waitTime)
      {
        _GF_MG_EXCEPTION_TRY2

          return m_nativeptr->tryResume(transactionId->GetNative(), TimeUtils::TimeSpanToDurationCeil<std::chrono::milliseconds>(waitTime));

        _GF_MG_EXCEPTION_CATCH_ALL2
      }
      bool CacheTransactionManager::Exists(Apache::Geode::Client::TransactionId^ transactionId)
      {
        _GF_MG_EXCEPTION_TRY2

          return m_nativeptr->exists(transactionId->GetNative());

        _GF_MG_EXCEPTION_CATCH_ALL2
      }

#ifdef CSTX_COMMENTED
      generic<class TKey, class TValue>
      ITransactionWriter<TKey, TValue>^ CacheTransactionManager::GetWriter( )
      {
        _GF_MG_EXCEPTION_TRY2

          // Conver the unmanaged object to  managed generic object 
          std::shared_ptr<apache::geode::client::TransactionWriter>& writerPtr( m_nativeptr->getWriter( ) );
          apache::geode::client::ManagedTransactionWriterGeneric* twg =
          dynamic_cast<apache::geode::client::ManagedTransactionWriterGeneric*>( writerPtr.get() );

          if (twg != nullptr)
          {
            return (ITransactionWriter<TKey, TValue>^)twg->userptr( );
          }
        
        _GF_MG_EXCEPTION_CATCH_ALL2
        
        return nullptr;
      }
      
      generic<class TKey, class TValue>
      void CacheTransactionManager::SetWriter(ITransactionWriter<TKey, TValue>^ transactionWriter)
      {
        _GF_MG_EXCEPTION_TRY2
          // Create a unmanaged object using the ManagedTransactionWriterGeneric.
          // Set the generic object inside the TransactionWriterGeneric that is a non generic object
          std::shared_ptr<apache::geode::client::TransactionWriter> writerPtr;
          if ( transactionWriter != nullptr ) 
          {
            TransactionWriterGeneric<TKey, TValue>^ twg = gcnew TransactionWriterGeneric<TKey, TValue> ();
            twg->SetTransactionWriter(transactionWriter);
            writerPtr = new apache::geode::client::ManagedTransactionWriterGeneric( transactionWriter );
            ((apache::geode::client::ManagedTransactionWriterGeneric*)writerPtr.get())->setptr(twg);
          }
          m_nativeptr->setWriter( writerPtr );
          
        _GF_MG_EXCEPTION_CATCH_ALL2
      }

      generic<class TKey, class TValue>
      void CacheTransactionManager::AddListener(ITransactionListener<TKey, TValue>^ transactionListener)
      {
        _GF_MG_EXCEPTION_TRY2
          // Create a unmanaged object using the ManagedTransactionListenerGeneric.
          // Set the generic object inside the TransactionListenerGeneric that is a non generic object
          std::shared_ptr<apache::geode::client::TransactionListener> listenerPtr;
          if ( transactionListener != nullptr ) 
          {
            TransactionListenerGeneric<TKey, TValue>^ twg = gcnew TransactionListenerGeneric<TKey, TValue> ();
            twg->SetTransactionListener(transactionListener);
            listenerPtr = new apache::geode::client::ManagedTransactionListenerGeneric( transactionListener );
            ((apache::geode::client::ManagedTransactionListenerGeneric*)listenerPtr.get())->setptr(twg);
          }
          m_nativeptr->addListener( listenerPtr );
          
        _GF_MG_EXCEPTION_CATCH_ALL2
      }
        
      generic<class TKey, class TValue>
      void CacheTransactionManager::RemoveListener(ITransactionListener<TKey, TValue>^ transactionListener)
      {
        _GF_MG_EXCEPTION_TRY2
          // Create an unmanaged non generic object using the managed generic object
          // use this to call the remove listener
          std::shared_ptr<apache::geode::client::TransactionListener> listenerPtr;
          if ( transactionListener != nullptr ) 
          {
            TransactionListenerGeneric<TKey, TValue>^ twg = gcnew TransactionListenerGeneric<TKey, TValue> ();
            twg->SetTransactionListener(transactionListener);
            listenerPtr = new apache::geode::client::ManagedTransactionListenerGeneric( transactionListener );
            ((apache::geode::client::ManagedTransactionListenerGeneric*)listenerPtr.get())->setptr(twg);
          }
          m_nativeptr->removeListener( listenerPtr );

        _GF_MG_EXCEPTION_CATCH_ALL2
      }
#endif
    }  // namespace Client
  }  // namespace Geode
}  // namespace Apache

