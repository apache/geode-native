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

#include <msclr/marshal.h>

#include "geode_defs.hpp"
#include "begin_native.hpp"
#include <geode/CacheableString.hpp>
#include "end_native.hpp"

#include "CacheableKey.hpp"
#include "IDataSerializablePrimitive.hpp"

using namespace System;

namespace Apache
{
  namespace Geode
  {
    namespace Client
    {

      /// <summary>
      /// An immutable string wrapper that can serve as a distributable
      /// key object for caching as well as being a string value.
      /// </summary>
      ref class CacheableString
        :  public IDataSerializablePrimitive, public CacheableKey
      {
      public:
        /// <summary>
        /// Allocates a new instance copying from the given string.
        /// </summary>
        /// <param name="value">the string value of the new instance</param>
        /// <exception cref="IllegalArgumentException">
        /// if the provided string is null or has zero length
        /// </exception>
        CacheableString(String^ value);

        /// <summary>
        /// Allocates a new instance copying from the given character array.
        /// </summary>
        /// <param name="value">
        /// the character array value of the new instance
        /// </param>
        /// <exception cref="IllegalArgumentException">
        /// if the provided array is null or has zero length
        /// </exception>
        CacheableString(array<Char>^ value);

        /// <summary>
        /// Static function to create a new instance copying from
        /// the given string.
        /// </summary>
        /// <remarks>
        /// Providing a null or zero size string will return a null
        /// <c>CacheableString</c> object.
        /// </remarks>
        /// <param name="value">the string value of the new instance</param>
        inline static CacheableString^ Create(String^ value)
        {
          return (value != nullptr ?
                  gcnew CacheableString(value, true) : nullptr);
        }

        virtual void ToData(DataOutput^ output);

        virtual void FromData(DataInput^ input);

        property int8_t DsCode
        {
          virtual int8_t get()
          {
            return static_cast<int8_t>(m_type);
          }
        }

        property System::UInt64 ObjectSize
        {
          System::UInt64 get() override;
        }

        /// <summary>
        /// Static function to create a new instance copying from
        /// the given character array.
        /// </summary>
        /// <remarks>
        /// Providing a null or zero size character array will return a null
        /// <c>CacheableString</c> object.
        /// </remarks>
        /// <param name="value">
        /// the character array value of the new instance
        /// </param>
        inline static CacheableString^ Create(array<Char>^ value)
        {
          return (value != nullptr && value->Length > 0 ?
                  gcnew CacheableString(value, true) : nullptr);
        }

        /// <summary>
        /// Return a string representation of the object.
        /// This returns the same string as <c>Value</c> property.
        /// </summary>
        virtual String^ ToString() override
        {
          return m_value;
        }

        /// <summary>
        /// Return true if this key matches other object.
        /// It invokes the '==' operator of the underlying
        /// <c>apache::geode::client::CacheableString</c> object.
        /// </summary>
        virtual bool Equals(Apache::Geode::Client::ICacheableKey^ other) override;

        /// <summary>
        /// Return true if this key matches other object.
        /// It invokes the '==' operator of the underlying
        /// <c>apache::geode::client::CacheableString</c> object.
        /// </summary>
        virtual bool Equals(Object^ obj) override;

        /// <summary>
        /// Return the hashcode for this key.
        /// </summary>
        virtual System::Int32 GetHashCode() override;

        /// <summary>
        /// Gets the string value.
        /// </summary>
        property String^ Value
        {
          inline String^ get()
          {
            return m_value;
          }
        }

        /// <summary>
        /// Static function to check whether IsNullOrEmpty.
        /// </summary>
        /// <remarks>
        /// This is similar to the C# string.IsNullOrEmpty method.
        /// </remarks>
        /// <param name="value">the CacheableString value to check</param>
        inline static bool IsNullOrEmpty(CacheableString^ value)
        {
          return (value == nullptr || value->Length == 0);
        }

        /// <summary>
        /// Implicit conversion operator to underlying <c>System.String</c>.
        /// </summary>
        inline static operator String ^ (CacheableString^ str)
        {
          return (str != nullptr ? str->ToString() : nullptr);
        }

        /// <summary>
        /// Gets the length of the underlying C string.
        /// </summary>
        property System::UInt32 Length
        {
          inline System::UInt32 get()
          {
            return m_value->Length;
          }
        }

        /// <summary>
        /// True when the underlying C string is a wide-character string.
        /// </summary>
        property bool IsWideString
        {
          inline bool get()
          {
            return true;//TODO:
          }
        }

      internal:
        static ISerializable^ CreateDeserializable()
        {
          return gcnew CacheableString(static_cast<int8_t>(DSCode::CacheableASCIIString));
        }

        static ISerializable^ createDeserializableHuge()
        {
          return gcnew CacheableString(static_cast<int8_t>(DSCode::CacheableASCIIStringHuge));
        }

        static ISerializable^ createUTFDeserializable()
        {
          return gcnew CacheableString(static_cast<int8_t>(DSCode::CacheableString));
        }

        static ISerializable^ createUTFDeserializableHuge()
        {
          return gcnew CacheableString(static_cast<int8_t>(DSCode::CacheableStringHuge));
        }
        /// <summary>
        /// Factory function to register wrapper
        /// </summary>
        static ISerializable^ Create(std::shared_ptr<apache::geode::client::Serializable> obj)
        {
          if (auto str = std::dynamic_pointer_cast<apache::geode::client::CacheableString>(obj)) {
            return gcnew CacheableString(str);
          }

          return nullptr;
        }

        CacheableString(System::UInt32 type) : CacheableKey()
        {
          m_type = static_cast<DSCode>(type);
        }

      private:
        String^ m_value;
        DSCode m_type;
        int m_hashcode;

        CacheableString() : CacheableKey()
        {
          m_type = DSCode::CacheableASCIIString;
        }

        void SetStringType();
        /// <summary>
        /// Private constructor to create a CacheableString without checking
        /// for arguments.
        /// </summary>
        CacheableString(String^ value, bool noParamCheck);

        /// <summary>
        /// Private constructor to create a CacheableString without checking
        /// for arguments.
        /// </summary>
        CacheableString(array<Char>^ value, bool noParamCheck);

        /// <summary>
        /// Private constructor to wrap a native object pointer
        /// </summary>
        /// <param name="nativeptr">The native object pointer</param>
        inline CacheableString(std::shared_ptr<apache::geode::client::CacheableString> nativeptr)
          : CacheableKey(nativeptr) {
        
          m_value = msclr::interop::marshal_as<String^>(nativeptr->value());
          m_type = nativeptr->getDsCode();
          m_hashcode = nativeptr->hashcode();
        }
      };
    }  // namespace Client
  }  // namespace Geode
}  // namespace Apache


