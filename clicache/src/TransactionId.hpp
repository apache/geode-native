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
#include <geode/TransactionId.hpp>
#include "end_native.hpp"



using namespace System;

namespace Apache
{
  namespace Geode
  {
    namespace Client
    {
        namespace native = apache::geode::client;

        /// <summary>
        /// This class encapsulates Id of a transaction.
        /// </summary>
        public ref class TransactionId sealed
        {
        internal:

          inline static TransactionId^ Create(native::TransactionId* nativeptr )
          {
          return __nullptr == nativeptr ? nullptr :
            gcnew TransactionId( nativeptr );
          }

          native::TransactionId& GetNative()
          {
            return *m_nativeptr;
          }

        private:

          /// <summary>
          /// Private constructor to wrap a native object pointer
          /// </summary>
          /// <param name="nativeptr">The native object pointer</param>
          inline TransactionId(native::TransactionId* nativeptr)
            : m_nativeptr(nativeptr)
          {
          }

          native::TransactionId* m_nativeptr;
        };
    }  // namespace Client
  }  // namespace Geode
}  // namespace Apache
 //namespace 
