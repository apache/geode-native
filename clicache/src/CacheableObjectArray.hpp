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
#include "IDataSerializablePrimitive.hpp"

#include "begin_native.hpp"
#include <geode/internal/DSCode.hpp>
#include "end_native.hpp"

using namespace System;
using namespace System::Collections::Generic;

namespace Apache
{
  namespace Geode
  {
    namespace Client
    {

      using namespace apache::geode::client;

      /// <summary>
      /// A mutable <c>ISerializable</c> object array wrapper that can serve
      /// as a distributable object for caching. Though this class provides
      /// compatibility with java Object[] serialization, it provides the
      /// semantics of .NET generic <c>List</c> class.
      /// </summary>
      public ref class CacheableObjectArray
        : public List<Object^>, public IDataSerializablePrimitive
      {
      public:
        /// <summary>
        /// Allocates a new empty instance.
        /// </summary>
        inline CacheableObjectArray()
          : List<Object^>()
        { }

        /// <summary>
        /// Allocates a new instance copying from the given collection.
        /// </summary>
        /// <param name="collection">
        /// The collection whose elements are copied to this list.
        /// </param>
        inline CacheableObjectArray(IEnumerable<Object^>^ collection)
          : List<Object^>(collection)
        { }

        /// <summary>
        /// Allocates a new empty instance with given initial size.
        /// </summary>
        /// <param name="capacity">
        /// The initial capacity of the vector.
        /// </param>
        inline CacheableObjectArray(System::Int32 capacity)
          : List<Object^>(capacity)
        { }

        /// <summary>
        /// Static function to create a new empty instance.
        /// </summary>
        inline static CacheableObjectArray^ Create()
        {
          return gcnew CacheableObjectArray();
        }

        /// <summary>
        /// Static function to create a new instance copying from the
        /// given collection.
        /// </summary>
        inline static CacheableObjectArray^ Create(
          IEnumerable<Object^>^ collection)
        {
          return gcnew CacheableObjectArray(collection);
        }

        /// <summary>
        /// Static function to create a new instance with given initial size.
        /// </summary>
        inline static CacheableObjectArray^ Create(System::Int32 capacity)
        {
          return gcnew CacheableObjectArray(capacity);
        }

        // Region: ISerializable Members

        virtual void ToData(DataOutput^ output);

        virtual void FromData(DataInput^ input);

        property System::UInt64 ObjectSize
        {
          virtual System::UInt64 get();
        }

        property int8_t DsCode
        {
          virtual int8_t get()
          {
            return static_cast<int8_t>(internal::DSCode::CacheableObjectArray);
          }
        }

        // End Region: ISerializable Members

        /// <summary>
        /// Factory function to register this class.
        /// </summary>
        static ISerializable^ CreateDeserializable()
        {
          return gcnew CacheableObjectArray();
        }
      };
    }  // namespace Client
  }  // namespace Geode
}  // namespace Apache

