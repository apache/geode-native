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
#include <geode/ResultSet.hpp>
#include "end_native.hpp"

#include "native_shared_ptr.hpp"
#include "ISelectResults.hpp"


using namespace System;

namespace Apache
{
  namespace Geode
  {
    namespace Client
    {
      namespace native = apache::geode::client;

      interface class ISerializable;

      generic<class TResult>
      ref class SelectResultsIterator;
      /// <summary>
      /// Encapsulates a query result set.
      /// It specifies the interface for the resultset obtained from the
      /// Geode cache server
      /// </summary>
      generic<class TResult>
      public ref class ResultSet sealed
        : public ISelectResults<TResult>
      {
      public:

        /// <summary>
        /// The size of the <c>ResultSet</c>.
        /// </summary>
        virtual property size_t Size
        {
          virtual size_t get();
        }

        /// <summary>
        /// Get an object at the given index.
        /// </summary>
        virtual property TResult default[ size_t ]
        {
          virtual TResult get(size_t index);
        }

        /// <summary>
        /// Returns an enumerator that iterates through the collection.
        /// </summary>
        /// <returns>
        /// A <c>System.Collections.Generic.IEnumerator</c> that
        /// can be used to iterate through the <c>ResultSet</c>.
        /// </returns>
        virtual System::Collections::Generic::IEnumerator<TResult>^ GetEnumerator();


        virtual System::Collections::IEnumerator^ GetIEnumerator()
          = System::Collections::IEnumerable::GetEnumerator;

      internal:

        /// <summary>
        /// Internal factory function to wrap a native object pointer inside
        /// this managed class with null pointer check.
        /// </summary>
        /// <param name="nativeptr">The native object pointer</param>
        /// <returns>
        /// The managed wrapper object; null if the native pointer is null.
        /// </returns>
        inline static ResultSet<TResult>^ Create(std::shared_ptr<native::ResultSet> nativeptr)
        {
          return __nullptr == nativeptr ? nullptr :
            gcnew ResultSet<TResult>( nativeptr );
        }


      private:

        /// <summary>
        /// Private constructor to wrap a native object pointer
        /// </summary>
        /// <param name="nativeptr">The native object pointer</param>
        inline ResultSet(std::shared_ptr<native::ResultSet> nativeptr)
        {
          m_nativeptr = gcnew native_shared_ptr<native::ResultSet>(nativeptr);
        }

        native_shared_ptr<native::ResultSet>^ m_nativeptr;
      };

    }  // namespace Client
  }  // namespace Geode
}  // namespace Apache

