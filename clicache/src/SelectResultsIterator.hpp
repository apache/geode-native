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

#include "ISelectResults.hpp"

using namespace System;

namespace Apache
{
  namespace Geode
  {
    namespace Client
    {
      interface class ISerializable;

      /// <summary>
      /// Iterator for a query result.
      /// </summary>
      generic<class TResult>
      public ref class SelectResultsIterator sealed
        : public System::Collections::Generic::IEnumerator<TResult>
      {
      public:

        /// <summary>
        /// Gets the element in the collection at the current
        /// position of the enumerator.
        /// </summary>
        /// <returns>
        /// The element in the collection at the current position
        /// of the enumerator.
        /// </returns>
        property TResult Current
        {
          virtual TResult get( );
        }

        property Object^ Current2
        {
          virtual Object^ get( ) = System::Collections::IEnumerator::Current::get;
        }

        /// <summary>
        /// Advances the enumerator to the next element of the collection.
        /// </summary>
        /// <returns>
        /// true if the enumerator was successfully advanced to the next
        /// element; false if the enumerator has passed the end of
        /// the collection.
        /// </returns>
        virtual bool MoveNext( );

        /// <summary>
        /// Sets the enumerator to its initial position, which is before
        /// the first element in the collection.
        /// </summary>
        virtual void Reset( );

        ~SelectResultsIterator() {};

      internal:

        /// <summary>
        /// Internal factory function to wrap a native object pointer inside
        /// this managed class with null pointer check.
        /// </summary>
        /// <param name="nativeptr">The native object pointer</param>
        /// <returns>
        /// The managed wrapper object; null if the native pointer is null.
        /// </returns>
        inline static SelectResultsIterator<TResult>^ Create(
          ISelectResults<TResult>^ results )
        {
          return results == nullptr ? nullptr :
            gcnew SelectResultsIterator<TResult>(results);
        }


      private:

        /// <summary>
        /// Private constructor to wrap a native object pointer
        /// </summary>
        /// <param name="nativeptr">The native object pointer</param>
        inline SelectResultsIterator(ISelectResults<TResult>^ results) : m_results(results)
        {
        }

        ISelectResults<TResult>^ m_results;
        int m_index = -1;
      };
    }  // namespace Client
  }  // namespace Geode
}  // namespace Apache

