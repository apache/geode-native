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

#pragma once


#include "geode_defs.hpp"
#include "begin_native.hpp"
#include <geode/CacheTransactionManager.hpp>
#include "end_native.hpp"
#include "native_shared_ptr.hpp"
#include "TransactionId.hpp"

using namespace System;
namespace Apache
{
  namespace Geode
  {
    namespace Client
    {
      namespace native = apache::geode::client;
      /// <summary>
      /// CacheTransactionManager encapsulates the transactions for a cache
      /// </summary>
      public ref class CacheTransactionManager sealed
      {
      public:
        /// <summary>
        /// Creates a new transaction and associates it with the current thread.
        /// </summary>
        /// <exception cref="IllegalStateException">
        /// Throws exception if the thread is already associated with a transaction
        /// </exception>
        void Begin();

        /// <summary>
        /// Prepare the first message of two-phase-commit transaction associated
        /// with the current thread.
        /// </summary>
        /// <exception cref="IllegalStateException">
        /// if the thread is not associated with a transaction
        /// </exception>
        /// <exception cref="CommitConflictException">
        /// if the commit operation fails due to a write conflict.
        /// </exception>
        void Prepare();

        /// <summary>
        /// Commit the transaction associated with the current thread. If
        /// the commit operation fails due to a conflict it will destroy
        /// the transaction state and throw a <c>CommitConflictException</c>. 
        /// If the commit operation succeeds,it returns after the transaction 
        /// state has been merged with committed state.  When this method 
        /// completes, the thread is no longer associated with a transaction.
        /// </summary>
        /// <exception cref="IllegalStateException">
        /// if the thread is not associated with a transaction
        /// </exception>
        /// <exception cref="CommitConflictException">
        /// if the commit operation fails due to a write conflict.
        /// </exception>
        void Commit();
        
        /// <summary>
        /// Roll back the transaction associated with the current thread. When
        /// this method completes, the thread is no longer associated with a
        /// transaction and the transaction context is destroyed.
        /// </summary>
        /// <exception cref="IllegalStateException">
        /// if the thread is not associated with a transaction
        /// </exception>  
        void Rollback();
        
        /// <summary>
        /// Reports the existence of a Transaction for this thread
        /// </summary>
        /// <returns>true if a transaction exists, false otherwise</returns>
        bool Exists();

        /// <summary>
        /// Suspends the transaction on the current thread. All subsequent operations
        /// performed by this thread will be non-transactional. The suspended
        /// transaction can be resumed by calling <see cref="TransactionId"/>
        /// <para>
        /// Since 3.6.2
        /// </para>
        /// </summary>
        /// <returns>the transaction identifier of the suspended transaction or null if
        /// the thread was not associated with a transaction</returns>
        Apache::Geode::Client::TransactionId^ Suspend();

        /// <summary>
        /// On the current thread, resumes a transaction that was previously suspended
        /// using <see cref="suspend"/>
        /// <para>
        /// Since 3.6.2
        /// </para>
        /// </summary>
        /// <param name="transactionId">the transaction to resume</param>
        /// <exception cref="IllegalStateException">if the thread is associated with a transaction or if
        /// would return false for the given transactionId</exception>
        /// <see cref="TransactionId"/> 
        void Resume(Apache::Geode::Client::TransactionId^ transactionId);

        /// <summary>
        /// This method can be used to determine if a transaction with the given
        /// transaction identifier is currently suspended locally. This method does not
        ///  check other members for transaction status.
        /// <para>
        /// Since 3.6.2
        /// </para>
        /// </summary>
        /// <param name="transactionId"></param>
        /// <returns>true if the transaction is in suspended state, false otherwise</returns>
        /// <see cref="TransactionId"/>
        bool IsSuspended(Apache::Geode::Client::TransactionId^ transactionId);


        /// <summary>
        /// On the current thread, resumes a transaction that was previously suspended
        /// using <see cref="suspend"/>.
        /// This method is equivalent to
        /// <code>
        /// if (isSuspended(txId)) {
        ///   resume(txId);
        /// }
        /// </code>
        /// except that this action is performed atomically
        /// <para>
        /// Since 3.6.2
        /// </para>
        /// </summary>
        /// <param name="transactionId">the transaction to resume</param>
        /// <returns>true if the transaction was resumed, false otherwise</returns>
        bool TryResume(Apache::Geode::Client::TransactionId^ transactionId);


        /// <summary>
        /// On the current thread, resumes a transaction that was previously suspended
        /// using <see cref="suspend"/>, or waits for the specified timeout interval if
        /// the transaction has not been suspended. This method will return if:
        /// <para>
        /// Another thread suspends the transaction
        /// </para>
        /// <para>
        /// Another thread calls commit/rollback on the transaction
        /// </para>
        /// <para>
        /// This thread has waited for the specified timeout
        /// </para>
        /// This method returns immediately if <see cref="TransactionId"/> returns false.
        /// <para>
        /// Since 3.6.2
        /// </para>
        /// </summary>
        /// <param name="transactionId">the transaction to resume</param>
        /// <param name="waitTime">the maximum time to wait </param>
        /// <returns>true if the transaction was resumed, false otherwise</returns>
        bool TryResume(Apache::Geode::Client::TransactionId^ transactionId, TimeSpan waitTime);



        /// <summary>
        /// Reports the existence of a transaction for the given transactionId. This
        /// method can be used to determine if a transaction with the given transaction
        /// identifier is currently in progress locally.
        /// <para>
        /// Since 3.6.2
        /// </para>
        /// </summary>
        /// <param name="transactionId">the given transaction identifier</param>
        /// <returns>true if the transaction is in progress, false otherwise.</returns>
        /// <see cref="isSuspended"/> 
        bool Exists(Apache::Geode::Client::TransactionId^ transactionId);


        /// <summary>
        /// Returns the transaction identifier for the current thread
        /// <para>
        /// Since 3.6.2
        /// </para>
        /// </summary>
        /// <returns>the transaction identifier or null if no transaction exists</returns>
        property Apache::Geode::Client::TransactionId^ TransactionId
        {
        //TODO::split
        Apache::Geode::Client::TransactionId^ get( );
        }        

      internal:

        inline static CacheTransactionManager^ Create(native::CacheTransactionManager* nativeptr )
        {
          return ( nativeptr != nullptr ?
            gcnew CacheTransactionManager( nativeptr ) : nullptr );
        }


      private:

        /// <summary>
        /// Private constructor to wrap a native object pointer
        /// </summary>
        /// <param name="nativeptr">The native object pointer</param>
        inline CacheTransactionManager(native::CacheTransactionManager* nativeptr )
          : m_nativeptr(nativeptr)
        {
        }

        native::CacheTransactionManager* m_nativeptr;
      };
    }  // namespace Client
  }  // namespace Geode
}  // namespace Apache
