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
#include "impl/ManagedCacheableKey.hpp"
#include "impl/SafeConvert.hpp"

namespace apache {
  namespace geode {
    namespace client {
      namespace Managed = Apache::Geode::Client;

      /**
       * Intercept (de)serialization of DataSerializable types into the .NET managed layer.
       */
      class ManagedDataSerializableHandler : public DataSerializableHandler
      {
      public:
        ~ManagedDataSerializableHandler() noexcept override = default;

        void serialize(const std::shared_ptr<DataSerializable>& dataSerializable,
          DataOutput& dataOutput, bool isDelta) const override
        {
          Managed::ISerializable^ data = Managed::SafeUMSerializableConvertGeneric(dataSerializable);
          auto cache = CacheResolver::Lookup(dataOutput.getCache());

          int32_t objectID = cache->GetTypeRegistry()->GetIdForManagedType(data->GetType());
          
          auto dsCode = SerializationRegistry::getSerializableDataDsCode(objectID);

          dataOutput.write(static_cast<int8_t>(dsCode));
          switch (dsCode) {
          case DSCode::CacheableUserData:
            dataOutput.write(static_cast<int8_t>(objectID));
            break;
          case DSCode::CacheableUserData2:
            dataOutput.writeInt(static_cast<int16_t>(objectID));
            break;
          case DSCode::CacheableUserData4:
            dataOutput.writeInt(static_cast<int32_t>(objectID));
            break;
          default:
            IllegalStateException("Invalid DS Code.");
          }

          if (isDelta) {
            const Delta* ptr = dynamic_cast<const Delta*>(dataSerializable.get());
            ptr->toDelta(dataOutput);
          }
          else {
            dataSerializable->toData(dataOutput);
          }
        }

        std::shared_ptr<DataSerializable> deserialize(DataInput& dataInput,  DSCode typeId) const override
        {
          try
          {
            int32_t classId = -1;

            switch (typeId) {
              case DSCode::CacheableUserData: {
                classId = dataInput.read();
                break;
              }
              case DSCode::CacheableUserData2: {
                classId = dataInput.readInt16();
                break;
              }
              case DSCode::CacheableUserData4: {
                classId = dataInput.readInt32();
                break;
              }
              default:
                break;
            }
                      
            auto cache = CacheResolver::Lookup(dataInput.getCache());
            auto createType = cache->GetTypeRegistry()->GetManagedObjectFactory(classId);

            if (createType == nullptr) {
              LOG_ERROR(
                  "Unregistered class ID %d during deserialization: Did the "
                  "application register serialization types?",
                  classId);

              // instead of a null key or null value... an Exception should be thrown..
              throw IllegalStateException("Unregistered class ID in deserialization");
            }

            auto managedDataSerializable = (Apache::Geode::Client::IDataSerializable^) createType();
            auto nativeDataSerializable = std::shared_ptr<DataSerializable>(
              dynamic_cast<DataSerializable*>(GetNativeWrapperForManagedIDataSerializable(managedDataSerializable)));
            nativeDataSerializable->fromData(dataInput);

            return nativeDataSerializable;
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
        inline DSCode getSerializableDataDsCode(int32_t classId) {
          if (classId <= std::numeric_limits<int8_t>::max() &&
            classId >= std::numeric_limits<int8_t>::min()) {
            return DSCode::CacheableUserData;
          } else if (classId <= std::numeric_limits<int16_t>::max() &&
                classId >= std::numeric_limits<int16_t>::min()) {
            return DSCode::CacheableUserData2;
          } else {
            return DSCode::CacheableUserData4;
          }
        }
  
      };

    } //  namespace client
  } //  namespace geode
} //  namespace apache

