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

#include "CacheableKey.hpp"
#include "Serializable.hpp"
#include "ExceptionTypes.hpp"
#include "DataOutput.hpp"
#include "DataInput.hpp"

using namespace System;

namespace Apache
{
  namespace Geode
  {
    namespace Client
    {

      namespace native = apache::geode::client;


      /// <summary>
      /// An immutable template wrapper for C++ <c>CacheableKey</c>s that can
      /// serve as a distributable key object for caching.
      /// </summary>
      template <typename TNative, typename TManaged, int8_t TYPEID>
      ref class CacheableBuiltinKey
        : public IDataSerializablePrimitive, public CacheableKey
      {
      public:
        /// <summary>
        /// Allocates a new instance 
        /// </summary>
        CacheableBuiltinKey()
        {
          auto nativeptr = TNative::create();
          m_nativeptr = gcnew native_shared_ptr<native::Serializable>(nativeptr);
        }

        /// <summary>
        /// Allocates a new instance with the given value.
        /// </summary>
        /// <param name="value">the value of the new instance</param>
        CacheableBuiltinKey(TManaged value)
        {
          auto nativeptr = TNative::create(value);
          m_nativeptr = gcnew native_shared_ptr<native::Serializable>(nativeptr);
        }

        property int8_t DsCode
        {
          virtual int8_t get()
          {
            return TYPEID;
          }
        }

        /// <summary>
        /// Return a string representation of the object.
        /// This returns the string for the <c>Value</c> property.
        /// </summary>
        String^ ToString() override
        {
          try
          {
            return GetNative()->value().ToString();
          }
          finally
          {
            GC::KeepAlive(m_nativeptr);
          }

        }

        /// <summary>
        /// Return true if this key matches other object.
        /// It invokes the '==' operator of the underlying
        /// native object.
        /// </summary>
        virtual bool Equals(ICacheableKey^ other) override
        {
          if (auto o = dynamic_cast<CacheableBuiltinKey^>(other)) {
            try
            {
              return GetNative()->operator==(
                *dynamic_cast<TNative*>(o->m_nativeptr->get()));
            }
            finally
            {
              GC::KeepAlive(m_nativeptr);
              GC::KeepAlive(o->m_nativeptr);
            }
          }

          return false;
        }

        /// <summary>
        /// Return true if this key matches other object.
        /// It invokes the '==' operator of the underlying
        /// native object.
        /// </summary>
        virtual bool Equals(Object^ obj) override
        {
          return Equals(dynamic_cast<CacheableBuiltinKey^>(obj));
        }

        /// <summary>
        /// Comparison operator against another value.
        /// </summary>
        bool operator == (TManaged other)
        {
          try
          {
            return (GetNative()->value() == other);
          }
          finally
          {
            GC::KeepAlive(m_nativeptr);
          }

        }

        /// <summary>
        /// Gets the value.
        /// </summary>
        property TManaged Value
        {
          inline TManaged get()
          {
            try
            {
              return GetNative()->value();
            }
            finally
            {
              GC::KeepAlive(m_nativeptr);
            }

          }
        }

        virtual void ToData(DataOutput^ dataOutput)
        {
          try
          {
            return GetNative()->toData(*dataOutput->GetNative());
          }
          finally
          {
            GC::KeepAlive(m_nativeptr);
          }
        }

        virtual void FromData(DataInput^ dataInput)
        {
          try
          {
            return GetNative()->fromData(*dataInput->GetNative());
          }
          finally
          {
            GC::KeepAlive(m_nativeptr);
          }
        }

      protected:

        /// <summary>
        /// Protected constructor to wrap a native object pointer
        /// </summary>
        /// <param name="nativeptr">The native object pointer</param>
        inline CacheableBuiltinKey(std::shared_ptr<native::Serializable> nativeptr)
          : CacheableKey(nativeptr) { }

      private:
        inline TNative* GetNative()
        {
          return dynamic_cast<TNative*>(m_nativeptr->get());
        }
      };


      /// <summary>
      /// An immutable template array wrapper that can serve as a
      /// distributable object for caching.
      /// </summary>
      template <typename TNative, typename TNativePtr, typename TManaged,
        int8_t TYPEID>
      ref class CacheableBuiltinArray
        : public IDataSerializablePrimitive
      {
      public:

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
            return TYPEID;
          }
        }

