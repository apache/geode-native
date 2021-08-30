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
#include <geode/StructSet.hpp>
#include "end_native.hpp"

#include "native_shared_ptr.hpp"
#include "ICqResults.hpp"


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
      /// Encapsulates a query struct set.
      /// </summary>
      generic<class TResult>
      public ref class StructSet sealed
        : public ICqResults<TResult>
      {
      public:

        /// <summary>
        /// The size of the <c>StructSet</c>.
        /// </summary>
        /// <returns>
        /// the number of items in the <c>StructSet</c>.
        /// </returns>
        virtual property size_t Size
        {
          virtual size_t get( );
        }

        /// <summary>
        /// Index operator to directly access an item in the <c>StructSet</c>.
        /// </summary>
        /// <exception cref="IllegalArgumentException">
        /// if the index is out of bounds.
        /// </exception>
        /// <returns>Item at the given index.</returns>
        virtual property TResult default[ size_t ]
        {
          virtual TResult get( size_t index );
        }

        /// <summary>
        /// Get the index number of the specified field name
        /// in the <c>StructSet</c>.
        /// </summary>
        /// <param name="fieldName">
        /// the field name for which the index is required.
        /// </param>
        /// <returns>the index number of the specified field name.</returns>
        /// <exception cref="IllegalArgumentException">
        /// if the field name is not found.
        /// </exception>
        int32_t GetFieldIndex( String^ fieldName );

        /// <summary>
        /// Get the field name of the <c>StructSet</c> from the
        /// specified index number.
        /// </summary>
        /// <param name="index">
        /// the index number of the field name to get.
        /// </param>
        /// <returns>
        /// the field name from the specified index number or null if not found.
        /// </returns>
        String^ GetFieldName( int32_t index );


        // Region: IEnumerable<ISerializable^> Members

        /// <summary>
        /// Returns an enumerator that iterates through the <c>StructSet</c>.
        /// </summary>
        /// <returns>
        /// A <c>System.Collections.Generic.IEnumerator</c> that
        /// can be used to iterate through the <c>StructSet</c>.
        /// </returns>
        virtual System::Collections::Generic::IEnumerator<TResult>^
          GetEnumerator( );

        // End Region: IEnumerable<ISerializable^> Members


      internal:

        /// <summary>
        /// Internal factory function to wrap a native object pointer inside
        /// this managed class with null pointer check.
        /// </summary>
        /// <param name="nativeptr">The native object pointer</param>
        /// <returns>
        /// The managed wrapper object; null if the native pointer is null.
        /// </returns>
        inline static StructSet<TResult>^ Create(std::shared_ptr<native::StructSet> nativeptr)
        {
          return __nullptr == nativeptr ? nullptr :
            gcnew StructSet<TResult>( nativeptr );
        }


      private:

        virtual System::Collections::IEnumerator^ GetIEnumerator( ) sealed
          = System::Collections::IEnumerable::GetEnumerator;

        /// <summary>
        /// Private constructor to wrap a native object pointer
        /// </summary>
        /// <param name="nativeptr">The native object pointer</param>
        inline StructSet(std::shared_ptr<native::StructSet> nativeptr)
        {
          m_nativeptr = gcnew native_shared_ptr<native::StructSet>(nativeptr);
        }

        native_shared_ptr<native::StructSet>^ m_nativeptr; 
      };

    }  // namespace Client
  }  // namespace Geode
}  // namespace Apache

