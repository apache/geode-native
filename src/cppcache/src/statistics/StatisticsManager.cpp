
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

#include <geode/geode_globals.hpp>

#include <ace/OS.h>
#include <ace/Thread_Mutex.h>
#include <ace/Time_Value.h>
#include <ace/Guard_T.h>
#include <geode/Exception.hpp>
#include "StatisticsManager.hpp"
#include <geode/Log.hpp>
#include "GeodeStatisticsFactory.hpp"
#include <string>
#include "AtomicStatisticsImpl.hpp"
#include "OsStatisticsImpl.hpp"

using namespace apache::geode::client;
using namespace apache::geode::statistics;

/**
 * static member initialization
 */

StatisticsManager::StatisticsManager(const char* filePath,
                                     int64_t sampleInterval, bool enabled,
                                     Cache* cache, const char* durableClientId,
                                     const uint32_t durableTimeout,
                                     int64_t statFileLimit,
                                     int64_t statDiskSpaceLimit)
    : m_sampler(nullptr), m_adminRegion(nullptr) {
  m_sampleIntervalMs =
      static_cast<int32_t>(sampleInterval) * 1000; /* convert to millis */
  m_newlyAddedStatsList.reserve(16);               // Allocate initial sizes
  m_statisticsFactory =
      std::unique_ptr<GeodeStatisticsFactory>(new GeodeStatisticsFactory(this));

  try {
    if (m_sampler == nullptr && enabled) {
      m_sampler = new HostStatSampler(filePath, m_sampleIntervalMs, this, cache,
                                      durableClientId, durableTimeout,
                                      statFileLimit, statDiskSpaceLimit);
      m_sampler->start();
    }
  } catch (...) {
    delete m_sampler;
    throw;
  }
}

void StatisticsManager::forceSample() {
  if (m_sampler) m_sampler->forceSample();
}

/**************************Dtor*******************************************/

StatisticsManager::~StatisticsManager() {
  try {
    // Stop the sampler
    closeSampler();

    // List should be empty if close() is called on each Stats object
    // If this is not done, delete all the pointers
    ACE_Guard<ACE_Recursive_Thread_Mutex> guard(m_statsListLock);
    int32_t count = static_cast<int32_t>(m_statsList.size());
    if (count > 0) {
      LOGFINEST("~StatisticsManager has found %d leftover statistics:", count);
      std::vector<Statistics*>::iterator iterFind = m_statsList.begin();
      while (iterFind != m_statsList.end()) {
        if (*iterFind != nullptr) {
          std::string temp((*iterFind)->getType()->getName());
          LOGFINEST("Leftover statistic: %s", temp.c_str());
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
    Log::warningCatch("~StatisticsManager swallowing Geode exception", ex);

  } catch (const std::exception& ex) {
    std::string what = "~StatisticsManager swallowing std::exception: ";
    what += ex.what();
    Log::warning(what.c_str());

  } catch (...) {
    Log::error("~StatisticsManager swallowing unknown exception");
  }
}

ACE_Recursive_Thread_Mutex& StatisticsManager::getListMutex() {
  return m_statsListLock;
}

void StatisticsManager::closeSampler() {
  if (m_sampler != nullptr) {
    m_sampler->stop();
    delete m_sampler;
    m_sampler = nullptr;
  }
}
void StatisticsManager::addStatisticsToList(Statistics* stat) {
  if (stat) {
    ACE_Guard<ACE_Recursive_Thread_Mutex> guard(m_statsListLock);
    m_statsList.push_back(stat);

    /* Add to m_newlyAddedStatsList also so that a fresh traversal not needed
    before sampling.
    After writing token to sampled file, stats ptrs will be deleted from list.
    */
    m_newlyAddedStatsList.push_back(stat);
  }
}

int32_t StatisticsManager::getStatListModCount() {
  ACE_Guard<ACE_Recursive_Thread_Mutex> guard(m_statsListLock);
  return static_cast<int32_t>(m_statsList.size());
}

std::vector<Statistics*>& StatisticsManager::getStatsList() {
  return this->m_statsList;
}

std::vector<Statistics*>& StatisticsManager::getNewlyAddedStatsList() {
  return this->m_newlyAddedStatsList;
}

Statistics* StatisticsManager::findFirstStatisticsByType(StatisticsType* type) {
  ACE_Guard<ACE_Recursive_Thread_Mutex> guard(m_statsListLock);
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

  ACE_Guard<ACE_Recursive_Thread_Mutex> guard(m_statsListLock);

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

  ACE_Guard<ACE_Recursive_Thread_Mutex> guard(m_statsListLock);

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

  ACE_Guard<ACE_Recursive_Thread_Mutex> guard(m_statsListLock);

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
  ACE_Guard<ACE_Recursive_Thread_Mutex> guard(m_statsListLock);

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
