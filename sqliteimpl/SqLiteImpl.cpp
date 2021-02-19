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

#include "SqLiteImpl.hpp"

#include <geode/Cache.hpp>
#include <geode/Region.hpp>

#include "sqliteimpl_export.h"

#ifdef _WIN32
#include <Windows.h>
#endif

namespace {
std::string g_default_persistence_directory = "GeodeRegionData";
}  // namespace

namespace apache {
namespace geode {
namespace client {

static constexpr char const* MAX_PAGE_COUNT = "MaxPageCount";
static constexpr char const* PAGE_SIZE = "PageSize";
static constexpr char const* PERSISTENCE_DIR = "PersistenceDirectory";

void SqLiteImpl::init(const std::shared_ptr<Region>& region,
                      const std::shared_ptr<Properties>& diskProperties) {
  // Set the default values

  int maxPageCount = 0;
  int pageSize = 0;
  m_regionPtr = region;
  m_persistanceDir = g_default_persistence_directory;
  std::string regionName = region->getName();
  if (diskProperties != nullptr) {
    auto maxPageCountPtr = diskProperties->find(MAX_PAGE_COUNT);
    auto pageSizePtr = diskProperties->find(PAGE_SIZE);
    auto persDir = diskProperties->find(PERSISTENCE_DIR);

    if (maxPageCountPtr != nullptr) {
      maxPageCount = atoi(maxPageCountPtr->value().c_str());
    }

    if (pageSizePtr != nullptr) pageSize = atoi(pageSizePtr->value().c_str());

    if (persDir != nullptr) m_persistanceDir = persDir->value().c_str();
  }

#ifndef _WIN32
  char currWDPath[512];
  ::getcwd(currWDPath, 512);

  if (m_persistanceDir.at(0) != '/') {
    if (0 == ::strlen(currWDPath)) {
      throw InitFailedException(
          "Failed to get absolute path for persistence directory.");
    }
    m_persistanceDir = std::string(currWDPath) + "/" + m_persistanceDir;
  }

  // Create persistence directory
  ::mkdir(m_persistanceDir.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

  // Create region directory
  std::string regionDirectory = m_persistanceDir + "/" + regionName;
  ::mkdir(regionDirectory.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
  m_regionDBFile = regionDirectory + "/" + regionName + ".db";

#else
  char currWDPath[512];
  GetCurrentDirectory(512, currWDPath);

  if (m_persistanceDir.find(":", 0) == std::string::npos) {
    if (currWDPath == NULL)
      throw InitFailedException(
          "Failed to get absolute path for persistence directory.");
    m_persistanceDir = std::string(currWDPath) + "/" + m_persistanceDir;
  }

  // Create persistence directory
  CreateDirectory(m_persistanceDir.c_str(), NULL);

  // Create region directory
  std::string regionDirectory = m_persistanceDir + "/" + regionName;
  CreateDirectory(regionDirectory.c_str(), NULL);
  m_regionDBFile = regionDirectory + "/" + regionName + ".db";

#endif

  if (m_sqliteHelper->initDB(region->getName().c_str(), maxPageCount, pageSize,
                             m_regionDBFile.c_str()) != 0) {
    throw IllegalStateException("Failed to initialize database in SQLITE.");
  }
}

void SqLiteImpl::write(const std::shared_ptr<CacheableKey>& key,
                       const std::shared_ptr<Cacheable>& value,
                       std::shared_ptr<void>&) {
  // Serialize key and value.
  auto& cache = m_regionPtr->getCache();
  auto keyDataBuffer = cache.createDataOutput();
  auto valueDataBuffer = cache.createDataOutput();
  size_t keyBufferSize, valueBufferSize;

  keyDataBuffer.writeObject(key);
  valueDataBuffer.writeObject(value);
  void* keyData = const_cast<uint8_t*>(keyDataBuffer.getBuffer(&keyBufferSize));
  void* valueData =
      const_cast<uint8_t*>(valueDataBuffer.getBuffer(&valueBufferSize));

  if (m_sqliteHelper->insertKeyValue(keyData, static_cast<int>(keyBufferSize),
                                     valueData,
                                     static_cast<int>(valueBufferSize)) != 0) {
    throw IllegalStateException("Failed to write key value in SQLITE.");
  }
}

bool SqLiteImpl::writeAll() { return true; }
std::shared_ptr<Cacheable> SqLiteImpl::read(
    const std::shared_ptr<CacheableKey>& key, const std::shared_ptr<void>&) {
  // Serialize key.
  auto keyDataBuffer = m_regionPtr->getCache().createDataOutput();
  size_t keyBufferSize;
  keyDataBuffer.writeObject(key);
  void* keyData = const_cast<uint8_t*>(keyDataBuffer.getBuffer(&keyBufferSize));
  void* valueData;
  int valueBufferSize;

  if (m_sqliteHelper->getValue(keyData, static_cast<int>(keyBufferSize),
                               valueData, valueBufferSize) != 0) {
    throw IllegalStateException("Failed to read the value from SQLITE.");
  }

  // Deserialize object and return value.
  auto valueDataBuffer = m_regionPtr->getCache().createDataInput(
      reinterpret_cast<uint8_t*>(valueData), valueBufferSize);
  std::shared_ptr<Cacheable> retValue;
  valueDataBuffer.readObject(retValue);

  // Free memory for serialized form of Cacheable object.
  free(valueData);
  return retValue;
}

bool SqLiteImpl::readAll() { return true; }

void SqLiteImpl::destroyRegion() {
  if (m_sqliteHelper->closeDB() != 0) {
    throw IllegalStateException("Failed to destroy region from SQLITE.");
  }

#ifndef _WIN32
  ::unlink(m_regionDBFile.c_str());
  ::rmdir(m_regionDir.c_str());
  ::rmdir(m_persistanceDir.c_str());
#else
  DeleteFile(m_regionDBFile.c_str());
  RemoveDirectory(m_regionDir.c_str());
  RemoveDirectory(m_persistanceDir.c_str());
#endif
}

void SqLiteImpl::destroy(const std::shared_ptr<CacheableKey>& key,
                         const std::shared_ptr<void>&) {
  // Serialize key and value.
  auto keyDataBuffer = m_regionPtr->getCache().createDataOutput();
  size_t keyBufferSize;
  keyDataBuffer.writeObject(key);
  void* keyData = const_cast<uint8_t*>(keyDataBuffer.getBuffer(&keyBufferSize));
  if (m_sqliteHelper->removeKey(keyData, static_cast<int>(keyBufferSize)) !=
      0) {
    throw IllegalStateException("Failed to destroy the key from SQLITE.");
  }
}

SqLiteImpl::SqLiteImpl() {
  m_sqliteHelper = std::unique_ptr<SqLiteHelper>(new SqLiteHelper());
}

void SqLiteImpl::close() {
  m_sqliteHelper->closeDB();

#ifndef _WIN32
  ::unlink(m_regionDBFile.c_str());
  ::rmdir(m_regionDir.c_str());
  ::rmdir(m_persistanceDir.c_str());
#else
  DeleteFile(m_regionDBFile.c_str());
  RemoveDirectory(m_regionDir.c_str());
  RemoveDirectory(m_persistanceDir.c_str());
#endif
}

}  // namespace client
}  // namespace geode
}  // namespace apache

extern "C" {

using apache::geode::client::PersistenceManager;
using apache::geode::client::SqLiteImpl;

SQLITEIMPL_EXPORT PersistenceManager* createSqLiteInstance() {
  return new SqLiteImpl();
}
}
