
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

#include <ace/Recursive_Thread_Mutex.h>
#include <ace/OS.h>
#include <ace/Thread_Mutex.h>
#include <ace/Guard_T.h>
#include <geode/Exception.hpp>
#include "GeodeStatisticsFactory.hpp"
#include <geode/Log.hpp>
#include <string>
#include "AtomicStatisticsImpl.hpp"
#include "OsStatisticsImpl.hpp"
#include "HostStatHelper.hpp"

using namespace apache::geode::client;
using namespace apache::geode::statistics;

GeodeStatisticsFactory::GeodeStatisticsFactory(StatisticsManager* statMngr) {
  m_name = "GeodeStatisticsFactory";
  m_id = ACE_OS::getpid();
  m_statsListUniqueId = 1;

  m_statMngr = statMngr;
}

GeodeStatisticsFactory::~GeodeStatisticsFactory() {
  try {
    m_statMngr = nullptr;

    // Clean Map : Delete all the pointers of StatisticsType from the map.
    if (statsTypeMap.total_size() == 0) return;

    ACE_Map_Manager<std::string, StatisticsTypeImpl*,
                    ACE_Recursive_Thread_Mutex>::iterator iterFind =
        statsTypeMap.begin();
    while (iterFind != statsTypeMap.end()) {
      delete (*iterFind).int_id_;
      (*iterFind).int_id_ = nullptr;
      iterFind++;
    }
    statsTypeMap.unbind_all();

  } catch (const Exception& ex) {
    Log::warningCatch("~GeodeStatisticsFactory swallowing Geode exception", ex);

  } catch (const std::exception& ex) {
    std::string what = "~GeodeStatisticsFactory swallowing std::exception: ";
    what += ex.what();
    LOGWARN(what.c_str());

  } catch (...) {
    LOGERROR("~GeodeStatisticsFactory swallowing unknown exception");
  }
}

const char* GeodeStatisticsFactory::getName() { return m_name; }

int64_t GeodeStatisticsFactory::getId() { return m_id; }

Statistics* GeodeStatisticsFactory::createStatistics(StatisticsType* type) {
  return createAtomicStatistics(type, nullptr, 0);
}

Statistics* GeodeStatisticsFactory::createStatistics(StatisticsType* type,
                                                     const char* textId) {
  return createAtomicStatistics(type, textId, 0);
}

Statistics* GeodeStatisticsFactory::createStatistics(StatisticsType* type,
                                                     const char* textId,
                                                     int64_t numericId) {
  return createAtomicStatistics(type, textId, 0);
}

Statistics* GeodeStatisticsFactory::createOsStatistics(StatisticsType* type,
                                                       const char* textId,
                                                       int64_t numericId) {
  // Validate input
  if (type == nullptr) {
    throw IllegalArgumentException("StatisticsType* is Null");
  }

  int64_t myUniqueId;
  {
    ACE_Guard<ACE_Recursive_Thread_Mutex> guard(m_statsListUniqueIdLock);
    myUniqueId = m_statsListUniqueId++;
  }

  Statistics* result =
      new OsStatisticsImpl(type, textId, numericId, myUniqueId, this);
  { m_statMngr->addStatisticsToList(result); }

  return result;
}

Statistics* GeodeStatisticsFactory::createAtomicStatistics(
    StatisticsType* type) {
  return createAtomicStatistics(type, nullptr, 0);
}

Statistics* GeodeStatisticsFactory::createAtomicStatistics(StatisticsType* type,
                                                           const char* textId) {
  return createAtomicStatistics(type, textId, 0);
}

Statistics* GeodeStatisticsFactory::createAtomicStatistics(StatisticsType* type,
                                                           const char* textId,
                                                           int64_t numericId) {
  // Validate input
  if (type == nullptr) {
    throw IllegalArgumentException("StatisticsType* is Null");
  }
  int64_t myUniqueId;

  {
    ACE_Guard<ACE_Recursive_Thread_Mutex> guard(m_statsListUniqueIdLock);
    myUniqueId = m_statsListUniqueId++;
  }

  Statistics* result =
      new AtomicStatisticsImpl(type, textId, numericId, myUniqueId, this);

  { m_statMngr->addStatisticsToList(result); }

  return result;
}

Statistics* GeodeStatisticsFactory::findFirstStatisticsByType(
    StatisticsType* type) {
  return (m_statMngr->findFirstStatisticsByType(type));
}

StatisticsTypeImpl* GeodeStatisticsFactory::addType(StatisticsTypeImpl* st) {
  StatisticsTypeImpl* st1;
  std::string temp(st->getName());
  int status;
  try {
    status = statsTypeMap.rebind(temp, st, st1);
  } catch (const std::exception& ex) {
    throw IllegalArgumentException(ex.what());
  } catch (...) {
    throw IllegalArgumentException("addType: unknown exception");
  }
  if (status == 1) {
  } else if (status == -1) {
    throw IllegalArgumentException(
        "GeodeStatisticsFactory::addType: failed "
        "to add new type %s",
        temp.c_str());
  }
  return st;
}

/**
 * Creates  a StatisticType for the given shared class.
 */
StatisticsType* GeodeStatisticsFactory::createType(const char* name,
                                                   const char* description,
                                                   StatisticDescriptor** stats,
                                                   int32_t statsLength) {
  StatisticsTypeImpl* st =
      new StatisticsTypeImpl(name, description, stats, statsLength);

  if (st != nullptr) {
    st = addType(st);
  } else {
    throw OutOfMemoryException(
        "GeodeStatisticsFactory::createType :: out memory");
  }
  return st;
}

StatisticsType* GeodeStatisticsFactory::findType(const char* name) {
  std::string statName = name;
  StatisticsTypeImpl* st = nullptr;
  int status = statsTypeMap.find(statName, st);
  if (status == -1) {
    std::string temp(name);
    std::string s = "There is no statistic named \"" + temp + "\"";
    // LOGWARN(s.c_str());
    // throw IllegalArgumentException(s.c_str());
    return nullptr;
  } else {
    return st;
  }
}

StatisticDescriptor* GeodeStatisticsFactory::createIntCounter(
    const char* name, const char* description, const char* units,
    bool largerBetter) {
  return StatisticDescriptorImpl::createIntCounter(name, description, units,
                                                   largerBetter);
}

StatisticDescriptor* GeodeStatisticsFactory::createLongCounter(
    const char* name, const char* description, const char* units,
    bool largerBetter) {
  return StatisticDescriptorImpl::createLongCounter(name, description, units,
                                                    largerBetter);
}

StatisticDescriptor* GeodeStatisticsFactory::createDoubleCounter(
    const char* name, const char* description, const char* units,
    bool largerBetter) {
  return StatisticDescriptorImpl::createDoubleCounter(name, description, units,
                                                      largerBetter);
}

StatisticDescriptor* GeodeStatisticsFactory::createIntGauge(
    const char* name, const char* description, const char* units,
    bool largerBetter) {
  return StatisticDescriptorImpl::createIntGauge(name, description, units,
                                                 largerBetter);
}

StatisticDescriptor* GeodeStatisticsFactory::createLongGauge(
    const char* name, const char* description, const char* units,
    bool largerBetter) {
  return StatisticDescriptorImpl::createLongGauge(name, description, units,
                                                  largerBetter);
}

StatisticDescriptor* GeodeStatisticsFactory::createDoubleGauge(
    const char* name, const char* description, const char* units,
    bool largerBetter) {
  return StatisticDescriptorImpl::createDoubleGauge(name, description, units,
                                                    largerBetter);
}
