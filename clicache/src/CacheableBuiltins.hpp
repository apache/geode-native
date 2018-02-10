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
#include "GeodeClassIds.hpp"
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
      template <typename TNative, typename TManaged, System::UInt32 TYPEID>
      ref class CacheableBuiltinKey
        : public CacheableKey
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

        /// <summary>
        /// Returns the classId of the instance being serialized.
        /// This is used by deserialization to determine what instance
        /// type to create and deserialize into.
        /// </summary>
        /// <returns>the classId</returns>
        virtual property System::UInt32 ClassId
        {
          virtual System::UInt32 get() override
          {
            return TYPEID;
          }
        }

        /// <summary>
        /// Return a string representation of the object.
        /// This returns the string for the <c>Value</c> property.
        /// </summary>
        virtual String^ ToString() override
        {
          try
          {
            return static_cast<TNative*>(m_nativeptr->get())->value().ToString();
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
              return static_cast<TNative*>(m_nativeptr->get())->operator==(
                *static_cast<TNative*>(o->m_nativeptr->get()));
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
            return (static_cast<TNative*>(m_nativeptr->get())->value() == other);
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
              return static_cast<TNative*>(m_nativeptr->get())->value();
            }
            finally
            {
              GC::KeepAlive(m_nativeptr);
            }

          }
        }

      protected:

        /// <summary>
        /// Protected constructor to wrap a native object pointer
        /// </summary>
        /// <param name="nativeptr">The native object pointer</param>
        inline CacheableBuiltinKey(std::shared_ptr<native::Serializable> nativeptr)
          : CacheableKey(nativeptr) { }
      };


      /// <summary>
      /// An immutable template array wrapper that can serve as a
      /// distributable object for caching.
      /// </summary>
      template <typename TNative, typename TNativePtr, typename TManaged,
        System::UInt32 TYPEID>
      ref class CacheableBuiltinArray
        : public Serializable
      {
      public:

        /// <summary>
        /// Returns the classId of the instance being serialized.
        /// This is used by deserialization to determine what instance
        /// type to create and deserialize into.
        /// </summary>
        /// <returns>the classId</returns>
        virtual property System::UInt32 ClassId
        {
          virtual System::UInt32 get() override
          {
            return TYPEID;
          }
        }

        virtual void ToData(DataOutput^ output) override
        {
          output->WriteObject(m_value);
        }

        virtual void FromData(DataInput^ input) override
        {
          input->ReadObject(m_value);
        }

        virtual property System::UInt64 ObjectSize
        {
          virtual System::UInt64 get() override
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
        property TManaged GFINDEXER(System::Int32)
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
          : Serializable(nptr)
        {
          auto nativeptr = std::static_pointer_cast<TNative>(nptr);
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




      //n = native type
      //m = CacheableInt(managed cacheable)
      //mt = managed type(bool, int)
#define _GFCLI_CACHEABLE_KEY_DEF_NEW(n, m, mt)                                   \
      ref class m : public CacheableBuiltinKey<n, mt,        \
        GeodeClassIds::m>                                                   \
      {                                                                       \
      public:                                                                 \
         /** <summary>
         *  Allocates a new instance with the given value.
         *  </summary>
         *  <param name="value">the value of the new instance</param>
         */                                                                   \
         inline m()                                                            \
         : CacheableBuiltinKey() { }                                         \
         /** <summary>
          *  Allocates a new instance with the given value.
          *  </summary>
          *  <param name="value">the value of the new instance</param>
          */                                                                   \
          inline m(mt value)                                                    \
          : CacheableBuiltinKey(value) { }                                    \
          /** <summary>
           *  Static function to create a new instance given value.
           *  </summary>
           *  <param name="value">the value of the new instance</param>
           */                                                                   \
           inline static m^ Create(mt value)                                     \
           {                                                                     \
           return gcnew m(value);                                              \
           }                                                                     \
           /** <summary>
            * Explicit conversion operator to contained value type.
            * </summary>
            */                                                                   \
            inline static explicit operator mt (m^ value)                         \
           {                                                                     \
           return value->Value;                                                \
           }                                                                     \
           \
           /** <summary>
            * Factory function to register this class.
            * </summary>
            */                                                                   \
            static IGeodeSerializable^ CreateDeserializable()                        \
           {                                                                     \
           return gcnew m();                                       \
           }                                                                     \
           \
           internal:                                                               \
           static IGeodeSerializable^ Create(std::shared_ptr<native::Serializable> obj)            \
           {                                                                     \
           return (obj != nullptr ? gcnew m(obj) : nullptr);                   \
           }                                                                     \
           \
           private:                                                                \
             inline m(std::shared_ptr<native::Serializable> nativeptr)                            \
              : CacheableBuiltinKey(nativeptr) { }                                \
      };


#define _GFCLI_CACHEABLE_ARRAY_DEF_NEW(m, mt)                                    \
      ref class m : public CacheableBuiltinArray<            \
        native::m, native::m, mt, GeodeClassIds::m>                  \
            {                                                                       \
      public:                                                                 \
        /** <summary>
      *  Static function to create a new instance copying
      *  from the given array.
      *  </summary>
      *  <remarks>
      *  Providing a null or zero size array will return a null object.
      *  </remarks>
      *  <param name="value">the array to create the new instance</param>
      */                                                                   \
      inline static m^ Create(array<mt>^ value)                             \
      {                                                                     \
      return (value != nullptr /*&& value->Length > 0*/ ? \
      gcnew m(value) : nullptr);                                        \
      }                                                                     \
      /** <summary>
       *  Static function to create a new instance copying
       *  from the given array.
       *  </summary>
       *  <remarks>
       *  Providing a null or zero size array will return a null object.
       *  </remarks>
       *  <param name="value">the array to create the new instance</param>
       */                                                                   \
       inline static m^ Create(array<mt>^ value, System::Int32 length)               \
      {                                                                     \
      return (value != nullptr && value->Length > 0 ? \
      gcnew m(value, length) : nullptr);                                \
      }                                                                     \
      /** <summary>
       * Explicit conversion operator to contained array type.
       * </summary>
       */                                                                   \
       inline static explicit operator array<mt> ^ (m^ value)                 \
      {                                                                     \
      return (value != nullptr ? value->Value : nullptr);                 \
      }                                                                     \
      \
      /** <summary>
       * Factory function to register this class.
       * </summary>
       */                                                                   \
       static IGeodeSerializable^ CreateDeserializable()                        \
      {                                                                     \
      return gcnew m();                                                   \
      }                                                                     \
      \
            internal:                                                               \
              static IGeodeSerializable^ Create(std::shared_ptr<native::Serializable> obj)            \
      {                                                                     \
      return (obj != nullptr ? gcnew m(obj) : nullptr);                   \
      }                                                                     \
      \
            private:                                                                \
            /** <summary>
             * Allocates a new instance
             *  </summary>
             */                                                                   \
             inline m()                                                            \
             : CacheableBuiltinArray() { }                                       \
             /** <summary>
              * Allocates a new instance copying from the given array.
              *  </summary>
              *  <remarks>
              *  Providing a null or zero size array will return a null object.
              *  </remarks>
              *  <param name="value">the array to create the new instance</param>
              */                                                                   \
              inline m(array<mt>^ value)                                            \
              : CacheableBuiltinArray(value) { }                                  \
              /** <summary>
               * Allocates a new instance copying given length from the
               * start of given array.
               *  </summary>
               *  <remarks>
               *  Providing a null or zero size array will return a null object.
               *  </remarks>
               *  <param name="value">the array to create the new instance</param>
               */                                                                   \
               inline m(array<mt>^ value, System::Int32 length)                              \
               : CacheableBuiltinArray(value, length) { }                          \
               inline m(std::shared_ptr<native::Serializable> nativeptr)                            \
               : CacheableBuiltinArray(nativeptr) { }                              \
      };


      // Built-in CacheableKeys

      /// <summary>
      /// An immutable wrapper for booleans that can serve
      /// as a distributable key object for caching.
      /// </summary>
      _GFCLI_CACHEABLE_KEY_DEF_NEW(native::CacheableBoolean,
                                   CacheableBoolean, bool);

      /// <summary>
      /// An immutable wrapper for bytes that can serve
      /// as a distributable key object for caching.
      /// </summary>
      _GFCLI_CACHEABLE_KEY_DEF_NEW(native::CacheableByte,
                                   CacheableByte, Byte);

      /// <summary>
      /// An immutable wrapper for 16-bit characters that can serve
      /// as a distributable key object for caching.
      /// </summary>
      _GFCLI_CACHEABLE_KEY_DEF_NEW(native::CacheableCharacter,
                                   CacheableCharacter, Char);

      /// <summary>
      /// An immutable wrapper for doubles that can serve
      /// as a distributable key object for caching.
      /// </summary>
      _GFCLI_CACHEABLE_KEY_DEF_NEW(native::CacheableDouble,
                                   CacheableDouble, Double);

      /// <summary>
      /// An immutable wrapper for floats that can serve
      /// as a distributable key object for caching.
      /// </summary>
      _GFCLI_CACHEABLE_KEY_DEF_NEW(native::CacheableFloat,
                                   CacheableFloat, Single);

      /// <summary>
      /// An immutable wrapper for 16-bit integers that can serve
      /// as a distributable key object for caching.
      /// </summary>
      _GFCLI_CACHEABLE_KEY_DEF_NEW(native::CacheableInt16,
                                   CacheableInt16, System::Int16);

      /// <summary>
      /// An immutable wrapper for 32-bit integers that can serve
      /// as a distributable key object for caching.
      /// </summary>
      _GFCLI_CACHEABLE_KEY_DEF_NEW(native::CacheableInt32,
                                   CacheableInt32, System::Int32);

      /// <summary>
      /// An immutable wrapper for 64-bit integers that can serve
      /// as a distributable key object for caching.
      /// </summary>
      _GFCLI_CACHEABLE_KEY_DEF_NEW(native::CacheableInt64,
                                   CacheableInt64, System::Int64);


      // Built-in Cacheable array types

      template <typename NativeArray, typename ManagedType, System::UInt32 GeodeClassId>
      ref class CacheableArray : public CacheableBuiltinArray<
          NativeArray, NativeArray, ManagedType, GeodeClassId> {
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
          static IGeodeSerializable^ CreateDeserializable()
          {
              return gcnew CacheableArray();
          }
        internal:
          static IGeodeSerializable^ Create(std::shared_ptr<native::Serializable> obj)
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
      using CacheableBytes = CacheableArray<native::CacheableArray<int8_t, native::GeodeTypeIds::CacheableBytes>, Byte, GeodeClassIds::CacheableBytes>;

      /// <summary>
      /// An immutable wrapper for array of doubles that can serve
      /// as a distributable object for caching.
      /// </summary>
      using CacheableDoubleArray = CacheableArray<native::CacheableArray<double, native::GeodeTypeIds::CacheableDoubleArray>, Double, GeodeClassIds::CacheableDoubleArray>;

      /// <summary>
      /// An immutable wrapper for array of floats that can serve
      /// as a distributable object for caching.
      /// </summary>
      using CacheableFloatArray = CacheableArray<native::CacheableArray<float, native::GeodeTypeIds::CacheableFloatArray>, Single, GeodeClassIds::CacheableFloatArray>;

      /// <summary>
      /// An immutable wrapper for array of 16-bit integers that can serve
      /// as a distributable object for caching.
      /// </summary>
      using CacheableInt16Array = CacheableArray<native::CacheableArray<int16_t, native::GeodeTypeIds::CacheableInt16Array>, System::Int16, GeodeClassIds::CacheableInt16Array>;

      /// <summary>
      /// An immutable wrapper for array of 32-bit integers that can serve
      /// as a distributable object for caching.
      /// </summary>
      using CacheableInt32Array = CacheableArray<native::CacheableArray<int32_t, native::GeodeTypeIds::CacheableInt32Array>, System::Int32, GeodeClassIds::CacheableInt32Array>;

      /// <summary>
      /// An immutable wrapper for array of 64-bit integers that can serve
      /// as a distributable object for caching.
      /// </summary>
      using CacheableInt64Array = CacheableArray<native::CacheableArray<int64_t, native::GeodeTypeIds::CacheableInt64Array>, System::Int64, GeodeClassIds::CacheableInt64Array>;

      /// <summary>
      /// An immutable wrapper for array of booleans that can serve
      /// as a distributable object for caching.
      /// </summary>
      using BooleanArray = CacheableArray<native::CacheableArray<bool, native::GeodeTypeIds::BooleanArray>, bool, GeodeClassIds::BooleanArray>;

      /// <summary>
      /// An immutable wrapper for array of 16-bit characters that can serve
      /// as a distributable object for caching.
      /// </summary>
      using CharArray = CacheableArray<native::CacheableArray<char16_t, native::GeodeTypeIds::CharArray>, Char, GeodeClassIds::CharArray>;
    }  // namespace Client
  }  // namespace Geode
}  // namespace Apache

