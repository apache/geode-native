#pragma once

#ifndef GEODE_STATISTICS_STATARCHIVEWRITER_H_
#define GEODE_STATISTICS_STATARCHIVEWRITER_H_

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

#include <map>
#include <list>
#include <geode/geode_globals.hpp>
#include <geode/ExceptionTypes.hpp>
#include <geode/Cache.hpp>
#include "StatsDef.hpp"
#include <geode/statistics/Statistics.hpp>
#include <geode/statistics/StatisticDescriptor.hpp>
#include "StatisticDescriptorImpl.hpp"
#include <geode/statistics/StatisticsType.hpp>
#include "HostStatSampler.hpp"
#include "util/Log.hpp"
#include <geode/DataOutput.hpp>
#include <NonCopyable.hpp>
#include <chrono>
#include "SerializationRegistry.hpp"

using namespace apache::geode::client;
/**
 * some constants to be used while archiving
 */
const int8_t ARCHIVE_VERSION = 4;
const int8_t SAMPLE_TOKEN = 0;
const int8_t RESOURCE_TYPE_TOKEN = 1;
const int8_t RESOURCE_INSTANCE_CREATE_TOKEN = 2;
const int8_t RESOURCE_INSTANCE_DELETE_TOKEN = 3;
const int8_t RESOURCE_INSTANCE_INITIALIZE_TOKEN = 4;
const int8_t HEADER_TOKEN = 77;
const int8_t ILLEGAL_RESOURCE_INST_ID = -1;
const int16_t MAX_BYTE_RESOURCE_INST_ID = 252;
const int16_t SHORT_RESOURCE_INST_ID_TOKEN = 253;
const int32_t INT_RESOURCE_INST_ID_TOKEN = 254;
const int16_t ILLEGAL_RESOURCE_INST_ID_TOKEN = -1;
const int32_t MAX_SHORT_RESOURCE_INST_ID = 65535;
const int32_t MAX_SHORT_TIMESTAMP = 65534;
const int32_t INT_TIMESTAMP_TOKEN = 65535;

/** @file
 */

namespace apache {
namespace geode {
namespace statistics {

using std::chrono::system_clock;
using std::chrono::steady_clock;

/**
 * Some of the classes which are used by the StatArchiveWriter Class
 * 1. StatDataOutput // Just a wrapper around DataOutput so that the number of
 *                 // bytes written is incremented automatically.
 * 2. ResourceType // The ResourceType and the ResourceInst class is used
 * 3. ResourceInst // by the StatArchiveWriter class for keeping track of
 *                 // of the Statistics object and the value of the
 *                 // descriptors in the previous sample run.
 */

class CPPCACHE_EXPORT StatDataOutput {
 public:
  StatDataOutput(CacheImpl* cache);
  StatDataOutput(std::string, CacheImpl* cache);
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
  void writeString(std::string v);
  /**
   * Writes wstring value in the buffer.
   */
  void writeUTF(std::wstring v);
  /**
   * This method is for the unit tests only for this class.
   */
  const uint8_t *getBuffer() { return dataBuffer->getBuffer(); }
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

class CPPCACHE_EXPORT ResourceType : private NonCopyable,
                                     private NonAssignable {
 public:
  ResourceType(int32_t id, StatisticsType *type);
  int32_t getId();
  StatisticDescriptor **getStats();
  int32_t getNumOfDescriptors();

 private:
  int32_t id;
  StatisticDescriptor **stats;
  int32_t numOfDescriptors;
};

/* adongre
 * CID 28735: Other violation (MISSING_COPY)
 * Class "apache::geode::statistics::ResourceInst" owns resources
 * that are managed in its constructor and destructor but has no user-written
 * copy constructor.
 * CID 28721: Other violation (MISSING_ASSIGN)
 * Class "apache::geode::statistics::ResourceInst" owns resources that are
 * managed
 * in its constructor and destructor but has no user-written assignment
 * operator.
 *
 * FIX : Make the class NonCopyable
 */

class CPPCACHE_EXPORT ResourceInst : private NonCopyable,
                                     private NonAssignable {
 public:
  ResourceInst(int32_t id, Statistics *, ResourceType *, StatDataOutput *);
  ~ResourceInst();
  int32_t getId();
  Statistics *getResource();
  ResourceType *getType();
  int64_t getStatValue(StatisticDescriptor *f);
  void writeSample();
  void writeStatValue(StatisticDescriptor *s, int64_t v);
  void writeCompactValue(int64_t v);
  void writeResourceInst(StatDataOutput *, int32_t);

 private:
  int32_t id;
  Statistics *resource;
  ResourceType *type;
  /* This will contain the previous values of the descriptors */
  int64_t *archivedStatValues;
  StatDataOutput *dataOut;
  /* Number of descriptors this resource instnace has */
  int32_t numOfDescps;
  /* To know whether the instance has come for the first time */
  bool firstTime;
};

class HostStatSampler;
/**
 * @class StatArchiveWriter
 */

class CPPCACHE_EXPORT StatArchiveWriter {
 private:
  HostStatSampler *sampler;
  StatDataOutput *dataBuffer;
  CacheImpl* cache;
  steady_clock::time_point previousTimeStamp;
  int32_t resourceTypeId;
  int32_t resourceInstId;
  int32_t statResourcesModCount;
  int64_t bytesWrittenToFile;
  int64_t m_samplesize;
  std::string archiveFile;
  std::map<Statistics *, ResourceInst *> resourceInstMap;
  std::map<StatisticsType *, ResourceType *> resourceTypeMap;

  /* private member functions */
  void allocateResourceInst(Statistics *r);
  void sampleResources();
  void resampleResources();
  void writeResourceInst(StatDataOutput *, int32_t);
  void writeTimeStamp(const steady_clock::time_point &timeStamp);
  void writeStatValue(StatisticDescriptor *f, int64_t v, DataOutput dataOut);
  ResourceType *getResourceType(Statistics *);
  bool resourceInstMapHas(Statistics *sp);

 public:
  StatArchiveWriter(std::string archiveName, HostStatSampler *sampler,
                    CacheImpl* cache);
  ~StatArchiveWriter();
  /**
   * Returns the number of bytes written so far to this archive.
   * This does not take compression into account.
   */
  int64_t bytesWritten();
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
   * @param std::string filename.
   */
  void openFile(std::string);

  /**
   * Returns the size of number of bytes written so far to this archive.
   */
  int64_t getSampleSize();

  /**
   * Flushes the contents of the dataBuffer to the archiveFile
   */
  void flush();
};
}  // namespace statistics
}  // namespace geode
}  // namespace apache

#endif  // GEODE_STATISTICS_STATARCHIVEWRITER_H_
