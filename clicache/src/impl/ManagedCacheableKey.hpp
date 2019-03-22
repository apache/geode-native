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


#include "../geode_defs.hpp"
#include <vcclr.h>
#include "../begin_native.hpp"
#include <geode/CacheableKey.hpp>
#include <geode/DataSerializable.hpp>
#include <geode/internal/DataSerializableFixedId.hpp>
#include <geode/internal/DataSerializablePrimitive.hpp>
#include <geode/internal/DataSerializableInternal.hpp>
#include "../end_native.hpp"

#include "../IDataSerializable.hpp"
#include "../IDataSerializableFixedId.hpp"
#include "../IDataSerializablePrimitive.hpp"
#include "../IDataSerializableInternal.hpp"

using namespace System;

namespace Apache
{
  namespace Geode
  {
    namespace Client
    {
      ref class Cache;
    }  // namespace Client
  }  // namespace Geode
}  // namespace Apache

namespace apache
{
  namespace geode
  {
    namespace client
    {
      namespace native = apache::geode::client;

      /// <summary>
      /// Wraps the managed <see cref="Apache.Geode.Client.IDataSerializable" />
      /// object and implements the native <c>apache::geode::client::CacheableKey</c> and 
      /// <c>apache::geode::client::DataSerializable</c> interfaces.
      /// </summary>
      class ManagedCacheableKeyGeneric
        : public apache::geode::client::CacheableKey, public native::DataSerializable
      {
      private:
        int m_hashcode;
        size_t m_objectSize;
      public:

        inline ManagedCacheableKeyGeneric(
          Apache::Geode::Client::IDataSerializable^ managedptr, int hashcode)
          : m_managedptr(managedptr) {
          m_hashcode = hashcode;
          m_objectSize = 0;
          msclr::interop::marshal_context context;
        }
        /// <summary>
        /// Constructor to initialize with the provided managed object.
        /// </summary>
        /// <param name="managedptr">
        /// The managed object.
        /// </param>
        inline ManagedCacheableKeyGeneric(Apache::Geode::Client::IDataSerializable^ managedptr)
          : m_managedptr(managedptr) {
          m_hashcode = 0;
          m_objectSize = 0;
          msclr::interop::marshal_context context;
        }

        ManagedCacheableKeyGeneric(const ManagedCacheableKeyGeneric&) = delete;
        ManagedCacheableKeyGeneric& operator = (const ManagedCacheableKeyGeneric&) = delete;

        size_t objectSize() const override;

        std::string toString() const override;

        void toData(apache::geode::client::DataOutput& output) const override;

        void fromData(apache::geode::client::DataInput& input) override;

        bool operator == (const CacheableKey& other) const override;

        virtual bool operator == (const ManagedCacheableKeyGeneric& other) const;

        int32_t hashcode() const override;

        inline Apache::Geode::Client::IDataSerializable^ ptr() const
        {
          return m_managedptr;
        }



      private:

        /// <summary>
        /// Using gcroot to hold the managed delegate pointer (since it cannot be stored directly).
        /// Note: not using auto_gcroot since it will result in 'Dispose' of the ISerializable
        /// to be called which is not what is desired when this object is destroyed. Normally this
        /// managed object may be created by the user and will be handled automatically by the GC.
        /// </summary>
        gcroot<Apache::Geode::Client::IDataSerializable^> m_managedptr;
      };

      class ManagedDataSerializablePrimitive
        : public native::internal::DataSerializablePrimitive , public native::CacheableKey
      {
      public:

        inline ManagedDataSerializablePrimitive(
          Apache::Geode::Client::IDataSerializablePrimitive^ managedptr)
          : m_managedptr(managedptr) {
        }

        ManagedDataSerializablePrimitive(const ManagedDataSerializablePrimitive&) = delete;
        ManagedDataSerializablePrimitive operator = (const ManagedDataSerializablePrimitive&) = delete;

        size_t objectSize() const override { return m_managedptr->ObjectSize; }

        std::string toString() const override;

        void toData(DataOutput& output) const override;

        void fromData(DataInput& input) override;

        native::internal::DSCode getDsCode() const override { return static_cast<native::internal::DSCode>(m_managedptr->DsCode); }

        bool operator == (const CacheableKey& other) const override;

        int32_t hashcode() const override;

        inline Apache::Geode::Client::IDataSerializablePrimitive^ ptr() const
        {
          return m_managedptr;
        }


      private:
        gcroot<Apache::Geode::Client::IDataSerializablePrimitive^> m_managedptr;
      };

      class ManagedDataSerializableInternal
        : public native::internal::DataSerializableInternal
      {
      public:

        inline ManagedDataSerializableInternal(
          Apache::Geode::Client::IDataSerializableInternal^ managedptr)
          : m_managedptr(managedptr) {
        }

        ManagedDataSerializableInternal(const ManagedDataSerializableInternal&) = delete;
        ManagedDataSerializableInternal& operator = (const ManagedDataSerializableInternal&) = delete;

        size_t objectSize() const override { return 0; }

        std::string toString() const override;

        void toData(DataOutput& output) const override;

        void fromData(DataInput& input) override;

        inline Apache::Geode::Client::IDataSerializableInternal^ ptr() const
        {
          return m_managedptr;
        }


      private:
        gcroot<Apache::Geode::Client::IDataSerializableInternal^> m_managedptr;
      };

      class ManagedDataSerializableFixedId
        : public native::internal::DataSerializableFixedId
      {
      public:

        inline ManagedDataSerializableFixedId(
          Apache::Geode::Client::IDataSerializableFixedId^ managedptr)
          : m_managedptr(managedptr) {
        }

        ManagedDataSerializableFixedId(const ManagedDataSerializableFixedId&) = delete;
        ManagedDataSerializableFixedId& operator = (const ManagedDataSerializableFixedId&) = delete;

        size_t objectSize() const override { return 0; }

        std::string toString() const override;

        void toData(DataOutput& output) const override;

        void fromData(DataInput& input) override;

        native::internal::DSFid getDSFID() const override { return static_cast<native::internal::DSFid>(m_managedptr->DSFID); }

        inline Apache::Geode::Client::IDataSerializableFixedId^ ptr() const
        {
          return m_managedptr;
        }


      private:
        gcroot<Apache::Geode::Client::IDataSerializableFixedId^> m_managedptr;
      };
    }  // namespace client
  }  // namespace geode
}  // namespace apache