        virtual void ToData(DataOutput^ output)
        {
          output->WriteObject(m_value);
        }

        virtual void FromData(DataInput^ input)
        {
          input->ReadObject(m_value);
        }

        property System::UInt64 ObjectSize
        {
          virtual System::UInt64 get()
          {
            return m_value->Length * sizeof(TManaged);
          }
        }
        /// <summary>
        /// Returns a copy of the underlying array.
        /// </summary>
        property array<TManaged>^ Value
        {
          inline array<TManaged>^ get()
          {
            return m_value;
          }
        }

        /// <summary>
        /// Returns the size of this array.
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
        /// Returns the value at the given index.
        /// </summary>
        property TManaged default[ System::Int32 ]
        {
          inline TManaged get(System::Int32 index)
          {
            return m_value[index];
          }
        }


      protected:

        array<TManaged>^ m_value;
        /// <summary>
        /// Protected constructor 
        /// </summary>
        inline CacheableBuiltinArray()
        {
          //TODO:
          //native::Serializable* sp = TNative::createDeserializable();
          //SetSP(sp);
        }

        /// <summary>
        /// Protected constructor to wrap a native object pointer
        /// </summary>
        /// <param name="nativeptr">The native object pointer</param>
        inline CacheableBuiltinArray(std::shared_ptr<native::Serializable> nptr)
        {
          auto nativeptr = std::dynamic_pointer_cast<TNative>(nptr);
          System::Int32 len = nativeptr->length();
          if (len > 0)
          {
            array<TManaged>^ buffer = gcnew array<TManaged>(len);
            pin_ptr<TManaged> pin_buffer = &buffer[0];

            memcpy((void*)pin_buffer, nativeptr->value().data(),
                   len * sizeof(TManaged));
            m_value = buffer;
          }
        }

        /// <summary>
        /// Allocates a new instance copying from the given array.
        /// </summary>
        /// <remarks>
        /// This method performs no argument checking which is the
        /// responsibility of the caller.
        /// </remarks>
        /// <param name="buffer">the array to copy from</param>
        CacheableBuiltinArray(array<TManaged>^ buffer)
        {
          m_value = buffer;
          //setting local value as well
          //m_value = gcnew array<TManaged>(buffer->Length);
          //System::Array::Copy(buffer, 0, m_value,0, buffer->Length);             
        }

        /// <summary>
        /// Allocates a new instance copying given length from the
        /// start of given array.
        /// </summary>
        /// <remarks>
        /// This method performs no argument checking which is the
        /// responsibility of the caller.
        /// </remarks>
        /// <param name="buffer">the array to copy from</param>
        /// <param name="length">length of array from start to copy</param>
        CacheableBuiltinArray(array<TManaged>^ buffer, System::Int32 length)
        {
          //TODO:
          if (length > buffer->Length) {
            length = buffer->Length;
          }
          //setting local value as well
          m_value = gcnew array<TManaged>(length);
          System::Array::Copy(buffer, 0, m_value, 0, length);
        }
      };

      // Built-in CacheableKeys

      /// <summary>
      /// An immutable wrapper for booleans that can serve
      /// as a distributable key object for caching.
      /// </summary>
      ref class CacheableBoolean : public CacheableBuiltinKey<native::CacheableBoolean, bool, static_cast<int8_t>(DSCode::CacheableBoolean)> { public: inline CacheableBoolean() : CacheableBuiltinKey() { } inline CacheableBoolean(bool value) : CacheableBuiltinKey(value) { } inline static CacheableBoolean^ Create(bool value) { return gcnew CacheableBoolean(value); } inline static explicit operator bool (CacheableBoolean^ value) { return value->Value; } static ISerializable^ CreateDeserializable() { return gcnew CacheableBoolean(); } internal: static ISerializable^ Create(std::shared_ptr<native::Serializable> obj) { return (obj != nullptr ? gcnew CacheableBoolean(obj) : nullptr); } private: inline CacheableBoolean(std::shared_ptr<native::Serializable> nativeptr) : CacheableBuiltinKey(nativeptr) { } };;

