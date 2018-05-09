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
#include <GeodeTypeIdsImpl.hpp>
#include "../end_native.hpp"

#include "../IGeodeSerializable.hpp"

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
        int m_classId;
        size_t m_objectSize;
      public:

        inline ManagedCacheableKeyGeneric(
          Apache::Geode::Client::IDataSerializable^ managedptr, int hashcode, int classId)
          : m_managedptr(managedptr) {
          m_hashcode = hashcode;
          m_classId = classId;
          m_objectSize = 0;
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
          m_classId = managedptr->ClassId;
          m_objectSize = 0;
        }

        ManagedCacheableKeyGeneric(const ManagedCacheableKeyGeneric&) = delete;
        ManagedCacheableKeyGeneric& operator = (const ManagedCacheableKeyGeneric&) = delete;

        size_t objectSize() const override;

        std::string toString() const override;

        void toData(apache::geode::client::DataOutput& output) const override;

        void fromData(apache::geode::client::DataInput& input) override;

        int32_t getClassId() const override;

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
        /// Note: not using auto_gcroot since it will result in 'Dispose' of the IGeodeSerializable
        /// to be called which is not what is desired when this object is destroyed. Normally this
        /// managed object may be created by the user and will be handled automatically by the GC.
        /// </summary>
        gcroot<Apache::Geode::Client::IDataSerializable^> m_managedptr;
      };

    }  // namespace client
  }  // namespace geode
}  // namespace apache
