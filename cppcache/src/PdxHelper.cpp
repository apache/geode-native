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
/*
 * PdxHelper.cpp
 *
 *  Created on: Dec 13, 2011
 *      Author: npatel
 */

#include "PdxHelper.hpp"
#include "PdxTypeRegistry.hpp"
#include "PdxWriterWithTypeCollector.hpp"
#include "SerializationRegistry.hpp"
#include "PdxLocalReader.hpp"
#include "PdxRemoteReader.hpp"
#include "PdxType.hpp"
#include "PdxReaderWithTypeCollector.hpp"
#include "PdxInstanceImpl.hpp"
#include "Utils.hpp"
#include "PdxRemoteWriter.hpp"
#include "CacheRegionHelper.hpp"
#include "ThinClientPoolDM.hpp"
#include <geode/Cache.hpp>
#include <geode/DataInput.hpp>
#include <geode/PoolManager.hpp>

namespace apache {
namespace geode {
namespace client {
uint8_t PdxHelper::PdxHeader = 8;

PdxHelper::PdxHelper() {}

PdxHelper::~PdxHelper() {}

void PdxHelper::serializePdx(DataOutput& output,
                             const PdxSerializable& pdxObject) {
  serializePdx(
      output,
      std::static_pointer_cast<PdxSerializable>(
          std::const_pointer_cast<Serializable>(pdxObject.shared_from_this())));
}

void PdxHelper::serializePdx(DataOutput& output,
                             const PdxSerializablePtr& pdxObject) {
  const char* pdxClassname = nullptr;

  auto pdxII = std::dynamic_pointer_cast<PdxInstanceImpl>(pdxObject);
  auto cacheImpl = CacheRegionHelper::getCacheImpl(output.getCache());
  auto pdxTypeRegistry = cacheImpl->getPdxTypeRegistry();
  auto& cachePerfStats = cacheImpl->getCachePerfStats();

  if (pdxII != nullptr) {
    PdxTypePtr piPt = pdxII->getPdxType();
    if (piPt != nullptr &&
        piPt->getTypeId() ==
            0)  // from pdxInstance factory need to get typeid from server
    {
      int typeId = pdxTypeRegistry->getPDXIdForType(piPt, output.getPoolName());
      pdxII->setPdxId(typeId);
    }
    auto plw = std::make_shared<PdxLocalWriter>(output, piPt, pdxTypeRegistry);
    pdxII->toData(plw);
    plw->endObjectWriting();  // now write typeid
    int len = 0;
    uint8_t* pdxStream = plw->getPdxStream(len);
    pdxII->updatePdxStream(pdxStream, len);

    delete[] pdxStream;

    return;
  }

  const char* pdxType = pdxObject->getClassName();
  pdxClassname = pdxType;
  PdxTypePtr localPdxType = pdxTypeRegistry->getLocalPdxType(pdxType);

  if (localPdxType == nullptr) {
    // need to grab type info, as fromdata is not called yet

    PdxWriterWithTypeCollectorPtr ptc =
        std::make_shared<PdxWriterWithTypeCollector>(output, pdxType,
                                                     pdxTypeRegistry);
    pdxObject->toData(std::dynamic_pointer_cast<PdxWriter>(ptc));
    PdxTypePtr nType = ptc->getPdxLocalType();

    nType->InitializeType();
    int32_t nTypeId = pdxTypeRegistry->getPDXIdForType(
        pdxType, output.getPoolName(), nType, true);
    nType->setTypeId(nTypeId);

    ptc->endObjectWriting();
    pdxTypeRegistry->addLocalPdxType(pdxType, nType);
    pdxTypeRegistry->addPdxType(nTypeId, nType);

    if (cacheImpl != nullptr) {
      uint8_t* stPos = const_cast<uint8_t*>(output.getBuffer()) +
                       ptc->getStartPositionOffset();
      int pdxLen = PdxHelper::readInt32(stPos);
      cachePerfStats.incPdxSerialization(
          pdxLen + 1 + 2 * 4);  // pdxLen + 93 DSID + len + typeID
    }

  } else  // we know locasl type, need to see preerved data
  {
    // if object got from server than create instance of RemoteWriter otherwise
    // local writer.

    PdxRemotePreservedDataPtr pd = pdxTypeRegistry->getPreserveData(pdxObject);

    // now always remotewriter as we have API Read/WriteUnreadFields
    // so we don't know whether user has used those or not;; Can we do some
    // trick here?
    PdxRemoteWriterPtr prw = nullptr;

    if (pd != nullptr) {
      PdxTypePtr mergedPdxType =
          pdxTypeRegistry->getPdxType(pd->getMergedTypeId());
      prw = std::make_shared<PdxRemoteWriter>(output, mergedPdxType, pd,
                                              pdxTypeRegistry);
    } else {
      prw = std::make_shared<PdxRemoteWriter>(output, pdxClassname,
                                              pdxTypeRegistry);
    }
    pdxObject->toData(std::dynamic_pointer_cast<PdxWriter>(prw));
    prw->endObjectWriting();

    //[ToDo] need to write bytes for stats
    if (cacheImpl != nullptr) {
      uint8_t* stPos = const_cast<uint8_t*>(output.getBuffer()) +
                       prw->getStartPositionOffset();
      int pdxLen = PdxHelper::readInt32(stPos);
      cachePerfStats.incPdxSerialization(
          pdxLen + 1 + 2 * 4);  // pdxLen + 93 DSID + len + typeID
    }
  }
}

PdxSerializablePtr PdxHelper::deserializePdx(DataInput& dataInput,
                                             bool forceDeserialize,
                                             int32_t typeId, int32_t length) {
  char* pdxClassname = nullptr;
  PdxSerializablePtr pdxObjectptr = nullptr;
  PdxTypePtr pdxLocalType = nullptr;

  auto cacheImpl = CacheRegionHelper::getCacheImpl(dataInput.getCache());
  auto pdxTypeRegistry = cacheImpl->getPdxTypeRegistry();
  auto serializationRegistry = cacheImpl->getSerializationRegistry();

  PdxTypePtr pType = pdxTypeRegistry->getPdxType(typeId);
  if (pType != nullptr) {  // this may happen with PdxInstanceFactory {
    pdxLocalType = pdxTypeRegistry->getLocalPdxType(
        pType->getPdxClassName());  // this should be fine for IPdxTypeMapper
  }
  if (pType != nullptr && pdxLocalType != nullptr)  // type found
  {
    pdxClassname = pType->getPdxClassName();
    LOGDEBUG("deserializePdx ClassName = %s, isLocal = %d ",
             pType->getPdxClassName(), pType->isLocal());

    pdxObjectptr = serializationRegistry->getPdxType(pdxClassname);
    if (pType->isLocal())  // local type no need to read Unread data
    {
      PdxLocalReaderPtr plr = std::make_shared<PdxLocalReader>(
          dataInput, pType, length, pdxTypeRegistry);
      pdxObjectptr->fromData(std::dynamic_pointer_cast<PdxReader>(plr));
      plr->MoveStream();
    } else {
      PdxRemoteReaderPtr prr = std::make_shared<PdxRemoteReader>(
          dataInput, pType, length, pdxTypeRegistry);
      pdxObjectptr->fromData(std::dynamic_pointer_cast<PdxReader>(prr));
      PdxTypePtr mergedVersion =
          pdxTypeRegistry->getMergedType(pType->getTypeId());

      PdxRemotePreservedDataPtr preserveData =
          prr->getPreservedData(mergedVersion, pdxObjectptr);
      if (preserveData != nullptr) {
        pdxTypeRegistry->setPreserveData(
            pdxObjectptr, preserveData,
            cacheImpl
                ->getExpiryTaskManager());  // it will set data in weakhashmap
      }
      prr->MoveStream();
    }
  } else {
    // type not found; need to get from server
    if (pType == nullptr) {
      pType = std::static_pointer_cast<PdxType>(
          serializationRegistry->GetPDXTypeById(
              cacheImpl->getCache()->getPoolManager().find(
                  dataInput.getPoolName()),
              typeId));
      pdxLocalType = pdxTypeRegistry->getLocalPdxType(pType->getPdxClassName());
    }
    /* adongre  - Coverity II
     * CID 29298: Unused pointer value (UNUSED_VALUE)
     * Pointer "pdxClassname" returned by "pType->getPdxClassName()" is never
     * used.
     * Fix : Commented the line
     */
    // pdxClassname = pType->getPdxClassName();
    pdxObjectptr = serializationRegistry->getPdxType(pType->getPdxClassName());
    PdxSerializablePtr pdxRealObject = pdxObjectptr;
    if (pdxLocalType == nullptr)  // need to know local type
    {
      PdxReaderWithTypeCollectorPtr prtc =
          std::make_shared<PdxReaderWithTypeCollector>(dataInput, pType, length,
                                                       pdxTypeRegistry);
      pdxObjectptr->fromData(std::dynamic_pointer_cast<PdxReader>(prtc));

      // Check for the PdxWrapper

      pdxLocalType = prtc->getLocalType();

      if (pType->Equals(pdxLocalType)) {
        pdxTypeRegistry->addLocalPdxType(pdxRealObject->getClassName(), pType);
        pdxTypeRegistry->addPdxType(pType->getTypeId(), pType);
        pType->setLocal(true);
      } else {
        // Need to know local type and then merge type
        pdxLocalType->InitializeType();
        pdxLocalType->setTypeId(pdxTypeRegistry->getPDXIdForType(
            pdxObjectptr->getClassName(), dataInput.getPoolName(), pdxLocalType,
            true));
        pdxLocalType->setLocal(true);
        pdxTypeRegistry->addLocalPdxType(pdxRealObject->getClassName(),
                                         pdxLocalType);  // added local type
        pdxTypeRegistry->addPdxType(pdxLocalType->getTypeId(), pdxLocalType);

        pType->InitializeType();
        pdxTypeRegistry->addPdxType(pType->getTypeId(),
                                    pType);  // adding remote type

        // create merge type
        createMergedType(pdxLocalType, pType, dataInput);

        PdxTypePtr mergedVersion =
            pdxTypeRegistry->getMergedType(pType->getTypeId());

        PdxRemotePreservedDataPtr preserveData =
            prtc->getPreservedData(mergedVersion, pdxObjectptr);
        if (preserveData != nullptr) {
          pdxTypeRegistry->setPreserveData(pdxObjectptr, preserveData,
                                           cacheImpl->getExpiryTaskManager());
        }
      }
      prtc->MoveStream();
    } else {  // remote reader will come here as local type is there
      pType->InitializeType();
      LOGDEBUG("Adding type %d ", pType->getTypeId());
      pdxTypeRegistry->addPdxType(pType->getTypeId(),
                                  pType);  // adding remote type
      PdxRemoteReaderPtr prr = std::make_shared<PdxRemoteReader>(
          dataInput, pType, length, pdxTypeRegistry);
      pdxObjectptr->fromData(std::dynamic_pointer_cast<PdxReader>(prr));

      // Check for PdxWrapper to getObject.

      createMergedType(pdxLocalType, pType, dataInput);

      PdxTypePtr mergedVersion =
          pdxTypeRegistry->getMergedType(pType->getTypeId());

      PdxRemotePreservedDataPtr preserveData =
          prr->getPreservedData(mergedVersion, pdxObjectptr);
      if (preserveData != nullptr) {
        pdxTypeRegistry->setPreserveData(pdxObjectptr, preserveData,
                                         cacheImpl->getExpiryTaskManager());
      }
      prr->MoveStream();
    }
  }
  return pdxObjectptr;
}

PdxSerializablePtr PdxHelper::deserializePdx(DataInput& dataInput,
                                             bool forceDeserialize) {
  auto cacheImpl = CacheRegionHelper::getCacheImpl(dataInput.getCache());
  auto pdxTypeRegistry = cacheImpl->getPdxTypeRegistry();
  auto serializationRegistry = cacheImpl->getSerializationRegistry();
  auto& cachePerfStats = cacheImpl->getCachePerfStats();
  if (pdxTypeRegistry->getPdxReadSerialized() == false || forceDeserialize) {
    // Read Length
    int32_t len = dataInput.readInt32();

    int32_t typeId = dataInput.readInt32();

    cachePerfStats.incPdxDeSerialization(len + 9);  // pdxLen + 1 + 2*4

    return PdxHelper::deserializePdx(dataInput, forceDeserialize,
                                     (int32_t)typeId, (int32_t)len);

  } else {
    // Read Length
    int32_t len = dataInput.readInt32();

    int typeId = dataInput.readInt32();

    auto pType = pdxTypeRegistry->getPdxType(typeId);

    if (pType == nullptr) {
      // TODO shared_ptr why redef?
      auto pType = std::static_pointer_cast<PdxType>(
          serializationRegistry->GetPDXTypeById(
              cacheImpl->getCache()->getPoolManager().find(
                  dataInput.getPoolName()),
              typeId));
      pdxTypeRegistry->addLocalPdxType(pType->getPdxClassName(), pType);
      pdxTypeRegistry->addPdxType(pType->getTypeId(), pType);
    }

    cachePerfStats.incPdxInstanceCreations();

    // TODO::Enable it once the PdxInstanceImple is CheckedIn.
    auto pdxObject = std::make_shared<PdxInstanceImpl>(
        const_cast<uint8_t*>(dataInput.currentBufferPosition()), len, typeId,
        &cachePerfStats, pdxTypeRegistry, dataInput.getCache(),
        cacheImpl->getDistributedSystem()
            .getSystemProperties()
            .getEnableTimeStatistics());

    dataInput.advanceCursor(len);

    return pdxObject;
  }
}

void PdxHelper::createMergedType(PdxTypePtr localType, PdxTypePtr remoteType,
                                 DataInput& dataInput) {
  PdxTypePtr mergedVersion = localType->mergeVersion(remoteType);
  auto cacheImpl = CacheRegionHelper::getCacheImpl(dataInput.getCache());
  auto pdxTypeRegistry = cacheImpl->getPdxTypeRegistry();
  auto serializaionRegistry = cacheImpl->getSerializationRegistry();

  if (mergedVersion->Equals(localType)) {
    pdxTypeRegistry->setMergedType(remoteType->getTypeId(), localType);
  } else if (mergedVersion->Equals(remoteType)) {
    pdxTypeRegistry->setMergedType(remoteType->getTypeId(), remoteType);
  } else {  // need to create new version
    mergedVersion->InitializeType();
    if (mergedVersion->getTypeId() == 0) {
      mergedVersion->setTypeId(serializaionRegistry->GetPDXIdForType(
          dataInput.getCache()->getPoolManager().find(dataInput.getPoolName()),
          mergedVersion));
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

int32_t PdxHelper::getEnumValue(const char* enumClassName, const char* enumName,
                                int hashcode,
                                PdxTypeRegistryPtr pdxTypeRegistry) {
  const auto& ei =
      std::make_shared<EnumInfo>(enumClassName, enumName, hashcode);
  return pdxTypeRegistry->getEnumValue(ei);
}

EnumInfoPtr PdxHelper::getEnum(int enumId, PdxTypeRegistryPtr pdxTypeRegistry) {
  const auto& ei = pdxTypeRegistry->getEnum(enumId);
  return ei;
}
}  // namespace client
}  // namespace geode
}  // namespace apache
