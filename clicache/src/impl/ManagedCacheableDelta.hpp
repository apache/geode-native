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

#include <geode/Delta.hpp>
#include <vcclr.h>

#include "../geode_defs.hpp"
#include "../begin_native.hpp"
#include <geode/CacheableKey.hpp>
#include <geode/Delta.hpp>
#include <geode/DataSerializable.hpp>
#include "../end_native.hpp"
#include "../IDelta.hpp"
#include "../IDataSerializable.hpp"

using namespace System;

namespace apache
{
  namespace geode
  {
    namespace client
    {

      /// <summary>
      /// Wraps the managed <see cref="Apache.Geode.Client.IDelta" />
      /// object and implements the native <c>apache::geode::client::CacheableKey</c> interface.
      /// </summary>
      class ManagedCacheableDeltaGeneric
        : public apache::geode::client::CacheableKey,
          public apache::geode::client::Delta,
          public apache::geode::client::DataSerializable
      {
        public:
          /// <summary>
          /// Constructor to initialize with the provided managed object.
          /// </summary>
          /// <param name="managedptr">
          /// The managed object.
          /// </param>
          inline ManagedCacheableDeltaGeneric(
            Apache::Geode::Client::IDelta^ managedptr)
            : Delta(), m_managedptr(managedptr)
          {
            m_managedSerializableptr = dynamic_cast<Apache::Geode::Client::IDataSerializable^>(managedptr);
            m_objectSize = 0;
          }

          inline ManagedCacheableDeltaGeneric(
            Apache::Geode::Client::IDelta^ managedptr, int hashcode, int classId)
            : Delta(),  m_managedptr(managedptr) 
          {
            m_hashcode = hashcode;
            m_managedSerializableptr = dynamic_cast<Apache::Geode::Client::IDataSerializable^> (managedptr);
            m_objectSize = 0;
          }

          size_t objectSize() const override;

          void toData(apache::geode::client::DataOutput& output) const override;

          void fromData(apache::geode::client::DataInput& input) override;

          void toDelta(apache::geode::client::DataOutput& output) const override;

          void fromDelta(apache::geode::client::DataInput& input) override;

          bool hasDelta() const override;

          std::shared_ptr<apache::geode::client::Delta> clone() const override;

          int32_t hashcode() const override;

          bool operator == (const CacheableKey& other) const override;

          virtual bool operator == (const ManagedCacheableDeltaGeneric& other) const;

          inline Apache::Geode::Client::IDelta^ ptr() const
          {
            return m_managedptr;
          }

          ManagedCacheableDeltaGeneric(const ManagedCacheableDeltaGeneric&) = delete;
          ManagedCacheableDeltaGeneric& operator = (const ManagedCacheableDeltaGeneric&) = delete;

        private:
          int m_hashcode;
          size_t m_objectSize;

          /// <summary>
          /// Using gcroot to hold the managed delegate pointer (since it cannot be stored directly).
          /// Note: not using auto_gcroot since it will result in 'Dispose' of the IDelta
          /// to be called which is not what is desired when this object is destroyed. Normally this
          /// managed object may be created by the user and will be handled automatically by the GC.
          /// </summary>
          gcroot<Apache::Geode::Client::IDelta^> m_managedptr;
          gcroot<Apache::Geode::Client::IDataSerializable^> m_managedSerializableptr;
      };

    }  // namespace client
  }  // namespace geode
}  // namespace apache
