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
#include <geode/CqAttributesMutator.hpp>
#include "end_native.hpp"

#include "native_shared_ptr.hpp"
#include "native_conditional_unique_ptr.hpp"

using namespace System;
using namespace System::Collections::Generic;
using namespace System::Threading;

namespace Apache
{
  namespace Geode
  {
    namespace Client
    {
      namespace native = apache::geode::client;

      generic<class TKey, class TResult>
	  interface class ICqListener;

      generic<class TKey, class TResult>
      private ref class CqListenerHelper sealed{
        public:
        static Dictionary<Client::ICqListener<TKey, TResult>^, native_shared_ptr<native::CqListener>^>^
          m_ManagedVsUnManagedCqLstrDict = gcnew 
          Dictionary<Client::ICqListener<TKey, TResult>^, native_shared_ptr<native::CqListener>^>();

        static ReaderWriterLock^ g_readerWriterLock = gcnew ReaderWriterLock();
      };

      /// <summary>
      /// Supports modification of certain cq attributes after the cq
      /// has been created.
      /// </summary>
      generic<class TKey, class TResult>
      public ref class CqAttributesMutator sealed
      {
      public:

        /// <summary>
        /// Adds the CqListener for the cq.
        /// </summary>
        /// <param name="cqListener">
        /// user-defined cq listener, or null for no cache listener
        /// </param>
        void AddCqListener( Client::ICqListener<TKey, TResult>^ cqListener );

        /// <summary>
        /// Remove a CqListener for the cq.
        /// </summary>
        
        void RemoveCqListener(Client::ICqListener<TKey, TResult>^ aListener);


        /// <summary>
	      /// Initialize with an array of listeners
        /// </summary>
        
        void SetCqListeners(array<Client::ICqListener<TKey, TResult>^>^ newListeners);


      internal:

        /// <summary>
        /// Internal factory function to wrap a native object pointer inside
        /// this managed class.
        /// </summary>
        /// <param name="nativeptr">The native object pointer</param>
        /// <returns>
        /// The managed wrapper object
        /// </returns>
        inline static Client::CqAttributesMutator<TKey, TResult>^ Create(native::CqAttributesMutator* nativeptr)
        {
          auto instance = gcnew CqAttributesMutator(nativeptr);
          return instance;
        }

      private:

        /// <summary>
        /// Private constructor to wrap a native object pointer
        /// </summary>
        /// <param name="nativeptr">The native object pointer</param>
        inline CqAttributesMutator<TKey, TResult>(native::CqAttributesMutator* nativeptr)
        {
            m_nativeptr = gcnew native_conditional_unique_ptr<native::CqAttributesMutator>(nativeptr);
        }

        native_conditional_unique_ptr<native::CqAttributesMutator>^ m_nativeptr;
      };
    }  // namespace Client
  }  // namespace Geode
}  // namespace Apache

