#pragma once

#ifndef GEODE_SQLITEIMPL_SQLITEIMPL_H_
#define GEODE_SQLITEIMPL_SQLITEIMPL_H_

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

#include "SqLiteHelper.hpp"

/**
 * @file
 */

namespace apache {
namespace geode {
namespace client {

/**
 * @class SqLite(3.7) Implementation SqLiteImpl.hpp
 * SqLite API for overflow.
 * The SqLiteImpl class derives from PersistenceManager base class and
 * implements a persistent store with SqLite DB.
 *
 */

class SqLiteImpl : public PersistenceManager {
  /**
   * @brief public methods
   */
 public:
  /**
   * Initializes the DB for the region. SqLite settings are passed via
   * diskProperties argument.
   * @throws InitfailedException if persistence directory/environment directory
   * initialization fails.
   */
  void init(const std::shared_ptr<Region>& regionptr,
            const std::shared_ptr<Properties>& diskProperties) override;

  /**
   * Stores a key-value pair in the SqLite implementation.
   * @param key the key to write.
   * @param value the value to write
   * @throws DiskFailureException if the write fails due to disk failure.
   */
  void write(const std::shared_ptr<CacheableKey>& key,
             const std::shared_ptr<Cacheable>& value,
             std::shared_ptr<void>& dbHandle) override;

  /**
   * Writes the entire region into the SqLite implementation.
   * @throws DiskFailureException if the write fails due to disk fail.
   */
  bool writeAll() override;

  /**
   * Reads the value for the key from SqLite.
   * @returns value of type std::shared_ptr<Cacheable>.
   * @param key is the key for which the value has to be read.
   * @throws IllegalArgumentException if the key is NULL.
   * @throws DiskCorruptException if the data to be read is corrupt.
   */
  std::shared_ptr<Cacheable> read(
      const std::shared_ptr<CacheableKey>& key,
      const std::shared_ptr<void>& dbHandle) override;

  /**
   * Read all the keys and values for a region stored in SqLite.
   */
  bool readAll() override;

  /**
   * Invalidates an entry stored in SqLite.
   * @throws IllegalArgumentException if the key is NULL.
   * @throws RegionDestroyedException is the region is already destroyed.
   * @throws EntryNotFoundException if the entry is not found on the disk.
   */
  // void invalidate(const std::shared_ptr<CacheableKey>& key);

  /**
   * Destroys an entry stored in SqLite. .
   * @throws IllegalArgumentException if the key is NULL.
   * @throws RegionDestroyedException is the region is already destroyed.
   * @throws EntryNotFoundException if the entry is not found on the disk.
   */
  void destroy(const std::shared_ptr<CacheableKey>& key,
               const std::shared_ptr<void>& dbHandle) override;

  /**
   * Returns number of entries stored in SqLite for the region.
   */
  //  are we removing this method from PersistenceManager?
  // int numEntries() const;

  /**
   * Destroys the region in the SqLite implementation.
   * @throws RegionDestroyedException is the region is already destroyed.
   */
  void destroyRegion();

  /**
   * Closes the SqLite persistence manager implementation.
   * @throws ShutdownFailedException if clean-up of region and environment files
   * fails..
   */
  void close() override;

  /**
   * @brief destructor
   */
  ~SqLiteImpl() override = default;

  /**
   * @brief constructor
   */
  SqLiteImpl();

  /**
   * @brief private members
   */

 private:
  std::unique_ptr<SqLiteHelper> m_sqliteHelper;

  std::string m_regionDBFile;
  std::string m_regionDir;
  std::string m_persistanceDir;
};
}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_SQLITEIMPL_SQLITEIMPL_H_
