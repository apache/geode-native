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

#include "PdxHelper.hpp"

#include <geode/Cache.hpp>
#include <geode/DataInput.hpp>
#include <geode/PdxWrapper.hpp>
#include <geode/PoolManager.hpp>

#include "CacheRegionHelper.hpp"
#include "DataInputInternal.hpp"
#include "DataOutputInternal.hpp"
#include "PdxInstanceImpl.hpp"
#include "PdxReaderImpl.hpp"
#include "PdxType.hpp"
#include "PdxTypeRegistry.hpp"
#include "PdxUnreadData.hpp"
#include "PdxWriterImpl.hpp"
#include "SerializationRegistry.hpp"
#include "ThinClientPoolDM.hpp"
#include "TrackingPdxReaderImpl.hpp"
#include "Utils.hpp"

namespace apache {
namespace geode {
namespace client {

void PdxHelper::serializePdx(DataOutput& output,
                             const std::shared_ptr<PdxSerializable>& object) {
  std::shared_ptr<PdxType> pdxType;
  auto startPosition = output.getBufferLength();
  auto cache = CacheRegionHelper::getCacheImpl(output.getCache());

  {
    auto instance = std::dynamic_pointer_cast<PdxInstanceImpl>(object);
    if (instance != nullptr) {
      pdxType = PdxHelper::serializePdxInstance(output, instance);
    } else {
      pdxType = PdxHelper::serializePdxSerializable(output, object);
    }
  }

  auto endPosition = output.getBufferLength();
  auto length = endPosition - startPosition;

  // Initialize PdxType
  pdxType->initialize();

  // Register PdxType if needed and update PdxTypeId
  {
    auto registry = cache->getPdxTypeRegistry();
    if (registry->registerPdxTypeIfNeeded(
            pdxType, DataOutputInternal::getPool(output))) {
      // Place the cursor at the PdxType ID position, write it, and set the
      // cursor back to where it was
      output.rewindCursor(length - sizeof(uint32_t));
      output.writeInt(pdxType->getTypeId());
      output.advanceCursor(length - sizeof(uint32_t) * 2);
    }
  }

  // Update cache serialization statistics
  {
    auto& cachePerfStats = cache->getCachePerfStats();
    cachePerfStats.incPdxSerialization(length);
  }
}

std::shared_ptr<PdxType> PdxHelper::serializePdxInstance(
    DataOutput& output, const std::shared_ptr<PdxInstanceImpl>& instance) {
  PdxWriterImpl writer{output};
  instance->toData(writer);
  return instance->getPdxType();
}

std::shared_ptr<PdxType> PdxHelper::serializePdxSerializable(
    DataOutput& output, const std::shared_ptr<PdxSerializable>& object) {
  auto cacheImpl = CacheRegionHelper::getCacheImpl(output.getCache());
  auto pdxType = std::make_shared<PdxType>(object->getClassName());
  auto registry = cacheImpl->getPdxTypeRegistry();
  std::shared_ptr<PdxUnreadData> ud;

  PdxWriterImpl writer{pdxType, output};
  if (!cacheImpl->getPdxIgnoreUnreadFields() &&
      (ud = registry->getUnreadData(object))) {
    writer.setUnreadData(ud);
  }

  object->toData(writer);
  writer.completeSerialization();
  return pdxType;
}

std::shared_ptr<PdxSerializable> PdxHelper::deserializePdx(DataInput& input) {
  auto length = input.readInt32();
  auto typeId = input.readInt32();

  auto cache = CacheRegionHelper::getCacheImpl(input.getCache());
  auto pdxType = cache->getPdxTypeRegistry()->getPdxType(
      typeId, DataInputInternal::getPool(input));

  return deserializePdx(input, pdxType, length);
}

std::shared_ptr<PdxSerializable> PdxHelper::deserializePdx(
    DataInput& input, std::shared_ptr<PdxType> pdxType, int32_t length) {
  auto cache = CacheRegionHelper::getCacheImpl(input.getCache());
  auto pdxRegistry = cache->getPdxTypeRegistry();

  if (!pdxType->isDomainClass() || pdxRegistry->getPdxReadSerialized()) {
    return deserializePdxInstance(input, pdxType, length);
  }

  auto object = deserializePdxSerializable(input, pdxType, length);
  if (!object) {
    // In case nullptr was returned, fallback to PdxInstance deserialization
    return deserializePdxInstance(input, pdxType, length);
  }

  return object;
}

std::shared_ptr<PdxSerializable> PdxHelper::deserializePdxInstance(
    DataInput& input, std::shared_ptr<PdxType> pdxType, int32_t length) {
  auto bufferPtr = input.currentBufferPosition();
  auto cache = CacheRegionHelper::getCacheImpl(input.getCache());

  input.advanceCursor(length);
  std::vector<uint8_t> buffer{bufferPtr, bufferPtr + length};
  return std::make_shared<PdxInstanceImpl>(std::move(buffer), pdxType, *cache);
}

std::shared_ptr<PdxSerializable> PdxHelper::deserializePdxSerializable(
    DataInput& input, std::shared_ptr<PdxType> pdxType, int32_t length) {
  auto cache = CacheRegionHelper::getCacheImpl(input.getCache());
  auto pdxRegistry = cache->getPdxTypeRegistry();

  std::shared_ptr<PdxReaderImpl> reader;
  std::shared_ptr<TrackingPdxReaderImpl> trackingReader;

  if (pdxRegistry->getPdxIgnoreUnreadFields()) {
    reader = std::make_shared<PdxReaderImpl>(input, pdxType, length);
  } else {
    reader = trackingReader =
        std::make_shared<TrackingPdxReaderImpl>(input, pdxType, length);
  }

  // If the classname is not registered, object will be of type PdxWrapper
  // and if there is no PdxSerializer set, this will return nullptr, falling
  // back to PdxInstance deserialization
  auto serializationRegistry = cache->getSerializationRegistry();
  auto object =
      serializationRegistry->getPdxSerializableType(pdxType->getPdxClassName());
  object->fromData(*reader);

  if (auto wrapper = std::dynamic_pointer_cast<PdxWrapper>(object)) {
    if (!wrapper->getObject()) {
      // No serializer was registered to deserialize this type.
      // Fall back to PdxInstance
      return nullptr;
    }
  }

  if (trackingReader) {
    if (auto ud = trackingReader->getUnreadData()) {
      pdxRegistry->setUnreadData(cache->getExpiryTaskManager(), object, ud);
    }
  }

  reader->moveInputToEnd();
  return object;
}

int32_t PdxHelper::getOffsetSize(int32_t payloadLength) {
  if (payloadLength <= std::numeric_limits<uint8_t>::max()) {
    return sizeof(uint8_t);
  } else if (payloadLength <= std::numeric_limits<uint16_t>::max()) {
    return sizeof(uint16_t);
  }

  return sizeof(uint32_t);
}

int32_t PdxHelper::readInt32(uint8_t* offsetPosition) {
  int32_t data = offsetPosition[0];
  data = (data << 8) | offsetPosition[1];
  data = (data << 8) | offsetPosition[2];
  data = (data << 8) | offsetPosition[3];

  return data;
}

int32_t PdxHelper::readInt16(uint8_t* offsetPosition) {
  int16_t data = offsetPosition[0];
  data = (data << 8) | offsetPosition[1];
  return static_cast<int32_t>(data);
}

int32_t PdxHelper::readUInt16(uint8_t* offsetPosition) {
  uint16_t data = offsetPosition[0];
  data = (data << 8) | offsetPosition[1];
  return static_cast<int32_t>(data);
}

int32_t PdxHelper::readByte(uint8_t* offsetPosition) {
  return static_cast<int32_t>(offsetPosition[0]);
}

void PdxHelper::writeByte(uint8_t* offsetPosition, int32_t value) {
  offsetPosition[0] = static_cast<uint8_t>(value);
}

int32_t PdxHelper::readInt(uint8_t* offsetPosition, int size) {
  switch (size) {
    case 1:
      return readByte(offsetPosition);
    case 2:
      return readUInt16(offsetPosition);
    case 4:
      return readInt32(offsetPosition);
  }
  throw;
}

int32_t PdxHelper::getEnumValue(
    const char* enumClassName, const char* enumName, int hashcode,
    std::shared_ptr<PdxTypeRegistry> pdxTypeRegistry) {
  const auto& ei =
      std::make_shared<EnumInfo>(enumClassName, enumName, hashcode);
  return pdxTypeRegistry->getEnumValue(ei);
}
std::shared_ptr<EnumInfo> PdxHelper::getEnum(
    int enumId, std::shared_ptr<PdxTypeRegistry> pdxTypeRegistry) {
  const auto& ei = pdxTypeRegistry->getEnum(enumId);
  return ei;
}
}  // namespace client
}  // namespace geode
}  // namespace apache
