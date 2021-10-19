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
#include "PdxLocalReader.hpp"
#include "PdxReaderWithTypeCollector.hpp"
#include "PdxRemoteReader.hpp"
#include "PdxRemoteWriter.hpp"
#include "PdxType.hpp"
#include "PdxTypeRegistry.hpp"
#include "PdxWriterWithTypeCollector.hpp"
#include "SerializationRegistry.hpp"
#include "ThinClientPoolDM.hpp"
#include "Utils.hpp"

namespace {
using apache::geode::client::IllegalStateException;
using apache::geode::client::PdxType;
using apache::geode::client::PdxTypeRegistry;
using apache::geode::client::Pool;
using apache::geode::client::SerializationRegistry;

std::shared_ptr<PdxType> checkAndFetchPdxType(
    Pool* pool, std::shared_ptr<PdxTypeRegistry>& pdxTypeRegistry,
    const std::shared_ptr<SerializationRegistry>& serializationRegistry,
    int32_t typeId) {
  auto pType = pdxTypeRegistry->getPdxType(typeId);
  if (pType != nullptr) {
    return pType;
  }

  pType = std::dynamic_pointer_cast<PdxType>(
      serializationRegistry->GetPDXTypeById(pool, typeId));
  pdxTypeRegistry->addLocalPdxType(pType->getPdxClassName(), pType);
  pdxTypeRegistry->addPdxType(pType->getTypeId(), pType);

  return pType;
}
}  // namespace

