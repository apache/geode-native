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
#include <geode/internal/InternalId.hpp>
#include "end_native.hpp"

using namespace System;

namespace Apache
{
  namespace Geode
  {
    namespace Client
    {

      /// <summary>
      /// A mutable generic <see cref="System.Object" /> wrapper that can
      /// serve as a distributable value for caching.
      /// </summary>
      /// <remarks>
      /// <para>
      /// This class can contain any object and uses the
      /// <see cref="System.Xml.Serialization.XmlSerializer" /> to
      /// serialize and deserialize the object. So the user must use the
      /// <c>XmlSerializer</c> attributes to control the serialization/deserialization
      /// of the object (or implement the <see cref="System.Xml.Serialization.IXmlSerializable" />)
      /// to change the serialization/deserialization. However, the latter should
      /// be avoided for efficiency reasons and the user should implement
      /// <see cref="../../ISerializable" /> instead.
      /// </para><para>
      /// The user must keep in mind that the rules that apply to <c>XmlSerializer</c>
      /// would be the rules that apply to this class. For instance the user
      /// cannot pass objects of class implementing or containing
      /// <see cref="System.Collections.IDictionary" /> class, must use
      /// <see cref="System.Xml.Serialization.XmlIncludeAttribute" /> to
      /// mark user-defined types etc.
      /// </para>
      /// </remarks>
      public ref class CacheableObjectXml
        : public ISerializable
      {
      public:
        /// <summary>
        /// Static function to create a new instance from the given object.
        /// </summary>
        /// <remarks>
        /// If the given object is null then this method returns null.
        /// </remarks>
        inline static CacheableObjectXml^ Create(Object^ value)
        {
          return (value != nullptr ? gcnew CacheableObjectXml(value) :
                  nullptr);
        }

        virtual void ToData(DataOutput^ output);

        virtual void FromData(DataInput^ input);

        virtual property System::UInt64 ObjectSize
        {
          virtual System::UInt64 get();
        }

        property int8_t DsCode
        {
          virtual int8_t get()
          {
            return static_cast<int8_t>(apache::geode::client::internal::InternalId::CacheableManagedObjectXml);
          }
        }

        /// <summary>
        /// Gets the object value.
        /// </summary>
        /// <remarks>
        /// The user can modify the object and the changes shall be reflected
        /// immediately in the local cache. For this change to be propagate to
        /// other members of the distributed system, the object needs to be
        /// put into the cache.
        /// </remarks>
        property Object^ Value
        {
          inline Object^ get()
          {
            return m_obj;
          }
        }

        virtual String^ ToString() override
        {
          return (m_obj == nullptr ? nullptr : m_obj->ToString());
        }

        /// <summary>
        /// Factory function to register this class.
        /// </summary>
        static ISerializable^ CreateDeserializable()
        {
          return gcnew CacheableObjectXml(nullptr);
        }

      internal:
        /// <summary>
        /// Allocates a new instance from the given object.
        /// </summary>
        inline CacheableObjectXml(Object^ value)
          : m_obj(value), m_objectSize(0) { }

      private:
        Object^ m_obj;
        System::UInt64 m_objectSize;
      };
    }  // namespace Client
  }  // namespace Geode
}  // namespace Apache

