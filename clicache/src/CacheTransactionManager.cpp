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
        try {
          m_nativeptr->begin( );
        }
        catch (const apache::geode::client::Exception& ex) {
          throw Apache::Geode::Client::GeodeException::Get(ex);
        }
        catch (System::AccessViolationException^ ex) {
          throw ex;
        }
      }

      void CacheTransactionManager::Prepare( )
      {
        try {
          m_nativeptr->prepare( );
        }
        catch (const apache::geode::client::Exception& ex) {
          throw Apache::Geode::Client::GeodeException::Get(ex);
        }
        catch (System::AccessViolationException^ ex) {
          throw ex;
        }
      }

      void CacheTransactionManager::Commit( )
      {
        try {
            m_nativeptr->commit( );
        }
        catch (const apache::geode::client::Exception& ex) {
          throw Apache::Geode::Client::GeodeException::Get(ex);
        }
        catch (System::AccessViolationException^ ex) {
          throw ex;
        }
      }

      void CacheTransactionManager::Rollback( )
      {
        try {
            m_nativeptr->rollback( );
        }
        catch (const apache::geode::client::Exception& ex) {
          throw Apache::Geode::Client::GeodeException::Get(ex);
        }
        catch (System::AccessViolationException^ ex) {
          throw ex;
        }
      }

      bool CacheTransactionManager::Exists( )
      {
        try {
          return m_nativeptr->exists( );
        }
        catch (const apache::geode::client::Exception& ex) {
          throw Apache::Geode::Client::GeodeException::Get(ex);
        }
        catch (System::AccessViolationException^ ex) {
          throw ex;
        }
      }

      Apache::Geode::Client::TransactionId^ CacheTransactionManager::Suspend( )
      {
        try {
          return Apache::Geode::Client::TransactionId::Create(&m_nativeptr->suspend());
        }
        catch (const apache::geode::client::Exception& ex) {
          throw Apache::Geode::Client::GeodeException::Get(ex);
        }
        catch (System::AccessViolationException^ ex) {
          throw ex;
        }
      }

			Apache::Geode::Client::TransactionId^ CacheTransactionManager::TransactionId::get( )
      {
        try {
          return Apache::Geode::Client::TransactionId::Create(&m_nativeptr->getTransactionId());
        }
        catch (const apache::geode::client::Exception& ex) {
          throw Apache::Geode::Client::GeodeException::Get(ex);
        }
        catch (System::AccessViolationException^ ex) {
          throw ex;
        }
      }

      void CacheTransactionManager::Resume(Apache::Geode::Client::TransactionId^ transactionId)
      {
        try {
          return m_nativeptr->resume(transactionId->GetNative());
        }
        catch (const apache::geode::client::Exception& ex) {
          throw Apache::Geode::Client::GeodeException::Get(ex);
        }
        catch (System::AccessViolationException^ ex) {
          throw ex;
        }
      }

      bool CacheTransactionManager::IsSuspended(Apache::Geode::Client::TransactionId^ transactionId)
      {
        try {
          return m_nativeptr->isSuspended(transactionId->GetNative());
        }
        catch (const apache::geode::client::Exception& ex) {
          throw Apache::Geode::Client::GeodeException::Get(ex);
        }
        catch (System::AccessViolationException^ ex) {
          throw ex;
        }
      }

      bool CacheTransactionManager::TryResume(Apache::Geode::Client::TransactionId^ transactionId)
      {
        try {
          return m_nativeptr->tryResume(transactionId->GetNative());
        }
        catch (const apache::geode::client::Exception& ex) {
          throw Apache::Geode::Client::GeodeException::Get(ex);
        }
        catch (System::AccessViolationException^ ex) {
          throw ex;
        }
      }

      bool CacheTransactionManager::TryResume(Apache::Geode::Client::TransactionId^ transactionId, TimeSpan waitTime)
      {
        try {
          return m_nativeptr->tryResume(transactionId->GetNative(), TimeUtils::TimeSpanToDurationCeil<std::chrono::milliseconds>(waitTime));
        }
        catch (const apache::geode::client::Exception& ex) {
          throw Apache::Geode::Client::GeodeException::Get(ex);
        }
        catch (System::AccessViolationException^ ex) {
          throw ex;
        }
      }

      bool CacheTransactionManager::Exists(Apache::Geode::Client::TransactionId^ transactionId)
      {
        try {
          return m_nativeptr->exists(transactionId->GetNative());
        }
        catch (const apache::geode::client::Exception& ex) {
          throw Apache::Geode::Client::GeodeException::Get(ex);
        }
        catch (System::AccessViolationException^ ex) {
          throw ex;
        }
      }

#ifdef CSTX_COMMENTED
      generic<class TKey, class TValue>
      ITransactionWriter<TKey, TValue>^ CacheTransactionManager::GetWriter( )
      {
        try {

          // Conver the unmanaged object to  managed generic object 
          std::shared_ptr<apache::geode::client::TransactionWriter>& writerPtr( m_nativeptr->getWriter( ) );
          apache::geode::client::ManagedTransactionWriterGeneric* twg =
          dynamic_cast<apache::geode::client::ManagedTransactionWriterGeneric*>( writerPtr.get() );

          if (twg != nullptr)
          {
            return (ITransactionWriter<TKey, TValue>^)twg->userptr( );
          }
        
        }
        catch (const apache::geode::client::Exception& ex) {
          throw Apache::Geode::Client::GeodeException::Get(ex);
        }
        catch (System::AccessViolationException^ ex) {
          throw ex;
        }
        
        return nullptr;
      }
      
      generic<class TKey, class TValue>
      void CacheTransactionManager::SetWriter(ITransactionWriter<TKey, TValue>^ transactionWriter)
      {
        try {
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
          
        }
        catch (const apache::geode::client::Exception& ex) {
          throw Apache::Geode::Client::GeodeException::Get(ex);
        }
        catch (System::AccessViolationException^ ex) {
          throw ex;
        }
      }

      generic<class TKey, class TValue>
      void CacheTransactionManager::AddListener(ITransactionListener<TKey, TValue>^ transactionListener)
      {
        try {
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
          
        }
        catch (const apache::geode::client::Exception& ex) {
          throw Apache::Geode::Client::GeodeException::Get(ex);
        }
        catch (System::AccessViolationException^ ex) {
          throw ex;
        }
      }
        
      generic<class TKey, class TValue>
      void CacheTransactionManager::RemoveListener(ITransactionListener<TKey, TValue>^ transactionListener)
      {
        try {
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

        }
        catch (const apache::geode::client::Exception& ex) {
          throw Apache::Geode::Client::GeodeException::Get(ex);
        }
        catch (System::AccessViolationException^ ex) {
          throw ex;
        }
      }
#endif
    }  // namespace Client
  }  // namespace Geode
}  // namespace Apache