namespace apache {
namespace geode {
namespace client {

uint8_t PdxHelper::PdxHeader = 8;

PdxHelper::PdxHelper() {}

PdxHelper::~PdxHelper() {}

void PdxHelper::serializePdx(DataOutput& output, PdxInstanceImpl* instance) {
  auto cacheImpl = CacheRegionHelper::getCacheImpl(output.getCache());
  auto registry = cacheImpl->getPdxTypeRegistry();

  auto type = instance->getPdxType(DataOutputInternal::getPool(output));
  PdxLocalWriter plw{output, type, registry};
  instance->toData(plw);
  plw.endObjectWriting();  // now write typeid

  instance->updatePdxStream(plw.getPdxStream());
}

void PdxHelper::serializePdx(
    DataOutput& output, const std::shared_ptr<PdxSerializable>& pdxObject) {
  auto cacheImpl = CacheRegionHelper::getCacheImpl(output.getCache());
  auto pdxTypeRegistry = cacheImpl->getPdxTypeRegistry();
  auto& cachePerfStats = cacheImpl->getCachePerfStats();

  auto&& className = pdxObject->getClassName();
  auto localPdxType = pdxTypeRegistry->getLocalPdxType(className);

  if (localPdxType == nullptr) {
    // need to grab type info, as fromdata is not called yet

    PdxWriterWithTypeCollector ptc(output, className, pdxTypeRegistry);
    pdxObject->toData(ptc);
    auto nType = ptc.getPdxLocalType();

    nType->InitializeType();
    int32_t nTypeId = pdxTypeRegistry->getPDXIdForType(
        className.c_str(), DataOutputInternal::getPool(output), nType, true);
    nType->setTypeId(nTypeId);

    ptc.endObjectWriting();
    pdxTypeRegistry->addLocalPdxType(className, nType);
    pdxTypeRegistry->addPdxType(nTypeId, nType);

    if (cacheImpl != nullptr) {
      uint8_t* stPos = const_cast<uint8_t*>(output.getBuffer()) +
                       ptc.getStartPositionOffset();
      int pdxLen = PdxHelper::readInt32(stPos);
      cachePerfStats.incPdxSerialization(
          pdxLen + 1 + 2 * 4);  // pdxLen + 93 DSID + len + typeID
    }

  } else  // we know locasl type, need to see preerved data
  {
    // if object got from server than create instance of RemoteWriter otherwise
    // local writer.
    // now always remotewriter as we have API Read/WriteUnreadFields
    // so we don't know whether user has used those or not;; Can we do some
    // trick here?

    auto createPdxRemoteWriter = [&]() -> PdxRemoteWriter {
      if (auto pd = pdxTypeRegistry->getPreserveData(pdxObject)) {
        auto mergedPdxType = pdxTypeRegistry->getPdxType(pd->getMergedTypeId());
        return PdxRemoteWriter(output, mergedPdxType, pd, pdxTypeRegistry);
      } else {
        return PdxRemoteWriter(output, localPdxType, pdxTypeRegistry);
      }
    };

    PdxRemoteWriter prw = createPdxRemoteWriter();

    pdxObject->toData(prw);
    prw.endObjectWriting();

    //[ToDo] need to write bytes for stats
    if (cacheImpl != nullptr) {
      uint8_t* stPos = const_cast<uint8_t*>(output.getBuffer()) +
                       prw.getStartPositionOffset();
      int pdxLen = PdxHelper::readInt32(stPos);
      cachePerfStats.incPdxSerialization(
          pdxLen + 1 + 2 * 4);  // pdxLen + 93 DSID + len + typeID
    }
  }
}

std::shared_ptr<PdxSerializable> PdxHelper::deserializePdx(DataInput& dataInput,
                                                           int32_t typeId,
                                                           int32_t length) {
  std::shared_ptr<PdxSerializable> object;
  std::shared_ptr<PdxType> localType;

  auto cacheImpl = CacheRegionHelper::getCacheImpl(dataInput.getCache());
  auto pdxTypeRegistry = cacheImpl->getPdxTypeRegistry();
  auto serializationRegistry = cacheImpl->getSerializationRegistry();

  auto pType = pdxTypeRegistry->getPdxType(typeId);
  if (pType) {
    // this may happen with PdxInstanceFactory
    localType = pdxTypeRegistry->getLocalPdxType(pType->getPdxClassName());
  }
  if (pType && localType) {
    // type found
    auto&& pdxClassname = pType->getPdxClassName();
    LOGDEBUG("deserializePdx ClassName = " + pdxClassname +
             ", isLocal = " + std::to_string(pType->isLocal()));

    object = serializationRegistry->getPdxSerializableType(pdxClassname);
    if (pType->isLocal())  // local type no need to read Unread data
    {
      auto plr = PdxLocalReader(dataInput, pType, length, pdxTypeRegistry);
      object->fromData(plr);
      plr.moveStream();
    } else {
      auto prr = PdxRemoteReader(dataInput, pType, length, pdxTypeRegistry);
      object->fromData(prr);
      auto mergedVersion = pdxTypeRegistry->getMergedType(pType->getTypeId());

      auto preserveData = prr.getPreservedData(mergedVersion, object);
      if (preserveData != nullptr) {
        pdxTypeRegistry->setPreserveData(
            object, preserveData,
            cacheImpl
                ->getExpiryTaskManager());  // it will set data in weakhashmap
      }
      prr.moveStream();

      if (auto wrapper = std::dynamic_pointer_cast<PdxWrapper>(object)) {
        if (!wrapper->getObject()) {
          // No serializer was registered to deserialize this type.
          // Fall back to PdxInstance
          return nullptr;
        }
      }
    }
  } else {
    if (pType == nullptr) {
      pType = std::dynamic_pointer_cast<PdxType>(
          serializationRegistry->GetPDXTypeById(
              DataInputInternal::getPool(dataInput), typeId));
      localType = pdxTypeRegistry->getLocalPdxType(pType->getPdxClassName());
    }

    object =
        serializationRegistry->getPdxSerializableType(pType->getPdxClassName());
    if (!localType) {
      // need to know local type
      auto pdxRealObject = object;
      PdxReaderWithTypeCollector prtc{dataInput, pType, length,
                                      pdxTypeRegistry};
      object->fromData(prtc);
      if (auto pdxWrapper = std::dynamic_pointer_cast<PdxWrapper>(object)) {
        if (!pdxWrapper->getObject()) {
          // No serializer was registered to deserialize this type.
          // Fall back to PdxInstance
          return nullptr;
        }
      }

      // Check for the PdxWrapper
      localType = prtc.getLocalType();

      if (*pType == *localType) {
        pdxTypeRegistry->addLocalPdxType(pdxRealObject->getClassName(), pType);
        pdxTypeRegistry->addPdxType(pType->getTypeId(), pType);
        pType->setLocal(true);
      } else {
        // Need to know local type and then merge type
        localType->InitializeType();
        auto id = pdxTypeRegistry->getPDXIdForType(
            object->getClassName(), DataInputInternal::getPool(dataInput),
            localType, true);
        localType->setTypeId(id);
        localType->setLocal(true);
        pdxTypeRegistry->addLocalPdxType(pdxRealObject->getClassName(),
                                         localType);  // added local type
        pdxTypeRegistry->addPdxType(id, localType);

        pType->InitializeType();
        pdxTypeRegistry->addPdxType(pType->getTypeId(),
                                    pType);  // adding remote type

        // create merge type
        createMergedType(localType, pType, dataInput);

        auto mergedVersion = pdxTypeRegistry->getMergedType(pType->getTypeId());

        if (auto preserveData = prtc.getPreservedData(mergedVersion, object)) {
          pdxTypeRegistry->setPreserveData(object, preserveData,
                                           cacheImpl->getExpiryTaskManager());
        }
      }
      prtc.moveStream();
    } else {  // remote reader will come here as local type is there
      pType->InitializeType();
      LOGDEBUG("Adding type %d ", pType->getTypeId());
      pdxTypeRegistry->addPdxType(pType->getTypeId(),
                                  pType);  // adding remote type
      PdxRemoteReader prr{dataInput, pType, length, pdxTypeRegistry};
      object->fromData(prr);

      // Check for PdxWrapper to getObject.

      createMergedType(localType, pType, dataInput);

      auto mergedVersion = pdxTypeRegistry->getMergedType(pType->getTypeId());

      auto preserveData = prr.getPreservedData(mergedVersion, object);
      if (preserveData != nullptr) {
        pdxTypeRegistry->setPreserveData(object, preserveData,
                                         cacheImpl->getExpiryTaskManager());
      }

      prr.moveStream();
    }
  }

  return object;
}

std::shared_ptr<PdxSerializable> PdxHelper::deserializePdx(
    DataInput& dataInput) {
  auto cacheImpl = CacheRegionHelper::getCacheImpl(dataInput.getCache());
  auto pdxTypeRegistry = cacheImpl->getPdxTypeRegistry();
  auto serializationRegistry = cacheImpl->getSerializationRegistry();
  auto& cachePerfStats = cacheImpl->getCachePerfStats();

  const auto len = dataInput.readInt32();
  const auto typeId = dataInput.readInt32();

  if (!pdxTypeRegistry->getPdxReadSerialized()) {
    const auto pos = dataInput.currentBufferPosition();
    if (auto pdxObject = PdxHelper::deserializePdx(dataInput, typeId, len)) {
      cachePerfStats.incPdxDeSerialization(len + 9);  // pdxLen + 1 + 2*4
      return pdxObject;
    }
    dataInput.rewindCursor(dataInput.currentBufferPosition() - pos);
  }

  auto pType =
      checkAndFetchPdxType(DataInputInternal::getPool(dataInput),
                           pdxTypeRegistry, serializationRegistry, typeId);

  auto object = std::make_shared<PdxInstanceImpl>(
      const_cast<uint8_t*>(dataInput.currentBufferPosition()), len, pType,
      cachePerfStats, *pdxTypeRegistry, *cacheImpl,
      cacheImpl->getDistributedSystem()
          .getSystemProperties()
          .getEnableTimeStatistics());

  dataInput.advanceCursor(len);
  return std::move(object);
}

void PdxHelper::createMergedType(std::shared_ptr<PdxType> localType,
                                 std::shared_ptr<PdxType> remoteType,
                                 DataInput& dataInput) {
  auto mergedVersion = localType->mergeVersion(remoteType);
  auto cacheImpl = CacheRegionHelper::getCacheImpl(dataInput.getCache());
  auto pdxTypeRegistry = cacheImpl->getPdxTypeRegistry();
  auto serializaionRegistry = cacheImpl->getSerializationRegistry();

  if (*mergedVersion == *localType) {
    pdxTypeRegistry->setMergedType(remoteType->getTypeId(), localType);
  } else if (*mergedVersion == *remoteType) {
    pdxTypeRegistry->setMergedType(remoteType->getTypeId(), remoteType);
  } else {  // need to create new version
    mergedVersion->InitializeType();
    if (mergedVersion->getTypeId() == 0) {
      mergedVersion->setTypeId(serializaionRegistry->GetPDXIdForType(
          DataInputInternal::getPool(dataInput), mergedVersion));
    }

    // PdxTypeRegistry::AddPdxType(remoteType->TypeId, mergedVersion);
    pdxTypeRegistry->addPdxType(mergedVersion->getTypeId(), mergedVersion);
    pdxTypeRegistry->setMergedType(remoteType->getTypeId(), mergedVersion);
    pdxTypeRegistry->setMergedType(mergedVersion->getTypeId(), mergedVersion);
  }
}

int32_t PdxHelper::readInt32(uint8_t* offsetPosition) {
  int32_t data = offsetPosition[0];
  data = (data << 8) | offsetPosition[1];
  data = (data << 8) | offsetPosition[2];
  data = (data << 8) | offsetPosition[3];

  return data;
}

void PdxHelper::writeInt32(uint8_t* offsetPosition, int32_t value) {
  offsetPosition[0] = static_cast<uint8_t>(value >> 24);
  offsetPosition[1] = static_cast<uint8_t>(value >> 16);
  offsetPosition[2] = static_cast<uint8_t>(value >> 8);
  offsetPosition[3] = static_cast<uint8_t>(value);
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

void PdxHelper::writeInt16(uint8_t* offsetPosition, int32_t value) {
  int16_t val = static_cast<int16_t>(value);
  offsetPosition[0] = static_cast<uint8_t>(val >> 8);
  offsetPosition[1] = static_cast<uint8_t>(val);
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
