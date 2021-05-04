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

#include <boost/asio/ip/host_name.hpp>
#include <boost/date_time.hpp>

#include <geode/internal/geode_globals.hpp>

#include "../CacheImpl.hpp"
#include "../util/chrono/time_point.hpp"
#include "GeodeStatisticsFactory.hpp"
#include "HostStatSampler.hpp"
#include "config.h"

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
      LOG_ERROR("Could not write into the statistics file");
      throw GeodeIOException("Could not write into the statistics file");
    }
  }
  int rVal = fflush(m_fp);
  if (rVal != 0) {
    LOG_ERROR("Could not flush into the statistics file");
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

const uint8_t *StatDataOutput::getBuffer() { return dataBuffer->getBuffer(); }

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
    : cache_(cache) {
  resourceTypeId_ = 0;
  resourceInstId_ = 0;
  archiveFile_ = outfile;
  bytesWrittenToFile_ = 0;

  sampleSize_ = 0;

  dataBuffer_ = new StatDataOutput(archiveFile_, cache);
  this->sampler_ = samplerArg;

  // write the time, system property etc.
  this->previousTimeStamp_ = steady_clock::now();

  this->dataBuffer_->writeByte(HEADER_TOKEN);
  this->dataBuffer_->writeByte(ARCHIVE_VERSION);
  this->dataBuffer_->writeLong(
      duration_cast<milliseconds>(system_clock::now().time_since_epoch())
          .count());
  int64_t sysId = sampler_->getSystemId();
  this->dataBuffer_->writeLong(sysId);
  this->dataBuffer_->writeLong(
      duration_cast<milliseconds>(
          sampler_->getSystemStartTime().time_since_epoch())
          .count());

  // C++20: Use std::chrono::time_zone
  boost::posix_time::time_duration timeZoneOffset(
      boost::posix_time::second_clock::local_time() -
      boost::posix_time::second_clock::universal_time());
  this->dataBuffer_->writeInt(
      static_cast<int32_t>(timeZoneOffset.total_milliseconds()));

  // C++20: Use std::chrono::time_zone
  auto now = std::chrono::system_clock::now();
  auto localTime = apache::geode::util::chrono::localtime(now);
  std::ostringstream timeZoneId;
  timeZoneId << std::put_time(&localTime, "%Z");
  this->dataBuffer_->writeUTF(timeZoneId.str());

  this->dataBuffer_->writeUTF(sampler_->getSystemDirectoryPath());
  this->dataBuffer_->writeUTF(sampler_->getProductDescription());
  this->dataBuffer_->writeUTF(GEODE_SYSTEM_NAME);
  this->dataBuffer_->writeUTF(std::string(GEODE_SYSTEM_PROCESSOR) + " " +
                              boost::asio::ip::host_name());

  resampleResources();
}

StatArchiveWriter::~StatArchiveWriter() {
  if (dataBuffer_ != nullptr) {
    delete dataBuffer_;
    dataBuffer_ = nullptr;
  }
  for (const auto &p : resourceTypeMap_) {
    auto rt = p.second;
    _GEODE_SAFE_DELETE(rt);
  }
}

size_t StatArchiveWriter::bytesWritten() { return bytesWrittenToFile_; }

size_t StatArchiveWriter::getSampleSize() { return sampleSize_; }

void StatArchiveWriter::sample(const steady_clock::time_point &timeStamp) {
  std::lock_guard<decltype(sampler_->getStatListMutex())> guard(
      sampler_->getStatListMutex());
  sampleSize_ = dataBuffer_->getBytesWritten();

  sampleResources();
  this->dataBuffer_->writeByte(SAMPLE_TOKEN);
  writeTimeStamp(timeStamp);
  for (auto p = resourceInstMap_.begin(); p != resourceInstMap_.end(); p++) {
    auto ri = (*p).second;
    if (!!ri && (*p).first != nullptr) {
      ri->writeSample();
    }
  }
  writeResourceInst(this->dataBuffer_,
                    static_cast<int32_t>(ILLEGAL_RESOURCE_INST_ID));
  sampleSize_ = dataBuffer_->getBytesWritten() - sampleSize_;
}

void StatArchiveWriter::sample() { sample(steady_clock::now()); }

void StatArchiveWriter::close() {
  sample();
  this->dataBuffer_->flush();
  this->dataBuffer_->close();
}

void StatArchiveWriter::closeFile() { this->dataBuffer_->close(); }

void StatArchiveWriter::openFile(std::string filename) {
  StatDataOutput *p_dataBuffer = new StatDataOutput(filename, cache_);

  const uint8_t *buffBegin = dataBuffer_->dataBuffer->getBuffer();
  if (buffBegin == nullptr) {
    throw NullPointerException("undefined stat data buffer beginning");
  }
  const uint8_t *buffEnd = dataBuffer_->dataBuffer->getCursor();
  if (buffEnd == nullptr) {
    throw NullPointerException("undefined stat data buffer end");
  }
  int32_t len = static_cast<int32_t>(buffEnd - buffBegin);

  for (int pos = 0; pos < len; pos++) {
    p_dataBuffer->writeByte(buffBegin[pos]);
  }

  delete dataBuffer_;
  dataBuffer_ = p_dataBuffer;
}

void StatArchiveWriter::flush() {
  this->dataBuffer_->flush();
  bytesWrittenToFile_ += dataBuffer_->getBytesWritten();
  this->dataBuffer_->resetBuffer();
  /*
    // have to figure out the problem with this code.
    delete dataBuffer;
    dataBuffer = nullptr;

    dataBuffer = new StatDataOutput(archiveFile);
   */
}

void StatArchiveWriter::sampleResources() {
  // Allocate ResourceInst for newly added stats ( Locked lists already )
  std::vector<Statistics *> &newStatsList = sampler_->getNewStatistics();
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
  std::map<Statistics *, std::shared_ptr<ResourceInst>>::iterator mapIter;
  std::vector<Statistics *> &statsList = sampler_->getStatistics();
  std::vector<Statistics *>::iterator statlistIter = statsList.begin();
  while (statlistIter != statsList.end()) {
    if ((*statlistIter)->isClosed()) {
      mapIter = resourceInstMap_.find(*statlistIter);
      if (mapIter != resourceInstMap_.end()) {
        // Write delete token to file and delete from map
        auto rinst = (*mapIter).second;
        int32_t id = rinst->getId();
        this->dataBuffer_->writeByte(RESOURCE_INSTANCE_DELETE_TOKEN);
        this->dataBuffer_->writeInt(id);
        resourceInstMap_.erase(mapIter);
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
  std::lock_guard<decltype(sampler_->getStatListMutex())> guard(
      sampler_->getStatListMutex());
  std::vector<Statistics *> &statsList = sampler_->getStatistics();
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
      duration_cast<milliseconds>(timeStamp - this->previousTimeStamp_)
          .count());
  if (delta > MAX_SHORT_TIMESTAMP) {
    dataBuffer_->writeShort(static_cast<uint16_t>(INT_TIMESTAMP_TOKEN));
    dataBuffer_->writeInt(delta);
  } else {
    dataBuffer_->writeShort(static_cast<uint16_t>(delta));
  }
  this->previousTimeStamp_ = timeStamp;
}

bool StatArchiveWriter::resourceInstMapHas(Statistics *sp) {
  auto p = resourceInstMap_.find(sp);
  if (p != resourceInstMap_.end()) {
    return true;
  } else {
    return false;
  }
}

void StatArchiveWriter::allocateResourceInst(Statistics *s) {
  if (s->isClosed()) return;
  const auto type = getResourceType(s);

  auto ri = std::shared_ptr<ResourceInst>(
      new ResourceInst(resourceInstId_, s, type, dataBuffer_));
  resourceInstMap_.insert(
      std::pair<Statistics *, std::shared_ptr<ResourceInst>>(s, ri));
  this->dataBuffer_->writeByte(RESOURCE_INSTANCE_CREATE_TOKEN);
  this->dataBuffer_->writeInt(resourceInstId_);
  this->dataBuffer_->writeUTF(s->getTextId());
  this->dataBuffer_->writeLong(s->getNumericId());
  this->dataBuffer_->writeInt(type->getId());

  resourceInstId_++;
}

const ResourceType *StatArchiveWriter::getResourceType(const Statistics *s) {
  const auto type = s->getType();
  if (type == nullptr) {
    throw NullPointerException(
        "could not know the type of the statistics object");
  }
  const ResourceType *rt = nullptr;
  const auto p = resourceTypeMap_.find(type);
  if (p != resourceTypeMap_.end()) {
    rt = p->second;
  } else {
    rt = new ResourceType(resourceTypeId_, type);
    if (type == nullptr) {
      throw NullPointerException(
          "could not allocate memory for a new resourcetype");
    }
    resourceTypeMap_.emplace(type, rt);
    // write the type to the archive
    this->dataBuffer_->writeByte(RESOURCE_TYPE_TOKEN);
    this->dataBuffer_->writeInt(resourceTypeId_);
    this->dataBuffer_->writeUTF(type->getName());
    this->dataBuffer_->writeUTF(type->getDescription());
    auto stats = rt->getStats();
    auto descCnt = rt->getNumOfDescriptors();
    this->dataBuffer_->writeShort(static_cast<int16_t>(descCnt));
    for (decltype(descCnt) i = 0; i < descCnt; i++) {
      std::string statsName = stats[i]->getName();
      this->dataBuffer_->writeUTF(statsName);
      auto sdImpl = std::static_pointer_cast<StatisticDescriptorImpl>(stats[i]);
      if (sdImpl == nullptr) {
        throw NullPointerException(
            "could not down cast to StatisticDescriptorImpl");
      }
      this->dataBuffer_->writeByte(static_cast<int8_t>(sdImpl->getTypeCode()));
      this->dataBuffer_->writeBoolean(stats[i]->isCounter());
      this->dataBuffer_->writeBoolean(stats[i]->isLargerBetter());
      this->dataBuffer_->writeUTF(stats[i]->getUnit());
      this->dataBuffer_->writeUTF(stats[i]->getDescription());
    }
    // increment resourceTypeId
    resourceTypeId_++;
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
