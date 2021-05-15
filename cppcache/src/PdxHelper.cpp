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

namespace apache {
namespace geode {
namespace client {

uint8_t PdxHelper::PdxHeader = 8;

PdxHelper::PdxHelper() {}

PdxHelper::~PdxHelper() {}

void PdxHelper::serializePdxWithRetries(
    DataOutput& output, const std::shared_ptr<PdxSerializable>& pdxObject,
    int maxRetries) {
  auto retries = 0;
  auto before = output.getCursor();

  for (;;) {
    try {
      serializePdx(output, pdxObject);
      break;
    } catch (UnknownPdxTypeException&) {
      if (retries++ >= maxRetries) {
        throw;
      } else {
        if (auto instance =
                std::dynamic_pointer_cast<PdxInstanceImpl>(pdxObject)) {
          instance->setPdxId(0);
        }

        output.advanceCursor(before - output.getCursor());
        LOG_DEBUG("Retrying PDX serialization due to PDX unknown exception");
      }
    }
  }
}

void PdxHelper::serializePdx(
    DataOutput& output, const std::shared_ptr<PdxSerializable>& pdxObject) {
  auto pdxII = std::dynamic_pointer_cast<PdxInstanceImpl>(pdxObject);
  auto cacheImpl = CacheRegionHelper::getCacheImpl(output.getCache());
  auto pdxTypeRegistry = cacheImpl->getPdxTypeRegistry();
  auto& cachePerfStats = cacheImpl->getCachePerfStats();

  if (pdxII != nullptr) {
    auto piPt = pdxII->getPdxType();
    if (piPt != nullptr &&
        piPt->getTypeId() ==
            0)  // from pdxInstance factory need to get typeid from server
    {
      int typeId = pdxTypeRegistry->getPDXIdForType(
          piPt, DataOutputInternal::getPool(output));
      pdxII->setPdxId(typeId);
    }
    auto plw = PdxLocalWriter(output, piPt, pdxTypeRegistry);
    pdxII->toData(plw);
    plw.endObjectWriting();  // now write typeid
    int len = 0;
    uint8_t* pdxStream = plw.getPdxStream(len);
    pdxII->updatePdxStream(pdxStream, len);

    delete[] pdxStream;

    return;
  }

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
        return PdxRemoteWriter(output, className, pdxTypeRegistry);
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
  std::shared_ptr<PdxSerializable> pdxObjectptr = nullptr;
  std::shared_ptr<PdxType> pdxLocalType = nullptr;

  auto cacheImpl = CacheRegionHelper::getCacheImpl(dataInput.getCache());
  auto pdxTypeRegistry = cacheImpl->getPdxTypeRegistry();
  auto serializationRegistry = cacheImpl->getSerializationRegistry();

  auto pType = pdxTypeRegistry->getPdxType(typeId);
  if (pType) {
    // this may happen with PdxInstanceFactory
    pdxLocalType = pdxTypeRegistry->getLocalPdxType(pType->getPdxClassName());
  }
  if (pType && pdxLocalType) {
    // type found
    auto&& pdxClassname = pType->getPdxClassName();
    LOG_DEBUG("deserializePdx ClassName = " + pdxClassname +
              ", isLocal = " + std::to_string(pType->isLocal()));

    pdxObjectptr = serializationRegistry->getPdxSerializableType(pdxClassname);
    if (pType->isLocal())  // local type no need to read Unread data
    {
      auto plr = PdxLocalReader(dataInput, pType, length, pdxTypeRegistry);
      pdxObjectptr->fromData(plr);
      plr.moveStream();
    } else {
      auto prr = PdxRemoteReader(dataInput, pType, length, pdxTypeRegistry);
      pdxObjectptr->fromData(prr);
      auto mergedVersion = pdxTypeRegistry->getMergedType(pType->getTypeId());

      auto preserveData = prr.getPreservedData(mergedVersion, pdxObjectptr);
      if (preserveData != nullptr) {
        pdxTypeRegistry->setPreserveData(
            pdxObjectptr, preserveData,
            cacheImpl
                ->getExpiryTaskManager());  // it will set data in weakhashmap
      }
      prr.moveStream();

      if (auto pdxWrapper =
              std::dynamic_pointer_cast<PdxWrapper>(pdxObjectptr)) {
        if (!pdxWrapper->getObject()) {
          // No serializer was registered to deserialize this type.
          // Fall back to PdxInstance
          return nullptr;
        }
      }
    }
  } else {
    // type not found; need to get from server
    if (!pType) {
      pType = std::dynamic_pointer_cast<PdxType>(
          serializationRegistry->GetPDXTypeById(
              DataInputInternal::getPool(dataInput), typeId));
      pdxLocalType = pdxTypeRegistry->getLocalPdxType(pType->getPdxClassName());
    }
    pdxObjectptr =
        serializationRegistry->getPdxSerializableType(pType->getPdxClassName());
    if (!pdxLocalType) {
      // need to know local type
      auto pdxRealObject = pdxObjectptr;
      auto prtc =
          PdxReaderWithTypeCollector(dataInput, pType, length, pdxTypeRegistry);
      pdxObjectptr->fromData(prtc);
      if (auto pdxWrapper =
              std::dynamic_pointer_cast<PdxWrapper>(pdxObjectptr)) {
        if (!pdxWrapper->getObject()) {
          // No serializer was registered to deserialize this type.
          // Fall back to PdxInstance
          return nullptr;
        }
      }

      // Check for the PdxWrapper
      pdxLocalType = prtc.getLocalType();

      if (*pType == *pdxLocalType) {
        pdxTypeRegistry->addLocalPdxType(pdxRealObject->getClassName(), pType);
        pdxTypeRegistry->addPdxType(pType->getTypeId(), pType);
        pType->setLocal(true);
      } else {
        // Need to know local type and then merge type
        pdxLocalType->InitializeType();
        pdxLocalType->setTypeId(pdxTypeRegistry->getPDXIdForType(
            pdxObjectptr->getClassName(), DataInputInternal::getPool(dataInput),
            pdxLocalType, true));
        pdxLocalType->setLocal(true);
        pdxTypeRegistry->addLocalPdxType(pdxRealObject->getClassName(),
                                         pdxLocalType);  // added local type
        pdxTypeRegistry->addPdxType(pdxLocalType->getTypeId(), pdxLocalType);

        pType->InitializeType();
        pdxTypeRegistry->addPdxType(pType->getTypeId(),
                                    pType);  // adding remote type

        // create merge type
        createMergedType(pdxLocalType, pType, dataInput);

        auto mergedVersion = pdxTypeRegistry->getMergedType(pType->getTypeId());

        if (auto preserveData =
                prtc.getPreservedData(mergedVersion, pdxObjectptr)) {
          pdxTypeRegistry->setPreserveData(pdxObjectptr, preserveData,
                                           cacheImpl->getExpiryTaskManager());
        }
      }
      prtc.moveStream();
    } else {  // remote reader will come here as local type is there
      pType->InitializeType();
      LOG_DEBUG("Adding type %d ", pType->getTypeId());
      pdxTypeRegistry->addPdxType(pType->getTypeId(),
                                  pType);  // adding remote type
      auto prr = PdxRemoteReader(dataInput, pType, length, pdxTypeRegistry);
      pdxObjectptr->fromData(prr);

      // Check for PdxWrapper to getObject.

      createMergedType(pdxLocalType, pType, dataInput);

      auto mergedVersion = pdxTypeRegistry->getMergedType(pType->getTypeId());

      auto preserveData = prr.getPreservedData(mergedVersion, pdxObjectptr);
      if (preserveData != nullptr) {
        pdxTypeRegistry->setPreserveData(pdxObjectptr, preserveData,
                                         cacheImpl->getExpiryTaskManager());
      }
      prr.moveStream();
    }
  }
  return pdxObjectptr;
}

std::shared_ptr<PdxSerializable> PdxHelper::deserializePdx(
    DataInput& dataInput, bool forceDeserialize) {
  auto cacheImpl = CacheRegionHelper::getCacheImpl(dataInput.getCache());
  auto pdxTypeRegistry = cacheImpl->getPdxTypeRegistry();
  auto serializationRegistry = cacheImpl->getSerializationRegistry();
  auto& cachePerfStats = cacheImpl->getCachePerfStats();

  const auto len = dataInput.readInt32();
  const auto typeId = dataInput.readInt32();

  if (!pdxTypeRegistry->getPdxReadSerialized() || forceDeserialize) {
    const auto pos = dataInput.currentBufferPosition();
    if (auto pdxObject = PdxHelper::deserializePdx(dataInput, typeId, len)) {
      cachePerfStats.incPdxDeSerialization(len + 9);  // pdxLen + 1 + 2*4
      return pdxObject;
    }
    dataInput.rewindCursor(dataInput.currentBufferPosition() - pos);
  }

  checkAndFetchPdxType(DataInputInternal::getPool(dataInput), pdxTypeRegistry,
                       serializationRegistry, typeId);

  cachePerfStats.incPdxInstanceCreations();

  auto pdxObject = std::make_shared<PdxInstanceImpl>(
      const_cast<uint8_t*>(dataInput.currentBufferPosition()), len, typeId,
      cachePerfStats, *pdxTypeRegistry, *cacheImpl,
      cacheImpl->getDistributedSystem()
          .getSystemProperties()
          .getEnableTimeStatistics());

  dataInput.advanceCursor(len);

  return std::move(pdxObject);
}
void PdxHelper::checkAndFetchPdxType(
    Pool* pool, std::shared_ptr<PdxTypeRegistry>& pdxTypeRegistry,
    const std::shared_ptr<SerializationRegistry>& serializationRegistry,
    int32_t typeId) {
  if (!pdxTypeRegistry->getPdxType(typeId)) {
    auto pType = std::dynamic_pointer_cast<PdxType>(
        serializationRegistry->GetPDXTypeById(pool, typeId));
    pdxTypeRegistry->addLocalPdxType(pType->getPdxClassName(), pType);
    pdxTypeRegistry->addPdxType(pType->getTypeId(), pType);
  }
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
