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

#ifndef GEODE_STATISTICS_STATARCHIVEWRITER_H_
#define GEODE_STATISTICS_STATARCHIVEWRITER_H_

#include <chrono>
#include <list>
#include <map>
#include <vector>

#include <geode/Cache.hpp>
#include <geode/DataOutput.hpp>
#include <geode/ExceptionTypes.hpp>
#include <geode/internal/geode_globals.hpp>

#include "../SerializationRegistry.hpp"
#include "../util/Log.hpp"
#include "HostStatSampler.hpp"
#include "StatisticDescriptor.hpp"
#include "StatisticDescriptorImpl.hpp"
#include "Statistics.hpp"
#include "StatisticsType.hpp"
#include "StatsDef.hpp"

constexpr int8_t ARCHIVE_VERSION = 4;
constexpr int8_t SAMPLE_TOKEN = 0;
constexpr int8_t RESOURCE_TYPE_TOKEN = 1;
constexpr int8_t RESOURCE_INSTANCE_CREATE_TOKEN = 2;
constexpr int8_t RESOURCE_INSTANCE_DELETE_TOKEN = 3;
constexpr int8_t RESOURCE_INSTANCE_INITIALIZE_TOKEN = 4;
constexpr int8_t HEADER_TOKEN = 77;
constexpr int8_t ILLEGAL_RESOURCE_INST_ID = -1;
constexpr int16_t MAX_BYTE_RESOURCE_INST_ID = 252;
constexpr int16_t SHORT_RESOURCE_INST_ID_TOKEN = 253;
constexpr int32_t INT_RESOURCE_INST_ID_TOKEN = 254;
constexpr int16_t ILLEGAL_RESOURCE_INST_ID_TOKEN = -1;
constexpr int32_t MAX_SHORT_RESOURCE_INST_ID = 65535;
constexpr int32_t MAX_SHORT_TIMESTAMP = 65534;
constexpr int32_t INT_TIMESTAMP_TOKEN = 65535;

namespace apache {
namespace geode {
namespace statistics {

class HostStatSampler;

using apache::geode::client::CacheImpl;
using apache::geode::client::DataOutput;
using std::chrono::steady_clock;
using std::chrono::system_clock;

/**
 * Some of the classes which are used by the StatArchiveWriter Class
 * 1. StatDataOutput // Just a wrapper around DataOutput so that the number of
 *                 // bytes written is incremented automatically.
 * 2. ResourceType // The ResourceType and the ResourceInst class is used
 * 3. ResourceInst // by the StatArchiveWriter class for keeping track of
 *                 // of the Statistics object and the value of the
 *                 // descriptors in the previous sample run.
 */
class StatDataOutput {
 public:
  explicit StatDataOutput(CacheImpl *cache);
  StatDataOutput(std::string, CacheImpl *cache);
  ~StatDataOutput();
  /**
   * Returns the number of bytes written into the buffer so far.
   * This does not takes the compression into account.
   */
  int64_t getBytesWritten();
  /**
   * Writes the buffer into the outfile.
   */
  void flush();
  /**
   * Writes the buffer into the outfile and sets bytesWritten to zero.
   */
  void resetBuffer();
  /**
   * Writes 8 bit integer value in the buffer.
   */
  void writeByte(int8_t v);
  /**
   * Writes boolean value in the buffer.
   * Nothing but an int8.
   */
  void writeBoolean(int8_t v);
  /**
   * Writes 16 bit integer value in the buffer.
   */
  void writeShort(int16_t v);
  /**
   * Writes Long value 32 bit in the buffer.
   */
  void writeInt(int32_t v);
  /**
   * Writes Double value 64 bit in the buffer.
   */
  void writeLong(int64_t v);
  /**
   * Writes string value in the buffer.
   */
  void writeUTF(std::string v);
  /**
   * This method is for the unit tests only for this class.
   */
  const uint8_t *getBuffer();
  void close();

  void openFile(std::string, int64_t);

