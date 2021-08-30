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
#include "Serializable.hpp"
#include "begin_native.hpp"
#include <geode/Struct.hpp>
#include "end_native.hpp"


using namespace System;

namespace Apache
{
  namespace Geode
  {
    namespace Client
    {

      generic<class TResult>
      ref class StructSet;

      /// <summary>
      /// Encapsulates a row of query struct set.
      /// </summary>
      /// <remarks>
      /// A Struct has a StructSet as its parent. It contains the field values
      /// returned after executing a Query obtained from a QueryService which in turn
      /// is obtained from a Cache.
      /// </remarks>
      public ref class Struct sealed
        : public Apache::Geode::Client::Serializable
      {
      public:

        /// <summary>
        /// Get the field value for the given index number.
        /// </summary>
        /// <returns>
        /// The value of the field or null if index is out of bounds.
        /// </returns>
        property Object^ default[ int32_t ]
        {
          Object^ get( int32_t index );
        }

        /// <summary>
        /// Get the field value for the given field name.
        /// </summary>
        /// <returns>The value of the field.</returns>
        /// <exception cref="IllegalArgumentException">
        /// if the field name is not found.
        /// </exception>
        property Object^ default[ String^ ]
        {
          Object^ get( String^ fieldName );
        }

        /// <summary>
        /// Get the parent <c>StructSet</c> of this <c>Struct</c>.
        /// </summary>
        /// <returns>
        /// A reference to the parent <c>StructSet</c> of this <c>Struct</c>.
        /// </returns>
        property Apache::Geode::Client::StructSet<Object^>^ Set
        {
          Apache::Geode::Client::StructSet<Object^>^ get( );
        }

        /// <summary>
        /// Get the number of field values available.
        /// </summary>
        /// <returns>the number of field values available.</returns>
        property int32_t Count
        {
          int32_t get( );
        }

      private:

        /// <summary>
        /// Private constructor to wrap a native object pointer
        /// </summary>
        /// <param name="nativeptr">The native object pointer</param>
        inline Apache::Geode::Client::Struct/*<TResult>*/( std::shared_ptr<apache::geode::client::Serializable> nativeptr )
          : Apache::Geode::Client::Serializable( nativeptr ) { }

        inline Apache::Geode::Client::Struct/*<TResult>*/(  )
          : Apache::Geode::Client::Serializable( std::shared_ptr<apache::geode::client::Serializable>(apache::geode::client::Struct::createDeserializable())) { }

      internal:

        /// <summary>
        /// Factory function to register wrapper
        /// </summary>
        inline static Apache::Geode::Client::ISerializable^ Create( ::std::shared_ptr<apache::geode::client::Serializable> obj )
        {
          return ( obj != nullptr ?
            gcnew Apache::Geode::Client::Struct( obj ) : nullptr );
        }

        inline static Apache::Geode::Client::ISerializable^ CreateDeserializable( )
        {
          return gcnew Apache::Geode::Client::Struct(  ) ;
        }
      };

    }  // namespace Client
  }  // namespace Geode
}  // namespace Apache
