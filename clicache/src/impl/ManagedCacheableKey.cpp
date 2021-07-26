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


#include "../begin_native.hpp"
#include "../end_native.hpp"

#include "../ICacheableKey.hpp"
#include "ManagedCacheableKey.hpp"
#include "../DataInput.hpp"
#include "../DataOutput.hpp"
#include "../CacheableString.hpp"
#include "../ExceptionTypes.hpp"
#include "../Log.hpp"
#include "../String.hpp"
#include "CacheResolver.hpp"

using namespace System;

namespace apache
{
  namespace geode
  {
    namespace client
    {

      void ManagedCacheableKeyGeneric::toData(apache::geode::client::DataOutput& output) const
      {
        try {
          System::UInt32 pos = (int)output.getBufferLength();
          //Apache::Geode::Client::Log::Debug("ManagedCacheableKeyGeneric::toData");
          auto cache = CacheResolver::Lookup(output.getCache());
          Apache::Geode::Client::DataOutput mg_output(&output, true, cache);
          m_managedptr->ToData(%mg_output);
          //this will move the cursor in c++ layer
          mg_output.WriteBytesToUMDataOutput();

          ManagedCacheableKeyGeneric* tmp = const_cast<ManagedCacheableKeyGeneric*>(this);
          tmp->m_objectSize = (int)(output.getBufferLength() - pos);
        }
        catch (Apache::Geode::Client::GeodeException^ ex) {
          ex->ThrowNative();
        }
        catch (System::Exception^ ex) {
          Apache::Geode::Client::GeodeException::ThrowNative(ex);
        }
      }

      void ManagedCacheableKeyGeneric::fromData(apache::geode::client::DataInput& input)
      {
        try {
          auto pos = input.getBytesRead();
          auto cache = CacheResolver::Lookup(input.getCache());
          Apache::Geode::Client::DataInput mg_input(&input, true, cache);
          m_managedptr->FromData(%mg_input);

          //this will move the cursor in c++ layer
          input.advanceCursor(mg_input.BytesReadInternally);
          m_objectSize = input.getBytesRead() - pos;
        }
        catch (Apache::Geode::Client::GeodeException^ ex) {
          ex->ThrowNative();
        }
        catch (System::Exception^ ex) {
          Apache::Geode::Client::GeodeException::ThrowNative(ex);
        }
      }
      
      size_t ManagedCacheableKeyGeneric::objectSize() const
      {
        try {
          auto ret = m_managedptr->ObjectSize;
          if (ret > m_objectSize)
            return ret;
          else
            return m_objectSize;
        }
        catch (Apache::Geode::Client::GeodeException^ ex) {
          ex->ThrowNative();
        }
        catch (System::Exception^ ex) {
          Apache::Geode::Client::GeodeException::ThrowNative(ex);
        }
        return 0;
      }

      std::string ManagedCacheableKeyGeneric::toString() const
      {
        try {
          return Apache::Geode::Client::to_utf8(m_managedptr->ToString());
        }
        catch (Apache::Geode::Client::GeodeException^ ex) {
          ex->ThrowNative();
        }
        catch (System::Exception^ ex) {
          Apache::Geode::Client::GeodeException::ThrowNative(ex);
        }
        return nullptr;
      }

      bool ManagedCacheableKeyGeneric::operator ==(const apache::geode::client::CacheableKey& other) const
      {
        if (auto&& otherKey = dynamic_cast<const ManagedCacheableKeyGeneric*>(&other))
        {
          return this->operator==(*otherKey);
        }

        return false;
      }

      bool ManagedCacheableKeyGeneric::operator ==(const ManagedCacheableKeyGeneric& other) const
      {
        try {
          return dynamic_cast<Apache::Geode::Client::ICacheableKey^>(ptr())->Equals(
            dynamic_cast<Apache::Geode::Client::ICacheableKey^>(other.ptr()));
        }
        catch (Apache::Geode::Client::GeodeException^ ex) {
          ex->ThrowNative();
        }
        catch (System::Exception^ ex) {
          Apache::Geode::Client::GeodeException::ThrowNative(ex);
        }
        return false;
      }

      System::Int32 ManagedCacheableKeyGeneric::hashcode() const
      {
        if (m_hashcode != 0)
          return m_hashcode;
        try {

          ManagedCacheableKeyGeneric* tmp = const_cast<ManagedCacheableKeyGeneric*>(this);
          tmp->m_hashcode = ((Apache::Geode::Client::ICacheableKey^)
                             (Apache::Geode::Client::ISerializable^)m_managedptr)
                             ->GetHashCode();
          return m_hashcode;
        }
        catch (Apache::Geode::Client::GeodeException^ ex) {
          ex->ThrowNative();
        }
        catch (System::Exception^ ex) {
          Apache::Geode::Client::GeodeException::ThrowNative(ex);
        }
        return 0;
      }

