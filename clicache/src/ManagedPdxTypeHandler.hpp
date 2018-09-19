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

#include "begin_native.hpp"
#include <SerializationRegistry.hpp>
#include "end_native.hpp"

#include "impl/CacheResolver.hpp"
#include "impl/PdxHelper.hpp"
#include "impl/PdxManagedCacheableKey.hpp"

namespace apache {
  namespace geode {
    namespace client {
      namespace Managed = Apache::Geode::Client;

      /**
       * Intercept (de)serialization of PDX types into the .NET managed layer.
       */
      class ManagedPdxTypeHandler : public PdxTypeHandler
      {
      public:
        ~ManagedPdxTypeHandler() noexcept override = default;

        void serialize(const std::shared_ptr<PdxSerializable>& pdxSerializable,
          DataOutput& dataOutput) const override
        {
          if (auto wrappedPdxSerializable = dynamic_cast<const PdxManagedCacheableKey*>(pdxSerializable.get()))
          {
            try
            {
              auto cache = CacheResolver::Lookup(dataOutput.getCache());
              Managed::DataOutput managedDataOutput(&dataOutput, true, cache);
              
              auto managedPdx = wrappedPdxSerializable->ptr();
              Managed::Internal::PdxHelper::SerializePdx(%managedDataOutput, managedPdx);
              
              managedDataOutput.WriteBytesToUMDataOutput();
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
          else
          {
            throw IllegalStateException("Not managed PDX object.");
          }
        }

        std::shared_ptr<PdxSerializable> deserialize(DataInput& dataInput) const override
        {
          try
          {
            auto cache = CacheResolver::Lookup(dataInput.getCache());

            Managed::DataInput managedDataInput(&dataInput, true, cache);

            auto serializationRegistry = CacheRegionHelper::getCacheImpl(dataInput.getCache())->getSerializationRegistry().get();
            auto managedPdx = Apache::Geode::Client::Internal::PdxHelper::DeserializePdx(%managedDataInput, false, serializationRegistry);
            
            dataInput.advanceCursor(managedDataInput.BytesReadInternally);
            
            return std::shared_ptr<PdxManagedCacheableKey>(new PdxManagedCacheableKey(managedPdx));
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

      };

    } //  namespace client
  } //  namespace geode
} //  namespace apache

