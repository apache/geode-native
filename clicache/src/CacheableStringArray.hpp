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
#include <geode/CacheableBuiltins.hpp>
#include "end_native.hpp"

#include "Serializable.hpp"
#include "CacheableString.hpp"

using namespace System;

namespace Apache
{
  namespace Geode
  {
    namespace Client
    {

      ref class CacheableString;

      /// <summary>
      /// An immutable wrapper for array of strings that can serve as
      /// a distributable object for caching.
      /// </summary>
      ref class CacheableStringArray
        :  public IDataSerializablePrimitive
      {
      public:
        /// <summary>
        /// Static function to create a new instance copying from the given
        /// string array.
        /// </summary>
        /// <remarks>
        /// If the given array of strings is null or of zero-length then
        /// this method returns null.
        /// </remarks>
        /// <exception cref="IllegalArgumentException">
        /// If the array contains a string greater than or equal 64K in length.
        /// </exception>
        inline static CacheableStringArray^ Create(array<String^>^ strings)
        {
          return (strings != nullptr && strings->Length > 0 ?
                  gcnew CacheableStringArray(strings) : nullptr);
        }

        virtual void ToData(DataOutput^ output);

        virtual void FromData(DataInput^ input);

        property int8_t DsCode
        {
          virtual int8_t get()
          {
            return static_cast<int8_t>(DSCode::CacheableStringArray);
          }
        }

        property System::UInt64 ObjectSize
        {
          virtual System::UInt64 get()
          {
            auto size = sizeof(this);
            for (int i = 0; i < m_value->Length; i++)
            {
              size += m_value[i]->Length;
            }
            return size;
          }

        }

        /// <summary>
        /// Returns a copy of the underlying array of strings.
        /// </summary>
        array<String^>^ GetValues();

        /// <summary>
        /// Returns a copy of the underlying string at the given index.
        /// </summary>
        property String^ default[ System::Int32 ]
        {
          String^ get(System::Int32 index);
        }

        /// <summary>
        /// Gets the length of the array.
        /// </summary>
        property System::Int32 Length
        {
          inline System::Int32 get()
          {
            return m_value->Length;
          }
        }

        virtual String^ ToString() override
        {
          return m_value->ToString();
        }

        /// <summary>
        /// Factory function to register this class.
        /// </summary>
        static ISerializable^ CreateDeserializable()
        {
          return gcnew CacheableStringArray();
        }


      private:
        array<String^>^ m_value;
        /// <summary>
        /// Allocates a new instance copying from the given string array.
        /// </summary>
        /// <exception cref="IllegalArgumentException">
        /// If the array contains a string greater than or equal 64K in length.
        /// </exception>
        CacheableStringArray(array<String^>^ strings);


        inline CacheableStringArray()
        {
        }
      };
    }  // namespace Client
  }  // namespace Geode
}  // namespace Apache