      std::string ManagedDataSerializablePrimitive::toString() const {
        return Apache::Geode::Client::to_utf8(m_managedptr->ToString());
      }

      void ManagedDataSerializablePrimitive::toData(apache::geode::client::DataOutput& output) const {
        try {
          auto cache = CacheResolver::Lookup(output.getCache());
          Apache::Geode::Client::DataOutput mg_output(&output, true, cache);
          m_managedptr->ToData(%mg_output);
          mg_output.WriteBytesToUMDataOutput();
        }
        catch (Apache::Geode::Client::GeodeException^ ex) {
          ex->ThrowNative();
        }
        catch (System::Exception^ ex) {
          Apache::Geode::Client::GeodeException::ThrowNative(ex);
        }
      }

      void ManagedDataSerializablePrimitive::fromData(apache::geode::client::DataInput& input) {
        try {
          auto cache = CacheResolver::Lookup(input.getCache());
          Apache::Geode::Client::DataInput mg_input(&input, true, cache);
          m_managedptr->FromData(%mg_input);
          input.advanceCursor(mg_input.BytesReadInternally);
        }
        catch (Apache::Geode::Client::GeodeException^ ex) {
          ex->ThrowNative();
        }
        catch (System::Exception^ ex) {
          Apache::Geode::Client::GeodeException::ThrowNative(ex);
        }
      }

      bool ManagedDataSerializablePrimitive::operator ==(const apache::geode::client::CacheableKey& other) const
      {
        if (auto&& otherKey = dynamic_cast<const ManagedDataSerializablePrimitive*>(&other))
        {
          return m_managedptr->Equals(otherKey->m_managedptr);
        }

        return false;
      }

      System::Int32 ManagedDataSerializablePrimitive::hashcode() const
      {
        try {
          return m_managedptr->GetHashCode();
        }
        catch (Apache::Geode::Client::GeodeException^ ex) {
          ex->ThrowNative();
        }
        catch (System::Exception^ ex) {
          Apache::Geode::Client::GeodeException::ThrowNative(ex);
        }
        return 0;
      }

      std::string ManagedDataSerializableInternal::toString() const {
        return Apache::Geode::Client::to_utf8(m_managedptr->ToString());
      }

      void ManagedDataSerializableInternal::toData(apache::geode::client::DataOutput& output) const {
        try {
          auto cache = CacheResolver::Lookup(output.getCache());
          Apache::Geode::Client::DataOutput mg_output(&output, true, cache);
          m_managedptr->ToData(%mg_output);
          mg_output.WriteBytesToUMDataOutput();
        }
        catch (Apache::Geode::Client::GeodeException^ ex) {
          ex->ThrowNative();
        }
        catch (System::Exception^ ex) {
          Apache::Geode::Client::GeodeException::ThrowNative(ex);
        }
      }

      void ManagedDataSerializableInternal::fromData(apache::geode::client::DataInput& input) {
        try {
          auto cache = CacheResolver::Lookup(input.getCache());
          Apache::Geode::Client::DataInput mg_input(&input, true, cache);
          m_managedptr->FromData(%mg_input);
          input.advanceCursor(mg_input.BytesReadInternally);
        }
        catch (Apache::Geode::Client::GeodeException^ ex) {
          ex->ThrowNative();
        }
        catch (System::Exception^ ex) {
          Apache::Geode::Client::GeodeException::ThrowNative(ex);
        }
      }

      std::string ManagedDataSerializableFixedId::toString() const {
        return Apache::Geode::Client::to_utf8(m_managedptr->ToString());
      }

      void ManagedDataSerializableFixedId::toData(apache::geode::client::DataOutput& output) const {
        try {
          auto cache = CacheResolver::Lookup(output.getCache());
          Apache::Geode::Client::DataOutput mg_output(&output, true, cache);
          m_managedptr->ToData(%mg_output);
          mg_output.WriteBytesToUMDataOutput();
        }
        catch (Apache::Geode::Client::GeodeException^ ex) {
          ex->ThrowNative();
        }
        catch (System::Exception^ ex) {
          Apache::Geode::Client::GeodeException::ThrowNative(ex);
        }
      }

      void ManagedDataSerializableFixedId::fromData(apache::geode::client::DataInput& input) {
        try {
          auto cache = CacheResolver::Lookup(input.getCache());
          Apache::Geode::Client::DataInput mg_input(&input, true, cache);
          m_managedptr->FromData(%mg_input);
          input.advanceCursor(mg_input.BytesReadInternally);
        }
        catch (Apache::Geode::Client::GeodeException^ ex) {
          ex->ThrowNative();
        }
        catch (System::Exception^ ex) {
          Apache::Geode::Client::GeodeException::ThrowNative(ex);
        }
      }

    }  // namespace client
  }  // namespace geode
}  // namespace apache
