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

#include "PdxManagedCacheableKeyBytes.hpp"
#include "../DataInput.hpp"
#include "../DataOutput.hpp"
#include "../Serializable.hpp"
#include "../CacheableString.hpp"
#include "../ExceptionTypes.hpp"
#include "ManagedString.hpp"
#include "SafeConvert.hpp"

using namespace System;

namespace apache
{
  namespace geode
  {
    namespace client
    {
      void PdxManagedCacheableKeyBytes::toData(apache::geode::client::DataOutput& output) const
      {
        // Apache::Geode::Client::Log::Debug("PdxManagedCacheableKeyBytes::toData: current domain ID: " + System::Threading::Thread::GetDomainID() + " for object: " + System::Convert::ToString((uint64_t) this) + " with its domain ID: " + m_domainId );
        try {
          //TODO: I think this should work as it is
          output.writeBytesOnly(m_bytes, m_size);
        }
        catch (Apache::Geode::Client::GeodeException^ ex) {
          ex->ThrowNative();
        }
        catch (System::Exception^ ex) {
          Apache::Geode::Client::GeodeException::ThrowNative(ex);
        }
      }

      apache::geode::client::Serializable* PdxManagedCacheableKeyBytes::fromData(apache::geode::client::DataInput& input)
      {
        try {

          Apache::Geode::Client::DataInput mg_input(&input, true, input.getCache());
          const System::Byte* objStartPos = input.currentBufferPosition();

          Apache::Geode::Client::IPdxSerializable^ obj = Apache::Geode::Client::Internal::PdxHelper::DeserializePdx(%mg_input, false, CacheRegionHelper::getCacheImpl(input.getCache())->getSerializationRegistry().get());
          input.advanceCursor(mg_input.BytesReadInternally);

          m_hashCode = obj->GetHashCode();

          const System::Byte* objEndPos = input.currentBufferPosition();

          m_size = (System::UInt32)(objEndPos - objStartPos);
          m_bytes = input.getBufferCopyFrom(objStartPos, m_size);

        }
        catch (Apache::Geode::Client::GeodeException^ ex) {
          ex->ThrowNative();
        }
        catch (System::Exception^ ex) {
          Apache::Geode::Client::GeodeException::ThrowNative(ex);
        }
        return this;
      }

      System::UInt32 PdxManagedCacheableKeyBytes::objectSize() const
      {
        try {
          return m_size;
        }
        catch (Apache::Geode::Client::GeodeException^ ex) {
          ex->ThrowNative();
        }
        catch (System::Exception^ ex) {
          Apache::Geode::Client::GeodeException::ThrowNative(ex);
        }
        return 0;
      }

      System::Int32 PdxManagedCacheableKeyBytes::classId() const
      {
        return 0;
      }

      int8_t PdxManagedCacheableKeyBytes::typeId() const
      {

        return (int8_t)GeodeTypeIdsImpl::PDX;
      }

      int8_t PdxManagedCacheableKeyBytes::DSFID() const
      {
         return 0;
      }

      apache::geode::client::CacheableStringPtr PdxManagedCacheableKeyBytes::toString() const
      {
        try {
          Apache::Geode::Client::IPdxSerializable^ manageObject = getManagedObject();
          if (manageObject != nullptr)
          {
            apache::geode::client::CacheableStringPtr cStr;
            Apache::Geode::Client::CacheableString::GetCacheableString(
              manageObject->ToString(), cStr);
            return cStr;
          }
        }
        catch (Apache::Geode::Client::GeodeException^ ex) {
          ex->ThrowNative();
        }
        catch (System::Exception^ ex) {
          Apache::Geode::Client::GeodeException::ThrowNative(ex);
        }
        return nullptr;
      }

      bool PdxManagedCacheableKeyBytes::operator ==(const apache::geode::client::CacheableKey& other) const
      {
        try {
          //  Apache::Geode::Client::Log::Debug("PdxManagedCacheableKeyBytes::equal");
          // now checking classId(), typeId(), DSFID() etc. will be much more
          // expensive than just a dynamic_cast
          const PdxManagedCacheableKeyBytes* p_other =
            dynamic_cast<const PdxManagedCacheableKeyBytes*>(&other);
          if (p_other != NULL) {
            Apache::Geode::Client::IPdxSerializable^ obj = getManagedObject();
            bool ret = obj->Equals(p_other->ptr());
            return ret;
          }
        }
        catch (Apache::Geode::Client::GeodeException^ ex) {
          ex->ThrowNative();
        }
        catch (System::Exception^ ex) {
          Apache::Geode::Client::GeodeException::ThrowNative(ex);
        }
        // Apache::Geode::Client::Log::Debug("PdxManagedCacheableKeyBytes::equal returns false");
        return false;
      }

      bool PdxManagedCacheableKeyBytes::operator ==(const PdxManagedCacheableKeyBytes& other) const
      {
        try {
          //Apache::Geode::Client::Log::Debug("PdxManagedCacheableKeyBytes::equal. ");
          Apache::Geode::Client::IPdxSerializable^ obj = getManagedObject();
          bool ret = obj->Equals(other.ptr());
          return ret;
        }
        catch (Apache::Geode::Client::GeodeException^ ex) {
          ex->ThrowNative();
        }
        catch (System::Exception^ ex) {
          Apache::Geode::Client::GeodeException::ThrowNative(ex);
        }
        //  Apache::Geode::Client::Log::Debug("PdxManagedCacheableKeyBytes::equal return false");
        return false;
      }