 private:
  int64_t bytesWritten;
  std::unique_ptr<DataOutput> dataBuffer;
  std::string outFile;
  FILE *m_fp;
  bool closed;
  friend class StatArchiveWriter;
};

class ResourceType {
 public:
  ResourceType(int32_t id, const StatisticsType *type);
  ResourceType(const ResourceType &) = delete;
  ResourceType &operator=(const ResourceType &) = delete;
  int32_t getId() const;
  const std::vector<std::shared_ptr<StatisticDescriptor>> &getStats() const;
  size_t getNumOfDescriptors() const;

 private:
  int32_t id;
  const StatisticsType *type;
};

class ResourceInst {
 public:
  ResourceInst(int32_t id, Statistics *, const ResourceType *,
               StatDataOutput *);
  ResourceInst(const ResourceInst &) = delete;
  ResourceInst &operator=(const ResourceInst &) = delete;
  ~ResourceInst();
  int32_t getId();
  Statistics *getResource();
  const ResourceType *getType() const;
  int64_t getStatValue(std::shared_ptr<StatisticDescriptor> f);
  void writeSample();
  void writeStatValue(std::shared_ptr<StatisticDescriptor> s, int64_t v);
  void writeCompactValue(int64_t v);
  void writeResourceInst(StatDataOutput *, int32_t);

 private:
  int32_t id;
  Statistics *resource;
  const ResourceType *type;
  /* This will contain the previous values of the descriptors */
  int64_t *archivedStatValues;
  StatDataOutput *dataOut;
  /* To know whether the instance has come for the first time */
  bool firstTime = true;
};

class HostStatSampler;

class StatArchiveWriter {
  HostStatSampler *sampler_;
  StatDataOutput *dataBuffer_;
  CacheImpl *cache_;
  steady_clock::time_point previousTimeStamp_;
  int32_t resourceTypeId_;
  int32_t resourceInstId_;
  size_t bytesWrittenToFile_;
  size_t sampleSize_;
  std::string archiveFile_;
  std::map<Statistics *, std::shared_ptr<ResourceInst>> resourceInstMap_;
  std::map<const StatisticsType *, const ResourceType *> resourceTypeMap_;

  void allocateResourceInst(Statistics *r);
  void sampleResources();
  void resampleResources();
  void writeResourceInst(StatDataOutput *, int32_t);
  void writeTimeStamp(const steady_clock::time_point &timeStamp);
  void writeStatValue(std::shared_ptr<StatisticDescriptor> f, int64_t v,
                      DataOutput dataOut);
  const ResourceType *getResourceType(const Statistics *);
  bool resourceInstMapHas(Statistics *sp);

 public:
  StatArchiveWriter(std::string archiveName, HostStatSampler *sampler,
                    CacheImpl *cache);
  ~StatArchiveWriter();
  /**
   * Returns the number of bytes written so far to this archive.
   * This does not take compression into account.
   */
  size_t bytesWritten();
  /**
   * Archives a sample snapshot at the given timeStamp.
   * @param timeStamp a value obtained using NanoTimer::now.
   */
  void sample(const steady_clock::time_point &timeStamp);
  /**
   * Archives a sample snapshot at the current time.
   */
  void sample();
  /**
   * Closes the statArchiver by flushing its data to disk and closing its
   * output stream.
   */
  void close();

  /**
   * Closes the statArchiver by closing its output stream.
   */
  void closeFile();

  /**
   * Opens the statArchiver by opening the file provided as a parameter.
   * @param filename string representation of the file name
   */
  void openFile(std::string filename);

  /**
   * Returns the size of number of bytes written so far to this archive.
   */
  size_t getSampleSize();

  /**
   * Flushes the contents of the dataBuffer to the archiveFile
   */
  void flush();
};
}  // namespace statistics
}  // namespace geode
}  // namespace apache

#endif  // GEODE_STATISTICS_STATARCHIVEWRITER_H_
