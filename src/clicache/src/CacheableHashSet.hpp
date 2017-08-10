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
#include "ExceptionTypes.hpp"
#include "impl/PdxInstanceImpl.hpp"
#include "native_shared_ptr.hpp"
#include "native_unique_ptr.hpp"

using namespace System;
using namespace System::Collections::Generic;
#pragma managed

namespace Apache
{
  namespace Geode
  {
    namespace Client
    {

      namespace Internal
      {
        /// <summary>
        /// A mutable <c>ICacheableKey</c> hash set wrapper that can serve as
        /// a distributable object for caching.
        /// </summary>
        template <System::UInt32 TYPEID, typename HSTYPE>
        public ref class CacheableHashSetType
          : public Serializable, public ICollection<Object^>
        {
        public:

          virtual void ToData(DataOutput^ output) override
          {
            output->WriteArrayLen(this->Count);

            auto set = static_cast<HSTYPE*>(m_nativeptr->get());
            for (const auto& iter : *set) {
              auto key = Serializable::GetManagedValueGeneric<Object^>(iter);
              output->WriteObject(key);
            }

            GC::KeepAlive(this);
          }

          virtual IGeodeSerializable^ FromData(DataInput^ input) override
          {
            int len = input->ReadArrayLen();
            if (len > 0)
            {
              for (int i = 0; i < len; i++)
              {
                Object^ key = (input->ReadObject());
                this->Add(key);
              }
            }
            return this;
          }

          virtual property System::UInt32 ObjectSize
          {
            virtual System::UInt32 get() override
            {
              System::UInt32 size = 0;
              for each (Object^ key in this) {
                if (key != nullptr)
                  //size += key->ObjectSize; 
                  //TODO:: how should we do this now
                  size += 1;
              }
              return size;
            }
          }

          virtual int GetHashCode() override
          {
            IEnumerator<Object^>^ ie = GetEnumerator();

            int h = 0;
            while (ie->MoveNext() == true)
            {
              h = h + PdxInstanceImpl::deepArrayHashCode(ie->Current);
            }
            return h;
          }

          virtual bool Equals(Object^ other)override
          {
            if (other == nullptr)
              return false;

            CacheableHashSetType^ otherCHST = dynamic_cast<CacheableHashSetType^>(other);

            if (otherCHST == nullptr)
              return false;

            if (Count != otherCHST->Count)
              return false;

            IEnumerator<Object^>^ ie = GetEnumerator();

            while (ie->MoveNext() == true)
            {
              if (otherCHST->Contains(ie->Current))
                return true;
              else
                return false;
            }

            return true;
          }

          /// <summary>
          /// Enumerator for <c>CacheableHashSet</c> class.
          /// </summary>
          ref class Enumerator sealed
            : public IEnumerator<Object^>
          {
          public:
            // Region: IEnumerator<ICacheableKey^> Members

            /// <summary>
            /// Gets the element in the collection at the current
            /// position of the enumerator.
            /// </summary>
            /// <returns>
            /// The element in the collection at the current position
            /// of the enumerator.
            /// </returns>
            property Object^ Current
            {
              virtual Object^ get() =
                IEnumerator<Object^>::Current::get
                {
                  if (!m_started) {
                    throw gcnew System::InvalidOperationException(
                      "Call MoveNext first.");
                  }
                auto ret = Serializable::GetManagedValueGeneric<Object^>(*(*(m_nativeptr->get())));
                GC::KeepAlive(this);
                return ret;
              }
            }

            // End Region: IEnumerator<ICacheableKey^> Members

            // Region: IEnumerator Members

            /// <summary>
            /// Advances the enumerator to the next element of the collection.
            /// </summary>
            /// <returns>
            /// true if the enumerator was successfully advanced to the next
            /// element; false if the enumerator has passed the end of
            /// the collection.
            /// </returns>
            virtual bool MoveNext()
            {
              auto nptr = m_nativeptr->get();
              bool isEnd = static_cast<HSTYPE*>(m_set->m_nativeptr->get())->end() == *nptr;
              if (!m_started) {
                m_started = true;
              }
              else {
                if (!isEnd) {
                  (*nptr)++;
                  isEnd = static_cast<HSTYPE*>(m_set->m_nativeptr->get())->end() == *nptr;
                }
              }
              GC::KeepAlive(this);
              return !isEnd;
            }

            /// <summary>
            /// Sets the enumerator to its initial position, which is before
            /// the first element in the collection.
            /// </summary>
            virtual void Reset()
            {
              try
              {
                m_nativeptr = gcnew native_unique_ptr<typename HSTYPE::iterator>(
                    std::make_unique<typename HSTYPE::iterator>(
                    static_cast<HSTYPE*>(m_set->m_nativeptr->get())->begin()));
              }
              finally
              {
                GC::KeepAlive(m_nativeptr);
              }
              m_started = false;
            }

            !Enumerator() {}
            ~Enumerator() {}

            // End Region: IEnumerator Members

          internal:
            /// <summary>
            /// Internal constructor to wrap a native object pointer
            /// </summary>
            /// <param name="nativeptr">The native object pointer</param>
            inline Enumerator(CacheableHashSetType<TYPEID, HSTYPE>^ set)
                              : m_set(set) {
              Reset();
            }

          private:
            // Region: IEnumerator Members

            /// <summary>
            /// Gets the current element in the collection.
            /// </summary>
            /// <returns>
            ///     The current element in the collection.
            /// </returns>
            /// <exception cref="System.InvalidOperationException">
            /// The enumerator is positioned before the first element of
            /// the collection or after the last element.
            /// </exception>
            property Object^ ICurrent
            {
              virtual Object^ get() sealed =
                System::Collections::IEnumerator::Current::get
              {
                return Current;
              }
            }

            // End Region: IEnumerator Members

            bool m_started;

            CacheableHashSetType<TYPEID, HSTYPE>^ m_set;

            native_unique_ptr<typename HSTYPE::iterator>^ m_nativeptr;
          };

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
          /// Get the largest possible size of the <c>CacheableHashSet</c>.
          /// </summary>
          property System::Int32 MaxSize
          {
            inline System::Int32 get()
            {
              try
              {
                return static_cast<HSTYPE*>(m_nativeptr->get())->max_size();
              }
              finally
              {
                GC::KeepAlive(m_nativeptr);
              }
            }
          }

          /// <summary>
          /// True if the <c>CacheableHashSet</c>'s size is 0.
          /// </summary>
          property bool IsEmpty
          {
            inline bool get()
            {
              try
              {
                return static_cast<HSTYPE*>(m_nativeptr->get())->empty();
              }
              finally
              {
                GC::KeepAlive(m_nativeptr);
              }
            }
          }

          /// <summary>
          /// Get the number of buckets used by the HashSet.
          /// </summary>
          property System::Int32 BucketCount
          {
            inline System::Int32 get()
            {
              try
              {
                return static_cast<HSTYPE*>(m_nativeptr->get())->bucket_count();
              }
              finally
              {
                GC::KeepAlive(m_nativeptr);
              }
            }
          }

          /// <summary>
          /// Increases the bucket count to at least <c>size</c> elements.
          /// </summary>
          /// <param name="size">The new size of the HashSet.</param>
          virtual void Resize(System::Int32 size) sealed
          {
            try
            {
              static_cast<HSTYPE*>(m_nativeptr->get())->reserve(size);
            }
            finally
            {
              GC::KeepAlive(m_nativeptr);
            }
          }

          /// <summary>
          /// Swap the contents of this <c>CacheableHashSet</c>
          /// with the given one.
          /// </summary>
          /// <param name="other">
          /// The other CacheableHashSet to use for swapping.
          /// </param>
          virtual void Swap(CacheableHashSetType<TYPEID, HSTYPE>^ other) sealed
          {
            try
            {
              if (other != nullptr) {
                static_cast<HSTYPE*>(m_nativeptr->get())->swap(
                  *static_cast<HSTYPE*>(other->m_nativeptr->get()));
              }
            }
            finally
            {
              GC::KeepAlive(m_nativeptr);
              GC::KeepAlive(other->m_nativeptr);
            }
          }

          // Region: ICollection<ICacheableKey^> Members

          /// <summary>
          /// Adds an item to the <c>CacheableHashSet</c>.
          /// </summary>
          /// <param name="item">
          /// The object to add to the collection.
          /// </param>
          virtual void Add(Object^ item)
          {
            _GF_MG_EXCEPTION_TRY2/* due to auto replace */

            try
            {
              static_cast<HSTYPE*>(m_nativeptr->get())->insert(Serializable::GetUnmanagedValueGeneric(item, nullptr));
            }
            finally
            {
              GC::KeepAlive(m_nativeptr);
            }

            _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */
          }

          /// <summary>
          /// Removes all items from the <c>CacheableHashSet</c>.
          /// </summary>
          virtual void Clear()
          {
            try
            {
              static_cast<HSTYPE*>(m_nativeptr->get())->clear();
            }
            finally
            {
              GC::KeepAlive(m_nativeptr);
            }
          }

          /// <summary>
          /// Determines whether the <c>CacheableHashSet</c> contains
          /// a specific value.
          /// </summary>
          /// <param name="item">
          /// The object to locate in the <c>CacheableHashSet</c>.
          /// </param>
          /// <returns>
          /// true if item is found in the <c>CacheableHashSet</c>;
          /// otherwise false.
          /// </returns>
          virtual bool Contains(Object^ item)
          {
            try
            {
              return static_cast<HSTYPE*>(m_nativeptr->get())->find(Serializable::GetUnmanagedValueGeneric(item, nullptr)) != static_cast<HSTYPE*>(m_nativeptr->get())->end();
            }
            finally
            {
              GC::KeepAlive(m_nativeptr);
            }
          }

          /// <summary>
          /// Copies the elements of the <c>CacheableHashSet</c> to an
          /// <c>System.Array</c>, starting at a particular
          /// <c>System.Array</c> index.
          /// </summary>
          /// <param name="array">
          /// The one-dimensional System.Array that is the destination of the
          /// elements copied from <c>CacheableHashSet</c>. The
          /// <c>System.Array</c> must have zero-based indexing.
          /// </param>
          /// <param name="arrayIndex">
          /// The zero-based index in array at which copying begins.
          /// </param>
          /// <exception cref="IllegalArgumentException">
          /// arrayIndex is less than 0 or array is null.
          /// </exception>
          /// <exception cref="OutOfRangeException">
          /// arrayIndex is equal to or greater than the length of array.
          /// -or-The number of elements in the source <c>CacheableHashSet</c>
          /// is greater than the available space from arrayIndex to the end
          /// of the destination array.
          /// </exception>
          virtual void CopyTo(array<Object^>^ array, System::Int32 arrayIndex)
          {
            if (array == nullptr || arrayIndex < 0) {
              throw gcnew IllegalArgumentException("CacheableHashSet.CopyTo():"
                                                   " array is null or array index is less than zero");
            }

            auto set = static_cast<HSTYPE*>(m_nativeptr->get());
            System::Int32 index = arrayIndex;

            if (arrayIndex >= array->Length ||
                array->Length < (arrayIndex + (System::Int32)set->size())) {
              throw gcnew OutOfRangeException("CacheableHashSet.CopyTo():"
                                              " array index is beyond the HashSet or length of given "
                                              "array is less than that required to copy all the "
                                              "elements from HashSet");
            }
            for (const auto& iter : *set) {
              array[index++] = Serializable::GetManagedValueGeneric<Object^>(iter);
            }

            GC::KeepAlive(m_nativeptr);
          }

          /// <summary>
          /// Gets the number of elements contained in the
          /// <c>CacheableHashSet</c>.
          /// </summary>
          virtual property System::Int32 Count
          {
            virtual System::Int32 get()
            {
              try
              {
                return static_cast<HSTYPE*>(m_nativeptr->get())->size();
              }
              finally
              {
                GC::KeepAlive(m_nativeptr);
              }
            }
          }

          /// <summary>
          /// Removes the first occurrence of a specific object from the
          /// <c>CacheableHashSet</c>.
          /// </summary>
          /// <param name="item">
          /// The object to remove from the <c>CacheableHashSet</c>.
          /// </param>
          /// <returns>
          /// true if item was successfully removed from the
          /// <c>CacheableHashSet</c>; otherwise, false. This method also
          /// returns false if item is not found in the original
          /// <c>CacheableHashSet</c>.
          /// </returns>
          virtual bool Remove(Object^ item)
          {
            try
            {
              return (static_cast<HSTYPE*>(m_nativeptr->get())->erase(Serializable::GetUnmanagedValueGeneric(item, nullptr)) > 0);
            }
            finally
            {
              GC::KeepAlive(m_nativeptr);
            }
          }

          /// <summary>
          /// Gets a value indicating whether the collection is read-only.
          /// </summary>
          /// <returns>
          /// always false for <c>CacheableHashSet</c>
          /// </returns>
          virtual property bool IsReadOnly
          {
            virtual bool get()
            {
              return false;
            }
          }

          // End Region: ICollection<ICacheableKey^> Members

          // Region: IEnumerable<ICacheableKey^> Members

          /// <summary>
          /// Returns an enumerator that iterates through the
          /// <c>CacheableHashSet</c>.
          /// </summary>
          /// <returns>
          /// A <c>System.Collections.Generic.IEnumerator</c> that
          /// can be used to iterate through the <c>CacheableHashSet</c>.
          /// </returns>
          virtual IEnumerator<Object^>^ GetEnumerator()
          {
            return gcnew Enumerator(this);
          }

          // End Region: IEnumerable<ICacheableKey^> Members

        internal:
          /// <summary>
          /// Factory function to register wrapper
          /// </summary>
          static IGeodeSerializable^ Create(apache::geode::client::Serializable* obj)
          {
            return (obj != NULL ?
                    gcnew CacheableHashSetType<TYPEID, HSTYPE>(obj) : nullptr);
          }

        private:
          // Region: IEnumerable Members

          /// <summary>
          /// Returns an enumerator that iterates through a collection.
          /// </summary>
          /// <returns>
          /// An <c>System.Collections.IEnumerator</c> object that can be used
          /// to iterate through the collection.
          /// </returns>
          virtual System::Collections::IEnumerator^ GetIEnumerator() sealed =
            System::Collections::IEnumerable::GetEnumerator
          {
            return GetEnumerator();
          }

            // End Region: IEnumerable Members

        protected:
          /// <summary>
          /// Private constructor to wrap a native object pointer
          /// </summary>
          /// <param name="nativeptr">The native object pointer</param>
          inline CacheableHashSetType<TYPEID, HSTYPE>(apache::geode::client::SerializablePtr nativeptr)
            : Serializable(nativeptr) { }

          /// <summary>
          /// Allocates a new empty instance.
          /// </summary>
          inline CacheableHashSetType<TYPEID, HSTYPE>()
            : Serializable(std::shared_ptr<HSTYPE>(static_cast<HSTYPE*>(HSTYPE::createDeserializable())))
          { }

          /// <summary>
          /// Allocates a new empty instance with given initial size.
          /// </summary>
          /// <param name="size">The initial size of the HashSet.</param>
          inline CacheableHashSetType<TYPEID, HSTYPE>(System::Int32 size)
            : Serializable(HSTYPE::create(size))
          { }
        };
      }

#define _GFCLI_CACHEABLEHASHSET_DEF_GENERIC(m, HSTYPE)                               \
	public ref class m : public Internal::CacheableHashSetType<Apache::Geode::Client::GeodeClassIds::m, HSTYPE>      \
            {                                                                       \
      public:                                                                 \
        /** <summary>
      *  Allocates a new empty instance.
      *  </summary>
      */                                                                   \
      inline m()                                                            \
      : Internal::CacheableHashSetType<Apache::Geode::Client::GeodeClassIds::m, HSTYPE>() {}                      \
      \
      /** <summary>
       *  Allocates a new instance with the given size.
       *  </summary>
       *  <param name="size">the intial size of the new instance</param>
       */                                                                   \
       inline m(System::Int32 size)                                                 \
       : Internal::CacheableHashSetType<Apache::Geode::Client::GeodeClassIds::m, HSTYPE>(size) {}                  \
       \
       /** <summary>
        *  Static function to create a new empty instance.
        *  </summary>
        */                                                                   \
        inline static m^ Create()                                             \
      {                                                                     \
      return gcnew m();                                                   \
      }                                                                     \
      \
      /** <summary>
       *  Static function to create a new instance with the given size.
       *  </summary>
       */                                                                   \
       inline static m^ Create(System::Int32 size)                                  \
      {                                                                     \
      return gcnew m(size);                                               \
      }                                                                     \
      \
      /* <summary>
       * Factory function to register this class.
       * </summary>
       */                                                                   \
       static IGeodeSerializable^ CreateDeserializable()                        \
      {                                                                     \
      return gcnew m();                                                   \
      }                                                                     \
      \
            internal:                                                               \
              static IGeodeSerializable^ Create(apache::geode::client::SerializablePtr obj)            \
      {                                                                     \
      return gcnew m(obj);                                                \
      }                                                                     \
      \
            private:                                                                \
              inline m(apache::geode::client::SerializablePtr nativeptr)                            \
              : Internal::CacheableHashSetType<Apache::Geode::Client::GeodeClassIds::m, HSTYPE>(nativeptr) { }             \
      };

      /// <summary>
      /// A mutable <c>ICacheableKey</c> hash set wrapper that can serve as
      /// a distributable object for caching.
      /// </summary>
      _GFCLI_CACHEABLEHASHSET_DEF_GENERIC(CacheableHashSet,
                                          apache::geode::client::CacheableHashSet);

      /// <summary>
      /// A mutable <c>ICacheableKey</c> hash set wrapper that can serve as
      /// a distributable object for caching. This is provided for compability
      /// with java side though is functionally identical to
      /// <c>CacheableHashSet</c> i.e. does not provide the linked semantics of
      /// java <c>LinkedHashSet</c>.
      /// </summary>
      _GFCLI_CACHEABLEHASHSET_DEF_GENERIC(CacheableLinkedHashSet,
                                          apache::geode::client::CacheableLinkedHashSet);
    }  // namespace Client
  }  // namespace Geode
}  // namespace Apache

