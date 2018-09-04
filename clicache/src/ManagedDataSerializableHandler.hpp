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
					DataOutput& dataOutput) const override
				{
					if (auto wrappedDataSerializable = dynamic_cast<const ManagedCacheableKeyGeneric*>(dataSerializable.get()))
					{
						try
						{
							auto&& cache = CacheResolver::Lookup(dataOutput.getCache());
							Managed::DataOutput managedDataOutput(&dataOutput, true, cache);
							auto&& type = wrappedDataSerializable->getType();
							auto&& typeIterator = ManagedTypeDelegates.find(type);
							auto&& id = typeIterator->second;

							auto dsCode = getSerializableDataDsCode(id);

							output.write(static_cast<int8_t>(dsCode));
							switch (dsCode) {
							  case DSCode::CacheableUserData:
								output.write(static_cast<int8_t>(id));
								break;
							  case DSCode::CacheableUserData2:
								output.writeInt(static_cast<int16_t>(id));
								break;
							  case DSCode::CacheableUserData4:
								output.writeInt(static_cast<int32_t>(id));
								break;
							  default:
								IllegalStateException("Invalid DS Code.");
							}

							if (isDelta) {
							  const Delta* ptr = dynamic_cast<const Delta*>(obj);
							  ptr->toDelta(output);
							} else {
								        
              auto&& managedPdx = wrappedPdxSerializable->ptr();
              Managed::Internal::PdxHelper::SerializePdx(%managedDataOutput, managedPdx);
              
              managedDataOutput.WriteBytesToUMDataOutput();
								obj->toData(output);
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
					}
					else
					{
						throw IllegalStateException("Not managed DataSerializable object.");
					}
				}

				std::shared_ptr<DataSerializable> deserialize(DataInput& dataInput,  int8_t typeId) const override
				{
					try
					{
						bool findinternal = false;
						auto typedTypeId = static_cast<DSCode>(typeId);
						int32_t classId = typeId;

						if (typeId == -1) {
						classId = dataInput.read();
						typedTypeId = static_cast<DSCode>(classId);
						}

						LOGDEBUG(
							"SerializationRegistry::deserialize typeid = %d currentTypeId= %" PRId8,
							typeId, typedTypeId);

						switch (typedTypeId) {
						case DSCode::CacheableNullString: {
							return std::shared_ptr<Serializable>(
								CacheableString::createDeserializable());
						}
						case DSCode::CacheableEnum: {
							auto enumObject = CacheableEnum::create(" ", " ", 0);
							enumObject->fromData(input);
							return enumObject;
						}
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
						case DSCode::FixedIDByte: {
							classId = dataInput.read();
							findinternal = true;
							break;
						}
						case DSCode::FixedIDShort: {
							classId = dataInput.readInt16();
							findinternal = true;
							break;
						}
						case DSCode::FixedIDInt: {
							classId = dataInput.readInt32();
							findinternal = true;
							break;
						}
						case DSCode::NullObj: {
							return nullptr;
						}
						default:
							break;
						}

						TypeFactoryMethod createType = nullptr;
						
						auto&& cache = CacheResolver::Lookup(dataInput.getCache());
						if (findinternal) {
							// need manaaged map
							cache->TypeRegistry().some map dot find...
						theTypeMap.findDataSerializableFixedId(classId, createType);
						} else {
						theTypeMap.findDataSerializable(classId, createType);
						}

						if (createType == nullptr) {
						if (findinternal) {
							LOGERROR(
								"Unregistered class ID %d during deserialization: Did the "
								"application register serialization types?",
								classId);
						} else {
							LOGERROR(
								"Unregistered class ID %d during deserialization: Did the "
								"application register serialization types?",
								classId);
						}

						// instead of a null key or null value... an Exception should be thrown..
						throw IllegalStateException("Unregistered class ID in deserialization");
						}

						std::shared_ptr<Serializable> obj(createType());

						deserialize(input, obj);

						return obj;
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

