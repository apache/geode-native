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

#include "ManagedCacheableDelta.hpp"
#include "../DataInput.hpp"
#include "../DataOutput.hpp"
#include "../CacheableString.hpp"
#include "../ExceptionTypes.hpp"
#include "SafeConvert.hpp"
#include "CacheResolver.hpp"

using namespace System;

namespace apache
{
  namespace geode
  {
    namespace client
    {
      void ManagedCacheableDeltaGeneric::toData(DataOutput& output) const
      {
        try {
          System::UInt32 pos = (int)output.getBufferLength();
          auto cache = CacheResolver::Lookup(output.getCache());
          Apache::Geode::Client::DataOutput mg_output(&output, true, cache);
          m_managedSerializableptr->ToData(%mg_output);
          //this will move the cursor in c++ layer
          mg_output.WriteBytesToUMDataOutput();
          auto tmp = const_cast<ManagedCacheableDeltaGeneric*>(this);
          tmp->m_objectSize = (int)(output.getBufferLength() - pos);
        }
        catch (Apache::Geode::Client::GeodeException^ ex) {
          ex->ThrowNative();
        }
        catch (System::Exception^ ex) {
          Apache::Geode::Client::GeodeException::ThrowNative(ex);
        }
      }

     void ManagedCacheableDeltaGeneric::fromData(DataInput& input)
      {
        try {
          auto pos = input.getBytesRead();
          auto cache = CacheResolver::Lookup(input.getCache());
          Apache::Geode::Client::DataInput mg_input(&input, true, cache);
          m_managedSerializableptr->FromData(%mg_input);

          //this will move the cursor in c++ layer
          input.advanceCursor(mg_input.BytesReadInternally);

          m_objectSize = input.getBytesRead() - pos;

          if (m_hashcode == 0)
            m_hashcode = m_managedptr->GetHashCode();

        }
        catch (Apache::Geode::Client::GeodeException^ ex) {
          ex->ThrowNative();
        }
        catch (System::Exception^ ex) {
          Apache::Geode::Client::GeodeException::ThrowNative(ex);
        }
      }

     size_t ManagedCacheableDeltaGeneric::objectSize() const
      {
        try {
          auto ret = m_managedSerializableptr->ObjectSize;
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

      bool ManagedCacheableDeltaGeneric::hasDelta() const
      {
        return m_managedptr->HasDelta();
      }

      void ManagedCacheableDeltaGeneric::toDelta(DataOutput& output) const
      {
        try {
          auto cache = CacheResolver::Lookup(output.getCache());
          Apache::Geode::Client::DataOutput mg_output(&output, true, cache);
          m_managedptr->ToDelta(%mg_output);
          //this will move the cursor in c++ layer
          mg_output.WriteBytesToUMDataOutput();
        }
        catch (Apache::Geode::Client::GeodeException^ ex) {
          ex->ThrowNative();
        }
        catch (System::Exception^ ex) {
          Apache::Geode::Client::GeodeException::ThrowNative(ex);
        }
      }

      void ManagedCacheableDeltaGeneric::fromDelta(DataInput& input)
      {
        try {
          auto cache = CacheResolver::Lookup(input.getCache());
          Apache::Geode::Client::DataInput mg_input(&input, true, cache);
          m_managedptr->FromDelta(%mg_input);

          //this will move the cursor in c++ layer
          input.advanceCursor(mg_input.BytesReadInternally);

          m_hashcode = m_managedptr->GetHashCode();
        }
        catch (Apache::Geode::Client::GeodeException^ ex) {
          ex->ThrowNative();
        }
        catch (System::Exception^ ex) {
          Apache::Geode::Client::GeodeException::ThrowNative(ex);
        }
      }

      std::shared_ptr<Delta> ManagedCacheableDeltaGeneric::clone() const
      {
        try {
          if (auto cloneable = dynamic_cast<ICloneable^>((
            Apache::Geode::Client::IDelta^) m_managedptr)) {
            auto Mclone = 
              dynamic_cast<Apache::Geode::Client::ISerializable^>(cloneable->Clone());
            return std::shared_ptr<Delta>(dynamic_cast<ManagedCacheableDeltaGeneric*>(
              GetNativeWrapperForManagedObject(Mclone)));
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

      bool ManagedCacheableDeltaGeneric::operator ==(const apache::geode::client::CacheableKey& other) const
      {
        try {
          // now checking classId(), typeId(), DSFID() etc. will be much more
          // expensive than just a dynamic_cast
          const ManagedCacheableDeltaGeneric* p_other =
            dynamic_cast<const ManagedCacheableDeltaGeneric*>(&other);
          if (p_other != NULL) {
            return m_managedptr->Equals(p_other->ptr());
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

      bool ManagedCacheableDeltaGeneric::operator == (const ManagedCacheableDeltaGeneric& other) const
      {
        try {
          return m_managedptr->Equals(other.ptr());
        }
        catch (Apache::Geode::Client::GeodeException^ ex) {
          ex->ThrowNative();
        }
        catch (System::Exception^ ex) {
          Apache::Geode::Client::GeodeException::ThrowNative(ex);
        }
        return false;

      }

      System::Int32 ManagedCacheableDeltaGeneric::hashcode() const
      {
        throw gcnew System::NotSupportedException;
      }

    }  // namespace client
  }  // namespace geode
}  // namespace apache
