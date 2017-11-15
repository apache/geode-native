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
#include <geode/CqAttributes.hpp>
#include "end_native.hpp"

#include "native_shared_ptr.hpp"


using namespace System;

namespace Apache
{
  namespace Geode
  {
    namespace Client
    {
      namespace native = apache::geode::client;

      generic<class TKey, class TResult>
      interface class ICqListener;

      /// <summary>
      /// Defines attributes for configuring a cq.
      /// </summary>
      generic<class TKey, class TResult>
      public ref class CqAttributes sealed
      {
      public:

        /// <summary>
        /// get all listeners in this attributes
        /// </summary>
        virtual array<Client::ICqListener<TKey, TResult>^>^ getCqListeners();

      internal:

        /// <summary>
        /// Internal factory function to wrap a native object pointer inside
        /// this managed class with null pointer check.
        /// </summary>
        /// <param name="nativeptr">The native object pointer</param>
        /// <returns>
        /// The managed wrapper object; null if the native pointer is null.
        /// </returns>
        inline static CqAttributes<TKey, TResult>^ Create( std::shared_ptr<native::CqAttributes> nativeptr )
        {
          return __nullptr == nativeptr ? nullptr :
            gcnew CqAttributes( nativeptr );
        }

        std::shared_ptr<native::CqAttributes> GetNative()
        {
          return m_nativeptr->get_shared_ptr();
        }

      private:

        /// <summary>
        /// Private constructor to wrap a native object pointer
        /// </summary>
        /// <param name="nativeptr">The native object pointer</param>
        inline CqAttributes( std::shared_ptr<native::CqAttributes> nativeptr )
        {
          m_nativeptr = gcnew native_shared_ptr<native::CqAttributes>(nativeptr);
        }

        native_shared_ptr<native::CqAttributes>^ m_nativeptr;
      };
    }  // namespace Client
  }  // namespace Geode
}  // namespace Apache