      /// <summary>
      /// An immutable wrapper for bytes that can serve
      /// as a distributable key object for caching.
      /// </summary>
      ref class CacheableByte : public CacheableBuiltinKey<native::CacheableByte, Byte, static_cast<int8_t>(DSCode::CacheableByte)> { public: inline CacheableByte() : CacheableBuiltinKey() { } inline CacheableByte(Byte value) : CacheableBuiltinKey(value) { } inline static CacheableByte^ Create(Byte value) { return gcnew CacheableByte(value); } inline static explicit operator Byte (CacheableByte^ value) { return value->Value; } static ISerializable^ CreateDeserializable() { return gcnew CacheableByte(); } internal: static ISerializable^ Create(std::shared_ptr<native::Serializable> obj) { return (obj != nullptr ? gcnew CacheableByte(obj) : nullptr); } private: inline CacheableByte(std::shared_ptr<native::Serializable> nativeptr) : CacheableBuiltinKey(nativeptr) { } };;

      /// <summary>
      /// An immutable wrapper for 16-bit characters that can serve
      /// as a distributable key object for caching.
      /// </summary>
      ref class CacheableCharacter : public CacheableBuiltinKey<native::CacheableCharacter, Char, static_cast<int8_t>(DSCode::CacheableCharacter)> { public: inline CacheableCharacter() : CacheableBuiltinKey() { } inline CacheableCharacter(Char value) : CacheableBuiltinKey(value) { } inline static CacheableCharacter^ Create(Char value) { return gcnew CacheableCharacter(value); } inline static explicit operator Char (CacheableCharacter^ value) { return value->Value; } static ISerializable^ CreateDeserializable() { return gcnew CacheableCharacter(); } internal: static ISerializable^ Create(std::shared_ptr<native::Serializable> obj) { return (obj != nullptr ? gcnew CacheableCharacter(obj) : nullptr); } private: inline CacheableCharacter(std::shared_ptr<native::Serializable> nativeptr) : CacheableBuiltinKey(nativeptr) { } };;

      /// <summary>
      /// An immutable wrapper for doubles that can serve
      /// as a distributable key object for caching.
      /// </summary>
      ref class CacheableDouble : public CacheableBuiltinKey<native::CacheableDouble, Double, static_cast<int8_t>(DSCode::CacheableDouble)> { public: inline CacheableDouble() : CacheableBuiltinKey() { } inline CacheableDouble(Double value) : CacheableBuiltinKey(value) { } inline static CacheableDouble^ Create(Double value) { return gcnew CacheableDouble(value); } inline static explicit operator Double (CacheableDouble^ value) { return value->Value; } static ISerializable^ CreateDeserializable() { return gcnew CacheableDouble(); } internal: static ISerializable^ Create(std::shared_ptr<native::Serializable> obj) { return (obj != nullptr ? gcnew CacheableDouble(obj) : nullptr); } private: inline CacheableDouble(std::shared_ptr<native::Serializable> nativeptr) : CacheableBuiltinKey(nativeptr) { } };;

      /// <summary>
      /// An immutable wrapper for floats that can serve
      /// as a distributable key object for caching.
      /// </summary>
      ref class CacheableFloat : public CacheableBuiltinKey<native::CacheableFloat, Single, static_cast<int8_t>(DSCode::CacheableFloat)> { public: inline CacheableFloat() : CacheableBuiltinKey() { } inline CacheableFloat(Single value) : CacheableBuiltinKey(value) { } inline static CacheableFloat^ Create(Single value) { return gcnew CacheableFloat(value); } inline static explicit operator Single (CacheableFloat^ value) { return value->Value; } static ISerializable^ CreateDeserializable() { return gcnew CacheableFloat(); } internal: static ISerializable^ Create(std::shared_ptr<native::Serializable> obj) { return (obj != nullptr ? gcnew CacheableFloat(obj) : nullptr); } private: inline CacheableFloat(std::shared_ptr<native::Serializable> nativeptr) : CacheableBuiltinKey(nativeptr) { } };;

