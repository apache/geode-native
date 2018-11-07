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

#include <vcclr.h>

#include "../begin_native.hpp"
#include <geode/CacheableKey.hpp>
#include <geode/Delta.hpp>
#include <geode/PdxSerializable.hpp>
#include "../end_native.hpp"

#include "../geode_defs.hpp"
#include "../IPdxSerializable.hpp"
#include "../IDelta.hpp"

using namespace System;
using namespace apache::geode::client;

namespace Apache
{
  namespace Geode
  {
    namespace Client
    {
      interface class IPdxSerializable;
    }  // namespace Client
  }  // namespace Geode
}  // namespace Apache


namespace apache
{
  namespace geode
  {
    namespace client
    {
      /// <summary>
      /// Wraps the managed <see cref="Com.Vmware.Cache.IPdxSerializable" />
      /// object and implements the native <c>apache::geode::client::CacheableKey</c> interface.
      /// </summary>
      class PdxManagedCacheableKey
        : public PdxSerializable,
          public Delta
      {
        public:
          inline PdxManagedCacheableKey()
            : Delta(), m_managedptr(nullptr), m_hashcode(0), m_objectSize(0), m_managedDeltaptr(nullptr) {}

          inline PdxManagedCacheableKey(
            Apache::Geode::Client::IPdxSerializable^ managedptr, int hashcode)
            :Delta(), m_managedptr(managedptr), m_objectSize(0) 
          {
            m_className = marshal_as<std::string>(m_managedptr->GetType()->Name);
            m_hashcode = hashcode;
            m_managedDeltaptr = dynamic_cast<Apache::Geode::Client::IDelta^>(managedptr);
          }

          /// <summary>
          /// Constructor to initialize with the provided managed object.
          /// </summary>
          /// <param name="managedptr">
          /// The managed object.
          /// </param>
          inline PdxManagedCacheableKey(
            Apache::Geode::Client::IPdxSerializable^ managedptr)
            : PdxManagedCacheableKey(managedptr, 0) {}

          void toData(PdxWriter& output) const override;

          void fromData(PdxReader& input) override;

          const std::string& getClassName() const override;

          void toDelta(DataOutput& output) const override;

          void fromDelta(DataInput& input) override;

          bool hasDelta() const override;

          std::shared_ptr<apache::geode::client::Delta> clone() const override;

          size_t objectSize() const override;

          std::string toString() const override;

          bool operator == (const CacheableKey& other) const override;
          
          virtual bool operator == (const PdxManagedCacheableKey& other) const;

          System::Int32 hashcode() const override;

          inline Apache::Geode::Client::IPdxSerializable^ ptr() const
          {
            return m_managedptr;
          }

          static Serializable* CreateDeserializable()
          {
            throw "Not implemented";
          }

          PdxManagedCacheableKey(const PdxManagedCacheableKey&) = delete;
          PdxManagedCacheableKey& operator = (const PdxManagedCacheableKey&) = delete;

        private:
          int m_hashcode;
          size_t m_objectSize;
          std::string m_className;

          /// <summary>
          /// Using gcroot to hold the managed delegate pointer (since it cannot be stored directly).
          /// Note: not using auto_gcroot since it will result in 'Dispose' of the ISerializable
          /// to be called which is not what is desired when this object is destroyed. Normally this
          /// managed object may be created by the user and will be handled automatically by the GC.
          /// </summary>
          gcroot<Apache::Geode::Client::IPdxSerializable^> m_managedptr;
          gcroot<Apache::Geode::Client::IDelta^> m_managedDeltaptr;
      };
    }  // namespace client
  }  // namespace geode
}  // namespace apache