      System::Int32 PdxManagedCacheableKeyBytes::hashcode() const
      {
        return m_hashCode;
      }

      size_t PdxManagedCacheableKeyBytes::logString(char* buffer, size_t maxLength) const
      {
        try {
          Apache::Geode::Client::IPdxSerializable^ manageObject = getManagedObject();
          if (manageObject != nullptr)
          {
            if (maxLength > 0) {
              String^ logstr = manageObject->GetType()->Name + '(' +
                manageObject->ToString() + ')';
              Apache::Geode::Client::ManagedString mg_str(logstr);
              return snprintf(buffer, maxLength, "%s", mg_str.CharPtr);
            }
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

      Apache::Geode::Client::IPdxSerializable^
        PdxManagedCacheableKeyBytes::getManagedObject() const
      {
        auto dinp = m_cache->createDataInput(m_bytes, m_size);
        Apache::Geode::Client::DataInput mg_dinp(dinp.get(), true, m_cache);
        return  Apache::Geode::Client::Internal::PdxHelper::DeserializePdx(%mg_dinp, false, CacheRegionHelper::getCacheImpl(m_cache)->getSerializationRegistry().get());
      }

      bool PdxManagedCacheableKeyBytes::hasDelta()
      {
            return m_hasDelta;
      }

      void PdxManagedCacheableKeyBytes::toDelta(DataOutput& output) const
      {
        try {
          Apache::Geode::Client::Log::Debug("PdxManagedCacheableKeyBytes::toDelta: current domain ID: " + System::Threading::Thread::GetDomainID() + " for object: " + System::Convert::ToString((uint64_t) this) + " with its domain ID: " + m_domainId);
          Apache::Geode::Client::IGeodeDelta^ deltaObj = dynamic_cast<Apache::Geode::Client::IGeodeDelta^>(this->getManagedObject());
          Apache::Geode::Client::DataOutput mg_output(&output, true);
          deltaObj->ToDelta(%mg_output);
          mg_output.WriteBytesToUMDataOutput();
        }
        catch (Apache::Geode::Client::GeodeException^ ex) {
          ex->ThrowNative();
        }
        catch (System::Exception^ ex) {
          Apache::Geode::Client::GeodeException::ThrowNative(ex);
        }
      }

      void PdxManagedCacheableKeyBytes::fromDelta(DataInput& input)
      {
        try {
          Apache::Geode::Client::Log::Debug("PdxManagedCacheableKeyBytes::fromDelta:");
          Apache::Geode::Client::IGeodeDelta^ deltaObj = dynamic_cast<Apache::Geode::Client::IGeodeDelta^>(this->getManagedObject());
          Apache::Geode::Client::DataInput mg_input(&input, true, input.getCache());
          deltaObj->FromDelta(%mg_input);

          Apache::Geode::Client::IPdxSerializable^ managedptr =
            dynamic_cast <Apache::Geode::Client::IPdxSerializable^> (deltaObj);
          {
            Apache::Geode::Client::Log::Debug("PdxManagedCacheableKeyBytes::fromDelta: current domain ID: " + System::Threading::Thread::GetDomainID() + " for object: " + System::Convert::ToString((uint64_t) this) + " with its domain ID: " + m_domainId);
            auto dataOut = m_cache->createDataOutput();
            Apache::Geode::Client::DataOutput mg_output(dataOut.get(), true);
            Apache::Geode::Client::Internal::PdxHelper::SerializePdx(%mg_output, managedptr);
            mg_output.WriteBytesToUMDataOutput();

             GF_SAFE_DELETE(m_bytes);
            m_bytes = dataOut->getBufferCopy();
            m_size = dataOut->getBufferLength();
            Apache::Geode::Client::Log::Debug("PdxManagedCacheableKeyBytes::fromDelta objectSize = " + m_size + " m_hashCode = " + m_hashCode);
            m_hashCode = managedptr->GetHashCode();
          }
        }
        catch (Apache::Geode::Client::GeodeException^ ex) {
          ex->ThrowNative();
        }
        catch (System::Exception^ ex) {
          Apache::Geode::Client::GeodeException::ThrowNative(ex);
        }
      }

      DeltaPtr PdxManagedCacheableKeyBytes::clone()
      {
        try {
          Apache::Geode::Client::IGeodeDelta^ deltaObj = dynamic_cast<Apache::Geode::Client::IGeodeDelta^>(this->getManagedObject());
          ICloneable^ cloneable = dynamic_cast<ICloneable^>((Apache::Geode::Client::IGeodeDelta^) deltaObj);
          if (cloneable) {
            Apache::Geode::Client::IPdxSerializable^ Mclone =
              dynamic_cast<Apache::Geode::Client::IPdxSerializable^>(cloneable->Clone());
            return DeltaPtr(static_cast<PdxManagedCacheableKeyBytes*>(
              SafeGenericM2UMConvert(Mclone, m_cache)));
          }
          else {
            return Delta::clone();
          }
        }
        catch (Apache::Geode::Client::GeodeException^ ex) {
          ex->ThrowNative();
        }
        catch (System::Exception^ ex) {
          Apache::Geode::Client::GeodeException::ThrowNative(ex);
        }
        return nullptr;
      }

    }  // namespace client
  }  // namespace geode
}  // namespace apache