      /// <summary>
      /// An immutable wrapper for 16-bit integers that can serve
      /// as a distributable key object for caching.
      /// </summary>
      ref class CacheableInt16 : public CacheableBuiltinKey<native::CacheableInt16, System::Int16, static_cast<int8_t>(DSCode::CacheableInt16)> { public: inline CacheableInt16() : CacheableBuiltinKey() { } inline CacheableInt16(System::Int16 value) : CacheableBuiltinKey(value) { } inline static CacheableInt16^ Create(System::Int16 value) { return gcnew CacheableInt16(value); } inline static explicit operator System::Int16 (CacheableInt16^ value) { return value->Value; } static ISerializable^ CreateDeserializable() { return gcnew CacheableInt16(); } internal: static ISerializable^ Create(std::shared_ptr<native::Serializable> obj) { return (obj != nullptr ? gcnew CacheableInt16(obj) : nullptr); } private: inline CacheableInt16(std::shared_ptr<native::Serializable> nativeptr) : CacheableBuiltinKey(nativeptr) { } };;

      /// <summary>
      /// An immutable wrapper for 32-bit integers that can serve
      /// as a distributable key object for caching.
      /// </summary>
      ref class CacheableInt32 : public CacheableBuiltinKey<native::CacheableInt32, System::Int32, static_cast<int8_t>(DSCode::CacheableInt32)> { public: inline CacheableInt32() : CacheableBuiltinKey() { } inline CacheableInt32(System::Int32 value) : CacheableBuiltinKey(value) { } inline static CacheableInt32^ Create(System::Int32 value) { return gcnew CacheableInt32(value); } inline static explicit operator System::Int32 (CacheableInt32^ value) { return value->Value; } static ISerializable^ CreateDeserializable() { return gcnew CacheableInt32(); } internal: static ISerializable^ Create(std::shared_ptr<native::Serializable> obj) { return (obj != nullptr ? gcnew CacheableInt32(obj) : nullptr); } private: inline CacheableInt32(std::shared_ptr<native::Serializable> nativeptr) : CacheableBuiltinKey(nativeptr) { } };;

      /// <summary>
      /// An immutable wrapper for 64-bit integers that can serve
      /// as a distributable key object for caching.
      /// </summary>
      ref class CacheableInt64 : public CacheableBuiltinKey<native::CacheableInt64, System::Int64, static_cast<int8_t>(DSCode::CacheableInt64)> { public: inline CacheableInt64() : CacheableBuiltinKey() { } inline CacheableInt64(System::Int64 value) : CacheableBuiltinKey(value) { } inline static CacheableInt64^ Create(System::Int64 value) { return gcnew CacheableInt64(value); } inline static explicit operator System::Int64 (CacheableInt64^ value) { return value->Value; } static ISerializable^ CreateDeserializable() { return gcnew CacheableInt64(); } internal: static ISerializable^ Create(std::shared_ptr<native::Serializable> obj) { return (obj != nullptr ? gcnew CacheableInt64(obj) : nullptr); } private: inline CacheableInt64(std::shared_ptr<native::Serializable> nativeptr) : CacheableBuiltinKey(nativeptr) { } };;



      // Built-in Cacheable array types

      template <typename NativeArray, typename ManagedType, int8_t DsCode>
      ref class CacheableArray : public CacheableBuiltinArray<
          NativeArray, NativeArray, ManagedType, DsCode> {
      public:
          /** <summary>
          *  Static function to create a new instance copying
          *  from the given array.
          *  </summary>
          *  <remarks>
          *  Providing a null or zero size array will return a null object.
          *  </remarks>
          *  <param name="value">the array to create the new instance</param>
          */
          inline static CacheableArray^ Create(array<ManagedType>^ value)
          {
              return (value != nullptr /*&& value->Length > 0*/ ?
                  gcnew CacheableArray(value) : nullptr);
          }
          /** <summary>
          *  Static function to create a new instance copying
          *  from the given array.
          *  </summary>
          *  <remarks>
          *  Providing a null or zero size array will return a null object.
          *  </remarks>
          *  <param name="value">the array to create the new instance</param>
          */
          inline static CacheableArray^ Create(array<ManagedType>^ value, System::Int32 length)
          {
              return (value != nullptr && value->Length > 0 ?
                  gcnew CacheableArray(value, length) : nullptr);
          }
          /** <summary>
          * Explicit conversion operator to contained array type.
          * </summary>
          */
          inline static explicit operator array<ManagedType> ^ (CacheableArray^ value)
          {
              return (value != nullptr ? value->Value : nullptr);
          }
          /** <summary>
          * Factory function to register this class.
          * </summary>
          */
          static ISerializable^ CreateDeserializable()
          {
              return gcnew CacheableArray();
          }
        internal:
          static ISerializable^ Create(std::shared_ptr<native::Serializable> obj)
          {
              return (obj != nullptr ? gcnew CacheableArray(obj) : nullptr);
          }
        private:
          /** <summary>
          * Allocates a new instance
          *  </summary>
          */
          inline CacheableArray() : CacheableBuiltinArray() { }
          /** <summary>
          * Allocates a new instance copying from the given array.
          *  </summary>
          *  <remarks>
          *  Providing a null or zero size array will return a null object.
          *  </remarks>
          *  <param name="value">the array to create the new instance</param>
          */
          inline CacheableArray(array<ManagedType>^ value) : CacheableBuiltinArray(value) { }
          /** <summary>
          * Allocates a new instance copying given length from the
          * start of given array.
          *  </summary>
          *  <remarks>
          *  Providing a null or zero size array will return a null object.
          *  </remarks>
          *  <param name="value">the array to create the new instance</param>
          */
          inline CacheableArray(array<ManagedType>^ value, System::Int32 length)
            : CacheableBuiltinArray(value, length) { }
          inline CacheableArray(std::shared_ptr<native::Serializable> nativeptr)
            : CacheableBuiltinArray(nativeptr) { }
      };

