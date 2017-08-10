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

#include "begin_native.hpp"
#include <GeodeTypeIdsImpl.hpp>
#include "end_native.hpp"

#include "../ICacheableKey.hpp"
#include "ManagedCacheableKey.hpp"
#include "../DataInput.hpp"
#include "../DataOutput.hpp"
#include "../CacheableString.hpp"
#include "../ExceptionTypes.hpp"
#include "../Log.hpp"

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
          Apache::Geode::Client::DataOutput mg_output(&output, true);
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

      apache::geode::client::Serializable* ManagedCacheableKeyGeneric::fromData(apache::geode::client::DataInput& input)
      {
        try {
          int pos = input.getBytesRead();
          //Apache::Geode::Client::Log::Debug("ManagedCacheableKeyGeneric::fromData");      
          Apache::Geode::Client::DataInput mg_input(&input, true, input.getCache());
          m_managedptr = m_managedptr->FromData(%mg_input);

          //this will move the cursor in c++ layer
          input.advanceCursor(mg_input.BytesReadInternally);
          m_objectSize = input.getBytesRead() - pos;
          //if(m_hashcode == 0)
          //m_hashcode = m_managedptr->GetHashCode();


        }
        catch (Apache::Geode::Client::GeodeException^ ex) {
          ex->ThrowNative();
        }
        catch (System::Exception^ ex) {
          Apache::Geode::Client::GeodeException::ThrowNative(ex);
        }
        return this;
      }

      System::UInt32 ManagedCacheableKeyGeneric::objectSize() const
      {
        try {
          int ret = m_managedptr->ObjectSize;
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

      System::Int32 ManagedCacheableKeyGeneric::classId() const
      {
        return (m_classId >= 0x80000000 ? 0 : m_classId);
      }

      int8_t ManagedCacheableKeyGeneric::typeId() const
      {
        if (m_classId >= 0x80000000) {
          return (int8_t)((m_classId - 0x80000000) % 0x20000000);
        }
        else if (m_classId <= 0x7F) {
          return (int8_t)GeodeTypeIdsImpl::CacheableUserData;
        }
        else if (m_classId <= 0x7FFF) {
          return (int8_t)GeodeTypeIdsImpl::CacheableUserData2;
        }
        else {
          return (int8_t)GeodeTypeIdsImpl::CacheableUserData4;
        }
      }

      int8_t ManagedCacheableKeyGeneric::DSFID() const
      {
        if (m_classId >= 0x80000000) {
          return (int8_t)((m_classId - 0x80000000) / 0x20000000);
        }
        return 0;
      }

      apache::geode::client::CacheableStringPtr ManagedCacheableKeyGeneric::toString() const
      {
        try {
          apache::geode::client::CacheableStringPtr cStr;
          Apache::Geode::Client::CacheableString::GetCacheableString(
            m_managedptr->ToString(), cStr);
          return cStr;
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
        try {
          // now checking classId(), typeId(), DSFID() etc. will be much more
          // expensive than just a dynamic_cast
          const ManagedCacheableKeyGeneric* p_other =
            dynamic_cast<const ManagedCacheableKeyGeneric*>(&other);
          if (p_other != NULL) {
            return static_cast<Apache::Geode::Client::ICacheableKey^>(
              (static_cast<Apache::Geode::Client::IGeodeSerializable^>((Apache::Geode::Client::IGeodeSerializable^)m_managedptr)))->Equals(
              static_cast<Apache::Geode::Client::ICacheableKey^>(p_other->ptr()));
          }
          return false;
        }
        catch (Apache::Geode::Client::GeodeException^ ex) {
          ex->ThrowNative();
        }
        catch (System::Exception^ ex) {
          Apache::Geode::Client::GeodeException::ThrowNative(ex);
        }
        return false;
      }

      bool ManagedCacheableKeyGeneric::operator ==(const ManagedCacheableKeyGeneric& other) const
      {
        try {
          return static_cast<Apache::Geode::Client::ICacheableKey^>(
            (Apache::Geode::Client::IGeodeSerializable^)(Apache::Geode::Client::IGeodeSerializable^)m_managedptr)->Equals(
            static_cast<Apache::Geode::Client::ICacheableKey^>(other.ptr()));
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
                             (Apache::Geode::Client::IGeodeSerializable^)m_managedptr)
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

      size_t ManagedCacheableKeyGeneric::logString(char* buffer, size_t maxLength) const
      {
        try {
          if (maxLength > 0) {
            String^ logstr = m_managedptr->GetType()->Name + '(' +
              m_managedptr->ToString() + ')';
            Apache::Geode::Client::ManagedString mg_str(logstr);
            return snprintf(buffer, maxLength, "%s", mg_str.CharPtr);
          }
        }
        catch (Apache::Geode::Client::GeodeException^ ex) {
          ex->ThrowNative();
        }
        catch (System::Exception^ ex) {
          Apache::Geode::Client::GeodeException::ThrowNative(ex);
        }
        return 0;
      }

    }  // namespace client
  }  // namespace geode
}  // namespace apache
