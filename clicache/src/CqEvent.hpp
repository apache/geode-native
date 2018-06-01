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


#include "native_shared_ptr.hpp"
#include "geode_defs.hpp"
#include "begin_native.hpp"
#include <geode/CqEvent.hpp>
#include "end_native.hpp"

#include "CqQuery.hpp"
#include "CqOperation.hpp"

#include "ICqEvent.hpp"
#include "ICacheableKey.hpp"

using namespace System;

namespace Apache
{
  namespace Geode
  {
    namespace Client
    {
      namespace native = apache::geode::client;

			interface class ISerializable;
      
      /// <summary>
      /// This class encapsulates events that occur for cq.
      /// </summary>
      generic<class TKey, class TResult>
      public ref class CqEvent sealed
      {
      public:


        /// <summary>
        /// Return the cqquery this event occurred in.
        /// </summary>
	      CqQuery<TKey, TResult>^ getCq();

        /// <summary>
        /// Get the operation on the base operation that triggered this event.
        /// </summary>
       CqOperation getBaseOperation();

        /// <summary>
        /// Get the operation on the query operation that triggered this event.
        /// </summary>
       CqOperation getQueryOperation();

        /// <summary>
        /// Get the key relating to the event.
        /// In case of REGION_CLEAR and REGION_INVALIDATE operation, the key will be null.
        /// </summary>
        TKey /*Generic::ICacheableKey^*/ getKey( );

        /// <summary>
        /// Get the new value of the modification.
        /// If there is no new value returns null, this will happen during delete
        /// operation.
        /// </summary>
        /*Object^*/ TResult getNewValue( );

        array< Byte >^ getDeltaValue( );

      internal:

        /// <summary>
        /// Private constructor to wrap a native object pointer
        /// </summary>
        /// <param name="nativeptr">The native object pointer</param>
        inline CqEvent( const native::CqEvent* nativeptr )
          : m_nativeptr(nativeptr)
        {
        }

        const native::CqEvent* GetNative()
        {
          return m_nativeptr;
        }

      private:
        const native::CqEvent* m_nativeptr;
      };
    }  // namespace Client
  }  // namespace Geode
}  // namespace Apache