      /// <summary>
      /// An immutable wrapper for byte arrays that can serve
      /// as a distributable object for caching.
      /// </summary>
      using CacheableBytes = CacheableArray<native::internal::CacheableArrayPrimitive<int8_t, native::internal::DSCode::CacheableBytes>, Byte, static_cast<int8_t>(DSCode::CacheableBytes)>;

      /// <summary>
      /// An immutable wrapper for array of doubles that can serve
      /// as a distributable object for caching.
      /// </summary>
      using CacheableDoubleArray = CacheableArray<native::internal::CacheableArrayPrimitive<double, native::internal::DSCode::CacheableDoubleArray>, Double, static_cast<int8_t>(DSCode::CacheableDoubleArray)>;

      /// <summary>
      /// An immutable wrapper for array of floats that can serve
      /// as a distributable object for caching.
      /// </summary>
      using CacheableFloatArray = CacheableArray<native::internal::CacheableArrayPrimitive<float, native::internal::DSCode::CacheableFloatArray>, Single, static_cast<int8_t>(DSCode::CacheableFloatArray)>;

      /// <summary>
      /// An immutable wrapper for array of 16-bit integers that can serve
      /// as a distributable object for caching.
      /// </summary>
      using CacheableInt16Array = CacheableArray<native::internal::CacheableArrayPrimitive<int16_t, native::internal::DSCode::CacheableInt16Array>, System::Int16, static_cast<int8_t>(DSCode::CacheableInt16Array)>;

      /// <summary>
      /// An immutable wrapper for array of 32-bit integers that can serve
      /// as a distributable object for caching.
      /// </summary>
      using CacheableInt32Array = CacheableArray<native::internal::CacheableArrayPrimitive<int32_t, native::internal::DSCode::CacheableInt32Array>, System::Int32, static_cast<int8_t>(DSCode::CacheableInt32Array)>;

      /// <summary>
      /// An immutable wrapper for array of 64-bit integers that can serve
      /// as a distributable object for caching.
      /// </summary>
      using CacheableInt64Array = CacheableArray<native::internal::CacheableArrayPrimitive<int64_t, native::internal::DSCode::CacheableInt64Array>, System::Int64, static_cast<int8_t>(DSCode::CacheableInt64Array)>;

      /// <summary>
      /// An immutable wrapper for array of booleans that can serve
      /// as a distributable object for caching.
      /// </summary>
      using BooleanArray = CacheableArray<native::internal::CacheableArrayPrimitive<bool, native::internal::DSCode::BooleanArray>, bool, static_cast<int8_t>(DSCode::BooleanArray)>;

      /// <summary>
      /// An immutable wrapper for array of 16-bit characters that can serve
      /// as a distributable object for caching.
      /// </summary>
      using CharArray = CacheableArray<native::internal::CacheableArrayPrimitive<char16_t, native::internal::DSCode::CharArray>, Char, static_cast<int8_t>(DSCode::CharArray)>;
    }  // namespace Client
  }  // namespace Geode
}  // namespace Apache

