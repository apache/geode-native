
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


#include "PdxManagedCacheableKey.hpp"

#include "../begin_native.hpp"
#include "../end_native.hpp"

#include "../DataInput.hpp"
#include "../DataOutput.hpp"
#include "../CacheableString.hpp"
#include "../ExceptionTypes.hpp"
#include "../String.hpp"
#include "CacheRegionHelper.hpp"
#include "PdxHelper.hpp"
#include "SafeConvert.hpp"
#include "CacheResolver.hpp"


using namespace System;
using namespace msclr::interop;

namespace apache
{
  namespace geode
  {
    namespace client
    {
      void PdxManagedCacheableKey::toData(PdxWriter& output) const
      {
        throw IllegalStateException("Should have been intercepted by ManagedPdxTypeHandler::serialize.");
      }

      void PdxManagedCacheableKey::fromData(PdxReader& input)
      {
        throw IllegalStateException("Should have been intercepted by ManagedPdxTypeHandler::deserialize.");
      }

      size_t PdxManagedCacheableKey::objectSize() const
      {
        try
        {
          return m_objectSize;
        }
        catch (Apache::Geode::Client::GeodeException^ ex)
        {
          ex->ThrowNative();
        }
        catch (System::Exception^ ex)
        {
          Apache::Geode::Client::GeodeException::ThrowNative(ex);
        }

        return 0;
      }

      std::string PdxManagedCacheableKey::toString() const
      {
        try
        {
          return Apache::Geode::Client::to_utf8(m_managedptr->ToString());
        }
        catch (Apache::Geode::Client::GeodeException^ ex)
        {
          ex->ThrowNative();
        }
        catch (System::Exception^ ex)
        {
          Apache::Geode::Client::GeodeException::ThrowNative(ex);
        }

        return "";
      }

      const std::string& PdxManagedCacheableKey::getClassName() const {
        return m_className;
      }

      bool PdxManagedCacheableKey::operator ==(const apache::geode::client::CacheableKey& other) const
      {
        try
        {
          // now checking classId(), typeId(), DSFID() etc. will be much more
          // expensive than just a dynamic_cast
          if (const auto p_other = dynamic_cast<const PdxManagedCacheableKey*>(&other))
          {
            return ((Apache::Geode::Client::IPdxSerializable^)m_managedptr)->Equals((p_other->ptr()));
          }

          return false;
        }
        catch (Apache::Geode::Client::GeodeException^ ex)
        {
          ex->ThrowNative();
        }
        catch (System::Exception^ ex)
        {
          Apache::Geode::Client::GeodeException::ThrowNative(ex);
        }

        return false;
      }

      bool PdxManagedCacheableKey::operator ==(const PdxManagedCacheableKey& other) const
      {
        try
        {
          return ((Apache::Geode::Client::IPdxSerializable^)m_managedptr)->Equals((other.ptr()));
        }
        catch (Apache::Geode::Client::GeodeException^ ex)
        {
          ex->ThrowNative();
        }
        catch (System::Exception^ ex)
        {
          Apache::Geode::Client::GeodeException::ThrowNative(ex);
        }

        return false;
      }

      System::Int32 PdxManagedCacheableKey::hashcode() const
      {
        if (m_hashcode != 0)
        {
          return m_hashcode;
        }
        try
        {
          auto tmp = const_cast<PdxManagedCacheableKey*>(this);
          tmp->m_hashcode =
            ((Apache::Geode::Client::IPdxSerializable^)tmp->m_managedptr)
            ->GetHashCode();

          return m_hashcode;
        }
        catch (Apache::Geode::Client::GeodeException^ ex)
        {
          ex->ThrowNative();
        }
        catch (System::Exception^ ex)
        {
          Apache::Geode::Client::GeodeException::ThrowNative(ex);
        }

        return 0;
      }

      bool PdxManagedCacheableKey::hasDelta() const
      {
        if (m_managedDeltaptr)
        {
          return m_managedDeltaptr->HasDelta();
        }

        return false;
      }

      void PdxManagedCacheableKey::toDelta(DataOutput& output) const
      {
        try
        {
          auto cache = CacheResolver::Lookup(output.getCache());
          Apache::Geode::Client::DataOutput mg_output(&output, true, cache);
          m_managedDeltaptr->ToDelta(%mg_output);
          //this will move the cursor in c++ layer
          mg_output.WriteBytesToUMDataOutput();
        }
        catch (Apache::Geode::Client::GeodeException^ ex)
        {
          ex->ThrowNative();
        }
        catch (System::Exception^ ex)
        {
          Apache::Geode::Client::GeodeException::ThrowNative(ex);
        }
      }

      void PdxManagedCacheableKey::fromDelta(native::DataInput& input)
      {
        try
        {
          auto cache = CacheResolver::Lookup(input.getCache());
          Apache::Geode::Client::DataInput mg_input(&input, true, cache);
          m_managedDeltaptr->FromDelta(%mg_input);

          //this will move the cursor in c++ layer
          input.advanceCursor(mg_input.BytesReadInternally);

          m_hashcode = m_managedptr->GetHashCode();
        }
        catch (Apache::Geode::Client::GeodeException^ ex)
        {
          ex->ThrowNative();
        }
        catch (System::Exception^ ex)
        {
          Apache::Geode::Client::GeodeException::ThrowNative(ex);
        }
      }

      std::shared_ptr<Delta> PdxManagedCacheableKey::clone() const
      {
        try
        {
          if (auto cloneable = dynamic_cast<ICloneable^>((Apache::Geode::Client::IDelta^) m_managedDeltaptr))
          {
            auto managedclone = dynamic_cast<Apache::Geode::Client::IPdxSerializable^>(cloneable->Clone());

            return std::shared_ptr<Delta>(dynamic_cast<PdxManagedCacheableKey*>(GetNativeWrapperForManagedObject(managedclone)));
          }
        }
        catch (Apache::Geode::Client::GeodeException^ ex)
        {
          ex->ThrowNative();
        }
        catch (System::Exception^ ex)
        {
          Apache::Geode::Client::GeodeException::ThrowNative(ex);
        }

        return nullptr;
      }
    }  // namespace client
  }  // namespace geode
}  // namespace apache
