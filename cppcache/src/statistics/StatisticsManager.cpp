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

#include "StatisticsManager.hpp"

#include <string>

#include <geode/Exception.hpp>
#include <geode/internal/geode_globals.hpp>
#include <geode/util/LogLevel.hpp>

#include "../AdminRegion.hpp"
#include "../util/Log.hpp"
#include "AtomicStatisticsImpl.hpp"
#include "GeodeStatisticsFactory.hpp"
#include "HostStatSampler.hpp"
#include "OsStatisticsImpl.hpp"

namespace apache {
namespace geode {
namespace statistics {

using client::Exception;
using client::LogLevel;

StatisticsManager::StatisticsManager(
    const char* filePath, const std::chrono::milliseconds sampleInterval,
    bool enabled, CacheImpl* cache, int64_t statFileLimit,
    int64_t statDiskSpaceLimit)
    : m_sampleIntervalMs(sampleInterval),
      m_sampler(nullptr),
      m_adminRegion(nullptr) {
  m_newlyAddedStatsList.reserve(16);  // Allocate initial sizes
  m_statisticsFactory =
      std::unique_ptr<GeodeStatisticsFactory>(new GeodeStatisticsFactory(this));

  try {
    if (enabled) {
      m_sampler = std::unique_ptr<HostStatSampler>(
          new HostStatSampler(filePath, m_sampleIntervalMs, this, cache,
                              statFileLimit, statDiskSpaceLimit));
      m_sampler->start();
    }
  } catch (...) {
    m_sampler = nullptr;
    throw;
  }
}

void StatisticsManager::forceSample() {
  if (m_sampler) m_sampler->forceSample();
}

StatisticsManager::~StatisticsManager() {
  try {
    // Stop the sampler
    closeSampler();

    // List should be empty if close() is called on each Stats object
    // If this is not done, delete all the pointers
    std::lock_guard<decltype(m_statsListLock)> guard(m_statsListLock);
    int32_t count = static_cast<int32_t>(m_statsList.size());
    if (count > 0) {
      LOG_FINEST("~StatisticsManager has found %d leftover statistics:", count);
      std::vector<Statistics*>::iterator iterFind = m_statsList.begin();
      while (iterFind != m_statsList.end()) {
        if (*iterFind != nullptr) {
          std::string temp((*iterFind)->getType()->getName());
          LOG_FINEST("Leftover statistic: %s", temp.c_str());
          /* adongre
           * Passing null variable "*iterFind" to function
           * "apache::geode::statistics::StatisticsManager::deleteStatistics(apache::geode::statistics::Statistics
           * *&)",
           * which dereferences it.
           * FIX : Put the call into the if condition
           */
          deleteStatistics(*iterFind);
          *iterFind = nullptr;
        }
        ++iterFind;
      }
      m_statsList.erase(m_statsList.begin(), m_statsList.end());
    }
  } catch (const Exception& ex) {
    LOG_WARN("~StatisticsManager swallowing Geode exception %s",
             ex.getName().c_str());
  } catch (const std::exception& ex) {
    std::string what = "~StatisticsManager swallowing std::exception: ";
    what += ex.what();
    LOG_WARN(what.c_str());
  } catch (...) {
    LOG_ERROR("~StatisticsManager swallowing unknown exception");
  }
}

std::recursive_mutex& StatisticsManager::getListMutex() {
  return m_statsListLock;
}

void StatisticsManager::closeSampler() {
  if (m_sampler) {
    m_sampler->stop();
    m_sampler = nullptr;
  }
}

void StatisticsManager::addStatisticsToList(Statistics* stat) {
  if (stat) {
    std::lock_guard<decltype(m_statsListLock)> guard(m_statsListLock);
    m_statsList.push_back(stat);

    /* Add to m_newlyAddedStatsList also so that a fresh traversal not needed
    before sampling.
    After writing token to sampled file, stats ptrs will be deleted from list.
    */
    m_newlyAddedStatsList.push_back(stat);
  }
}

int32_t StatisticsManager::getStatListModCount() {
  std::lock_guard<decltype(m_statsListLock)> guard(m_statsListLock);
  return static_cast<int32_t>(m_statsList.size());
}

std::vector<Statistics*>& StatisticsManager::getStatsList() {
  return this->m_statsList;
}

std::vector<Statistics*>& StatisticsManager::getNewlyAddedStatsList() {
  return this->m_newlyAddedStatsList;
}

Statistics* StatisticsManager::findFirstStatisticsByType(
    const StatisticsType* type) {
  std::lock_guard<decltype(m_statsListLock)> guard(m_statsListLock);
  std::vector<Statistics*>::iterator start = m_statsList.begin();
  while (start != m_statsList.end()) {
    if (!((*start)->isClosed()) && ((*start)->getType() == type)) {
      return *start;
    }
    start++;
  }
  return nullptr;
}

std::vector<Statistics*> StatisticsManager::findStatisticsByType(
    StatisticsType* type) {
  std::vector<Statistics*> hits;

  std::lock_guard<decltype(m_statsListLock)> guard(m_statsListLock);

  std::vector<Statistics*>::iterator start = m_statsList.begin();
  while (start != m_statsList.end()) {
    if (!((*start)->isClosed()) && ((*start)->getType() == type)) {
      hits.push_back(*start);
    }
    start++;
  }
  return hits;
}

std::vector<Statistics*> StatisticsManager::findStatisticsByTextId(
    char* textId) {
  std::vector<Statistics*> hits;

  std::lock_guard<decltype(m_statsListLock)> guard(m_statsListLock);

  std::vector<Statistics*>::iterator start = m_statsList.begin();
  while (start != m_statsList.end()) {
    if (!((*start)->isClosed()) && ((*start)->getTextId() == textId)) {
      hits.push_back(*start);
    }
    start++;
  }
  return hits;
}

std::vector<Statistics*> StatisticsManager::findStatisticsByNumericId(
    int64_t numericId) {
  std::vector<Statistics*> hits;

  std::lock_guard<decltype(m_statsListLock)> guard(m_statsListLock);

  std::vector<Statistics*>::iterator start = m_statsList.begin();
  while (start != m_statsList.end()) {
    if (!((*start)->isClosed()) && ((*start)->getNumericId() == numericId)) {
      hits.push_back(*start);
    }
    start++;
  }
  return hits;
}

Statistics* StatisticsManager::findStatisticsByUniqueId(int64_t uniqueId) {
  std::lock_guard<decltype(m_statsListLock)> guard(m_statsListLock);

  std::vector<Statistics*>::iterator start = m_statsList.begin();
  while (start != m_statsList.end()) {
    if (!((*start)->isClosed()) && ((*start)->getUniqueId() == uniqueId)) {
      Statistics* ret = *start;
      return ret;
    }
    start++;
  }
  return nullptr;
}

void StatisticsManager::deleteStatistics(Statistics*& stat) {
  if (stat->isAtomic()) {
    AtomicStatisticsImpl* ptr = dynamic_cast<AtomicStatisticsImpl*>(stat);
    delete ptr;
  } else {
    OsStatisticsImpl* ptr = dynamic_cast<OsStatisticsImpl*>(stat);
    delete ptr;
  }
  stat = nullptr;
}

void StatisticsManager::RegisterAdminRegion(
    std::shared_ptr<client::AdminRegion> adminRegPtr) {
  m_adminRegion = adminRegPtr;
}

std::shared_ptr<client::AdminRegion> StatisticsManager::getAdminRegion() {
  return m_adminRegion;
}

GeodeStatisticsFactory* StatisticsManager::getStatisticsFactory() const {
  return m_statisticsFactory.get();
}

}  // namespace statistics
}  // namespace geode
}  // namespace apache
