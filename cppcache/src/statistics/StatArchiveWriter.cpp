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

#include "StatArchiveWriter.hpp"

#include <chrono>
#include <ctime>

#include <ace/ACE.h>
#include <ace/OS_NS_sys_time.h>
#include <ace/OS_NS_sys_utsname.h>
#include <ace/OS_NS_time.h>
#include <ace/Task.h>
#include <ace/Thread_Mutex.h>

#include <geode/internal/geode_globals.hpp>

#include "../CacheImpl.hpp"
#include "../util/chrono/time_point.hpp"
#include "GeodeStatisticsFactory.hpp"
#include "HostStatSampler.hpp"

namespace apache {
namespace geode {
namespace statistics {

using std::chrono::duration_cast;
using std::chrono::milliseconds;
using std::chrono::nanoseconds;
using std::chrono::steady_clock;
using std::chrono::system_clock;

using client::GeodeIOException;
using client::IllegalArgumentException;
using client::NullPointerException;

// Constructor and Member functions of StatDataOutput class

StatDataOutput::StatDataOutput(CacheImpl *cacheImpl)
    : bytesWritten(0), m_fp(nullptr), closed(false) {
  dataBuffer = std::unique_ptr<DataOutput>(
      new DataOutput(cacheImpl->createDataOutput()));
}

StatDataOutput::StatDataOutput(std::string filename, CacheImpl *cacheImpl) {
  if (filename.length() == 0) {
    throw IllegalArgumentException("undefined archive file name");
  }

  dataBuffer = std::unique_ptr<DataOutput>(
      new DataOutput(cacheImpl->createDataOutput()));
  outFile = filename;
  closed = false;
  bytesWritten = 0;
  m_fp = fopen(outFile.c_str(), "a+b");
  if (m_fp == nullptr) {
    throw NullPointerException("error in opening archive file for writing");
  }
}

StatDataOutput::~StatDataOutput() {
  if (!closed && m_fp != nullptr) {
    fclose(m_fp);
  }
}

int64_t StatDataOutput::getBytesWritten() { return this->bytesWritten; }

void StatDataOutput::flush() {
  const uint8_t *buffBegin = dataBuffer->getBuffer();
  if (buffBegin == nullptr) {
    throw NullPointerException("undefined stat data buffer beginning");
  }
  const uint8_t *buffEnd = dataBuffer->getCursor();
  if (buffEnd == nullptr) {
    throw NullPointerException("undefined stat data buffer end");
  }
  int32_t sizeOfUInt8 = sizeof(uint8_t);
  int32_t len = static_cast<int32_t>(buffEnd - buffBegin);

  if (len > 0) {
    if (fwrite(buffBegin, sizeOfUInt8, len, m_fp) != static_cast<size_t>(len)) {
      LOGERROR("Could not write into the statistics file");
      throw GeodeIOException("Could not write into the statistics file");
    }
  }
  int rVal = fflush(m_fp);
  if (rVal != 0) {
    LOGERROR("Could not flush into the statistics file");
    throw GeodeIOException("Could not flush into the statistics file");
  }
}

void StatDataOutput::resetBuffer() {
  dataBuffer->reset();
  bytesWritten = 0;
}

void StatDataOutput::writeByte(int8_t v) {
  dataBuffer->write(v);
  bytesWritten += 1;
}

void StatDataOutput::writeBoolean(int8_t v) { writeByte(v); }

void StatDataOutput::writeShort(int16_t v) {
  dataBuffer->writeInt(v);
  bytesWritten += 2;
}

void StatDataOutput::writeInt(int32_t v) {
  dataBuffer->writeInt(v);
  bytesWritten += 4;
}

void StatDataOutput::writeLong(int64_t v) {
  dataBuffer->writeInt(v);
  bytesWritten += 8;
}

void StatDataOutput::writeUTF(std::string s) {
  size_t len = s.length();
  dataBuffer->writeUTF(s);
  bytesWritten += len;
}

void StatDataOutput::close() {
  fclose(m_fp);
  m_fp = nullptr;
  closed = true;
}

void StatDataOutput::openFile(std::string filename, int64_t size) {
  m_fp = fopen(filename.c_str(), "a+b");
  if (m_fp == nullptr) {
    throw NullPointerException("error in opening archive file for writing");
  }
  closed = false;
  bytesWritten = size;
}

// Constructor and Member functions of ResourceType class

ResourceType::ResourceType(int32_t idArg, const StatisticsType *typeArg)
    : type(typeArg) {
  this->id = idArg;
}

int32_t ResourceType::getId() const { return this->id; }

size_t ResourceType::getNumOfDescriptors() const {
  return this->type->getDescriptorsCount();
}

const std::vector<std::shared_ptr<StatisticDescriptor>>
    &ResourceType::getStats() const {
  return this->type->getStatistics();
}

// Constructor and Member functions of ResourceInst class

ResourceInst::ResourceInst(int32_t idArg, Statistics *resourceArg,
                           const ResourceType *typeArg,
                           StatDataOutput *dataOutArg)
    : type(typeArg) {
  id = idArg;
  resource = resourceArg;
  dataOut = dataOutArg;
  auto cnt = type->getNumOfDescriptors();
  archivedStatValues = new int64_t[cnt];
  // initialize to zero
  for (decltype(cnt) i = 0; i < cnt; i++) {
    archivedStatValues[i] = 0;
  }
  firstTime = true;
}

ResourceInst::~ResourceInst() { delete[] archivedStatValues; }

int32_t ResourceInst::getId() { return this->id; }

Statistics *ResourceInst::getResource() { return this->resource; }

const ResourceType *ResourceInst::getType() const { return this->type; }

int64_t ResourceInst::getStatValue(std::shared_ptr<StatisticDescriptor> f) {
  return this->resource->getRawBits(f);
}

void ResourceInst::writeSample() {
  bool wroteInstId = false;
  bool checkForChange = true;
  auto &stats = type->getStats();
  if (resource->isClosed()) {
    return;
  }
  if (firstTime) {
    firstTime = false;
    checkForChange = false;
  }
  auto count = type->getNumOfDescriptors();
  for (decltype(count) i = 0; i < count; i++) {
    int64_t value = getStatValue(stats[i]);
    if (!checkForChange || value != archivedStatValues[i]) {
      int64_t delta = value - archivedStatValues[i];
      archivedStatValues[i] = value;
      if (!wroteInstId) {
        wroteInstId = true;
        writeResourceInst(dataOut, id);
      }
      dataOut->writeByte(static_cast<int8_t>(i));
      writeStatValue(stats[i], delta);
    }
  }
  if (wroteInstId) {
    dataOut->writeByte(static_cast<unsigned char>(ILLEGAL_STAT_OFFSET));
  }
}

void ResourceInst::writeStatValue(std::shared_ptr<StatisticDescriptor> sd,
                                  int64_t v) {
  auto sdImpl = std::static_pointer_cast<StatisticDescriptorImpl>(sd);
  if (!sdImpl) {
    throw NullPointerException("could not downcast to StatisticDescriptorImpl");
  }
  FieldType typeCode = sdImpl->getTypeCode();

  switch (typeCode) {
    case INT_TYPE:
    case LONG_TYPE:
    //   case GF_FIELDTYPE_FLOAT:
    case DOUBLE_TYPE:
      writeCompactValue(v);
      break;
    default:
      throw IllegalArgumentException("Unexpected type code");
  }
}

void ResourceInst::writeCompactValue(int64_t v) {
  if (v <= MAX_1BYTE_COMPACT_VALUE && v >= MIN_1BYTE_COMPACT_VALUE) {
    this->dataOut->writeByte(static_cast<int8_t>(v));
  } else if (v <= MAX_2BYTE_COMPACT_VALUE && v >= MIN_2BYTE_COMPACT_VALUE) {
    this->dataOut->writeByte(COMPACT_VALUE_2_TOKEN);
    this->dataOut->writeShort(static_cast<int16_t>(v));
  } else {
    int8_t buffer[8];
    int32_t idx = 0;
    if (v < 0) {
      while (v != -1 && v != 0) {
        buffer[idx++] = static_cast<int8_t>(v & 0xFF);
        v >>= 8;
      }
      // On windows v goes to zero somtimes; seems like a bug
      if (v == 0) {
        // when this happens we end up with a bunch of -1 bytes
        // so strip off the high order ones
        while (0 < idx && buffer[idx - 1] == -1) {
          idx--;
        }
      }
      if (0 < idx && (buffer[idx - 1] & 0x80) == 0) {
        /* If the most significant byte does not have its high order bit set
         * then add a -1 byte so we know this is a negative number
         */
        buffer[idx++] = -1;
      }
    } else {
      while (v != 0) {
        buffer[idx++] = static_cast<int8_t>(v & 0xFF);
        v >>= 8;
      }
      if ((buffer[idx - 1] & 0x80) != 0) {
        /* If the most significant byte has its high order bit set
         * then add a zero byte so we know this is a positive number
         */
        buffer[idx++] = 0;
      }
    }
    int8_t token = COMPACT_VALUE_2_TOKEN + (idx - 2);
    this->dataOut->writeByte(token);
    for (int32_t i = idx - 1; i >= 0; i--) {
      this->dataOut->writeByte(buffer[i]);
    }
  }
}

void ResourceInst::writeResourceInst(StatDataOutput *dataOutArg,
                                     int32_t instId) {
  if (instId > MAX_BYTE_RESOURCE_INST_ID) {
    if (instId > MAX_SHORT_RESOURCE_INST_ID) {
      dataOutArg->writeByte(static_cast<uint8_t>(INT_RESOURCE_INST_ID_TOKEN));
      dataOutArg->writeInt(instId);
    } else {
      dataOutArg->writeByte(static_cast<uint8_t>(SHORT_RESOURCE_INST_ID_TOKEN));
      dataOutArg->writeShort(instId);
    }
  } else {
    dataOutArg->writeByte(static_cast<uint8_t>(instId));
  }
}

// Constructor and Member functions of StatArchiveWriter class
StatArchiveWriter::StatArchiveWriter(std::string outfile,
                                     HostStatSampler *samplerArg,
                                     CacheImpl *cache)
    : cache(cache) {
  resourceTypeId = 0;
  resourceInstId = 0;
  statResourcesModCount = 0;
  archiveFile = outfile;
  bytesWrittenToFile = 0;

  /* adongre
   * CID 28982: Uninitialized scalar field (UNINIT_CTOR)
   */
  m_samplesize = 0;

  dataBuffer = new StatDataOutput(archiveFile, cache);
  this->sampler = samplerArg;

  // write the time, system property etc.
  this->previousTimeStamp = steady_clock::now();

  this->dataBuffer->writeByte(HEADER_TOKEN);
  this->dataBuffer->writeByte(ARCHIVE_VERSION);
  this->dataBuffer->writeLong(
      duration_cast<milliseconds>(system_clock::now().time_since_epoch())
          .count());
  int64_t sysId = sampler->getSystemId();
  this->dataBuffer->writeLong(sysId);
  this->dataBuffer->writeLong(
      duration_cast<milliseconds>(
          sampler->getSystemStartTime().time_since_epoch())
          .count());
  int32_t tzOffset = ACE_OS::timezone();
  // offset in milli seconds
  tzOffset = tzOffset * -1 * 1000;
  this->dataBuffer->writeInt(tzOffset);

  auto now = std::chrono::system_clock::now();
  auto tm_val = apache::geode::util::chrono::localtime(now);
  char buf[512] = {0};
  std::strftime(buf, sizeof(buf), "%Z", &tm_val);
  std::string tzId(buf);
  this->dataBuffer->writeUTF(tzId);

  std::string sysDirPath = sampler->getSystemDirectoryPath();
  this->dataBuffer->writeUTF(sysDirPath);
  std::string prodDesc = sampler->getProductDescription();

  this->dataBuffer->writeUTF(prodDesc);
  ACE_utsname u;
  ACE_OS::uname(&u);
  std::string os(u.sysname);
  os += " ";
  /* This version name returns date of release of the version which
   creates confusion about the creation time of the vsd file. Hence
   removing it now. Later I'll change it to just show version without
   date. For now only name of the OS will be displayed.
   */
  // os += u.version;

  this->dataBuffer->writeUTF(os);
  std::string machineInfo(u.machine);
  machineInfo += " ";
  machineInfo += u.nodename;
  this->dataBuffer->writeUTF(machineInfo);

  resampleResources();
}

StatArchiveWriter::~StatArchiveWriter() {
  if (dataBuffer != nullptr) {
    delete dataBuffer;
    dataBuffer = nullptr;
  }
  for (const auto &p : resourceTypeMap) {
    auto rt = p.second;
    _GEODE_SAFE_DELETE(rt);
  }
}

int64_t StatArchiveWriter::bytesWritten() { return bytesWrittenToFile; }

int64_t StatArchiveWriter::getSampleSize() { return m_samplesize; }

void StatArchiveWriter::sample(const steady_clock::time_point &timeStamp) {
  std::lock_guard<decltype(sampler->getStatListMutex())> guard(
      sampler->getStatListMutex());
  m_samplesize = dataBuffer->getBytesWritten();

  sampleResources();
  this->dataBuffer->writeByte(SAMPLE_TOKEN);
  writeTimeStamp(timeStamp);
  std::map<Statistics *, ResourceInst *>::iterator p;
  for (p = resourceInstMap.begin(); p != resourceInstMap.end(); p++) {
    ResourceInst *ri = (*p).second;
    if (!!ri && (*p).first != nullptr) {
      ri->writeSample();
    }
  }
  writeResourceInst(this->dataBuffer,
                    static_cast<int32_t>(ILLEGAL_RESOURCE_INST_ID));
  m_samplesize = dataBuffer->getBytesWritten() - m_samplesize;
}

void StatArchiveWriter::sample() { sample(steady_clock::now()); }

void StatArchiveWriter::close() {
  sample();
  this->dataBuffer->flush();
  this->dataBuffer->close();
}

void StatArchiveWriter::closeFile() { this->dataBuffer->close(); }

void StatArchiveWriter::openFile(std::string filename) {
  // this->dataBuffer->openFile(filename, m_samplesize);

  StatDataOutput *p_dataBuffer = new StatDataOutput(filename, cache);

  const uint8_t *buffBegin = dataBuffer->dataBuffer->getBuffer();
  if (buffBegin == nullptr) {
    throw NullPointerException("undefined stat data buffer beginning");
  }
  const uint8_t *buffEnd = dataBuffer->dataBuffer->getCursor();
  if (buffEnd == nullptr) {
    throw NullPointerException("undefined stat data buffer end");
  }
  int32_t len = static_cast<int32_t>(buffEnd - buffBegin);

  for (int pos = 0; pos < len; pos++) {
    p_dataBuffer->writeByte(buffBegin[pos]);
  }

  delete dataBuffer;
  dataBuffer = p_dataBuffer;

  // sample();
}

void StatArchiveWriter::flush() {
  this->dataBuffer->flush();
  bytesWrittenToFile += dataBuffer->getBytesWritten();
  this->dataBuffer->resetBuffer();
  /*
    // have to figure out the problem with this code.
    delete dataBuffer;
    dataBuffer = nullptr;

    dataBuffer = new StatDataOutput(archiveFile);
   */
}

void StatArchiveWriter::sampleResources() {
  // Allocate ResourceInst for newly added stats ( Locked lists already )
  std::vector<Statistics *> &newStatsList = sampler->getNewStatistics();
  std::vector<Statistics *>::iterator newlistIter;
  for (newlistIter = newStatsList.begin(); newlistIter != newStatsList.end();
       ++newlistIter) {
    if (!resourceInstMapHas(*newlistIter)) {
      allocateResourceInst(*newlistIter);
    }
  }
  newStatsList.clear();

  // for closed stats, write token and then delete from statlist and
  // resourceInstMap.
  std::map<Statistics *, ResourceInst *>::iterator mapIter;
  std::vector<Statistics *> &statsList = sampler->getStatistics();
  std::vector<Statistics *>::iterator statlistIter = statsList.begin();
  while (statlistIter != statsList.end()) {
    if ((*statlistIter)->isClosed()) {
      mapIter = resourceInstMap.find(*statlistIter);
      if (mapIter != resourceInstMap.end()) {
        // Write delete token to file and delete from map
        ResourceInst *rinst = (*mapIter).second;
        int32_t id = rinst->getId();
        this->dataBuffer->writeByte(RESOURCE_INSTANCE_DELETE_TOKEN);
        this->dataBuffer->writeInt(id);
        resourceInstMap.erase(mapIter);
        delete rinst;
      }
      // Delete stats object stat list
      StatisticsManager::deleteStatistics(*statlistIter);
      statsList.erase(statlistIter);
      statlistIter = statsList.begin();
    } else {
      ++statlistIter;
    }
  }
}

void StatArchiveWriter::resampleResources() {
  std::lock_guard<decltype(sampler->getStatListMutex())> guard(
      sampler->getStatListMutex());
  std::vector<Statistics *> &statsList = sampler->getStatistics();
  std::vector<Statistics *>::iterator statlistIter = statsList.begin();
  while (statlistIter != statsList.end()) {
    if (!(*statlistIter)->isClosed()) {
      allocateResourceInst(*statlistIter);
    }
    ++statlistIter;
  }
}

void StatArchiveWriter::writeTimeStamp(
    const steady_clock::time_point &timeStamp) {
  auto delta = static_cast<int32_t>(
      duration_cast<milliseconds>(timeStamp - this->previousTimeStamp).count());
  if (delta > MAX_SHORT_TIMESTAMP) {
    dataBuffer->writeShort(static_cast<uint16_t>(INT_TIMESTAMP_TOKEN));
    dataBuffer->writeInt(delta);
  } else {
    dataBuffer->writeShort(static_cast<uint16_t>(delta));
  }
  this->previousTimeStamp = timeStamp;
}

bool StatArchiveWriter::resourceInstMapHas(Statistics *sp) {
  std::map<Statistics *, ResourceInst *>::iterator p;
  p = resourceInstMap.find(sp);
  if (p != resourceInstMap.end()) {
    return true;
  } else {
    return false;
  }
}

void StatArchiveWriter::allocateResourceInst(Statistics *s) {
  if (s->isClosed()) return;
  const auto type = getResourceType(s);

  ResourceInst *ri = new ResourceInst(resourceInstId, s, type, dataBuffer);
  if (ri == nullptr) {
    throw NullPointerException("could not create new resource instance");
  }
  resourceInstMap.insert(std::pair<Statistics *, ResourceInst *>(s, ri));
  this->dataBuffer->writeByte(RESOURCE_INSTANCE_CREATE_TOKEN);
  this->dataBuffer->writeInt(resourceInstId);
  this->dataBuffer->writeUTF(s->getTextId());
  this->dataBuffer->writeLong(s->getNumericId());
  this->dataBuffer->writeInt(type->getId());

  resourceInstId++;
}

const ResourceType *StatArchiveWriter::getResourceType(const Statistics *s) {
  const auto type = s->getType();
  if (type == nullptr) {
    throw NullPointerException(
        "could not know the type of the statistics object");
  }
  const ResourceType *rt = nullptr;
  const auto p = resourceTypeMap.find(type);
  if (p != resourceTypeMap.end()) {
    rt = p->second;
  } else {
    rt = new ResourceType(resourceTypeId, type);
    if (type == nullptr) {
      throw NullPointerException(
          "could not allocate memory for a new resourcetype");
    }
    resourceTypeMap.emplace(type, rt);
    // write the type to the archive
    this->dataBuffer->writeByte(RESOURCE_TYPE_TOKEN);
    this->dataBuffer->writeInt(resourceTypeId);
    this->dataBuffer->writeUTF(type->getName());
    this->dataBuffer->writeUTF(type->getDescription());
    auto stats = rt->getStats();
    auto descCnt = rt->getNumOfDescriptors();
    this->dataBuffer->writeShort(static_cast<int16_t>(descCnt));
    for (decltype(descCnt) i = 0; i < descCnt; i++) {
      std::string statsName = stats[i]->getName();
      this->dataBuffer->writeUTF(statsName);
      auto sdImpl = std::static_pointer_cast<StatisticDescriptorImpl>(stats[i]);
      if (sdImpl == nullptr) {
        throw NullPointerException(
            "could not down cast to StatisticDescriptorImpl");
      }
      this->dataBuffer->writeByte(static_cast<int8_t>(sdImpl->getTypeCode()));
      this->dataBuffer->writeBoolean(stats[i]->isCounter());
      this->dataBuffer->writeBoolean(stats[i]->isLargerBetter());
      this->dataBuffer->writeUTF(stats[i]->getUnit());
      this->dataBuffer->writeUTF(stats[i]->getDescription());
    }
    // increment resourceTypeId
    resourceTypeId++;
  }
  return rt;
}

void StatArchiveWriter::writeResourceInst(StatDataOutput *dataOut,
                                          int32_t instId) {
  if (instId > MAX_BYTE_RESOURCE_INST_ID) {
    if (instId > MAX_SHORT_RESOURCE_INST_ID) {
      dataOut->writeByte(static_cast<uint8_t>(INT_RESOURCE_INST_ID_TOKEN));
      dataOut->writeInt(instId);
    } else {
      dataOut->writeByte(static_cast<uint8_t>(SHORT_RESOURCE_INST_ID_TOKEN));
      dataOut->writeShort(instId);
    }
  } else {
    dataOut->writeByte(static_cast<uint8_t>(instId));
  }
}
}  // namespace statistics
}  // namespace geode
}  // namespace apache
