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
#include "ISerializable.hpp"
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
      namespace native = apache::geode::client;

      /// <summary>
      /// A mutable <c>ICacheableKey</c> to <c>ISerializable</c> hash map
      /// that can serve as a distributable object for caching. This class
      /// extends .NET generic <c>Dictionary</c> class.
      /// </summary>
      ref class CacheableHashMap
        : public IDataSerializablePrimitive
      {
      protected:
        Object^ m_dictionary;
      public:
        /// <summary>
        /// Allocates a new empty instance.
        /// </summary>
        inline CacheableHashMap()
        { }

        /// <summary>
        /// Allocates a new instance copying from the given dictionary.
        /// </summary>
        /// <param name="dictionary">
        /// The dictionary whose elements are copied to this HashMap.
        /// </param>
        inline CacheableHashMap(Object^ dictionary)
        {
          m_dictionary = dictionary;
        }


        /// <summary>
        /// Static function to create a new empty instance.
        /// </summary>
        inline static CacheableHashMap^ Create()
        {
          return gcnew CacheableHashMap();
        }

        /// <summary>
        /// Static function to create a new instance copying from the
        /// given dictionary.
        /// </summary>
        inline static CacheableHashMap^ Create(Object^ dictionary)
        {
          return gcnew CacheableHashMap(dictionary);
        }


        // Region: ISerializable Members

        /// <summary>
        /// Serializes this object.
        /// </summary>
        /// <param name="output">
        /// the DataOutput object to use for serializing the object
        /// </param>
        virtual void ToData(DataOutput^ output);

        /// <summary>
        /// Deserialize this object, typical implementation should return
        /// the 'this' pointer.
        /// </summary>
        /// <param name="input">
        /// the DataInput stream to use for reading the object data
        /// </param>
        /// <returns>the deserialized object</returns>
        virtual void FromData(DataInput^ input);

        /// <summary>
        /// return the size of this object in bytes
        /// </summary>
        virtual property System::UInt64 ObjectSize
        {
          virtual System::UInt64 get();
        }

        /// <summary>
        /// Returns the classId of the instance being serialized.
        /// This is used by deserialization to determine what instance
        /// type to create and deserialize into.
        /// </summary>
        /// <returns>the classId</returns>
        property int8_t DsCode
        {
          virtual int8_t get()
          {
            return static_cast<int8_t>(native::internal::DSCode::CacheableHashMap);
          }
        }

        property Object^ Value
        {
          Object^ get()
          {
            return m_dictionary;
          }
        }

        // End Region: ISerializable Members

        /// <summary>
        /// Factory function to register this class.
        /// </summary>
        static ISerializable^ CreateDeserializable()
        {
          return gcnew CacheableHashMap();
        }
      };
    }  // namespace Client
  }  // namespace Geode
}  // namespace Apache

