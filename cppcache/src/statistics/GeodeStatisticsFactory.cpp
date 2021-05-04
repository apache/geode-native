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

#include "GeodeStatisticsFactory.hpp"

#include <string>

#include <boost/process/environment.hpp>

#include <geode/Exception.hpp>
#include <geode/internal/geode_globals.hpp>
#include <geode/util/LogLevel.hpp>

#include "../util/Log.hpp"
#include "AtomicStatisticsImpl.hpp"
#include "OsStatisticsImpl.hpp"
#include "StatisticDescriptorImpl.hpp"

namespace apache {
namespace geode {
namespace statistics {

using client::Exception;
using client::IllegalArgumentException;
using client::OutOfMemoryException;

GeodeStatisticsFactory::GeodeStatisticsFactory(StatisticsManager* statMngr) {
  m_name = "GeodeStatisticsFactory";
  m_id = boost::this_process::get_id();
  m_statsListUniqueId = 1;

  m_statMngr = statMngr;
}

GeodeStatisticsFactory::~GeodeStatisticsFactory() {
  try {
    m_statMngr = nullptr;

    // Clean Map : Delete all the pointers of StatisticsType from the map.
    std::lock_guard<decltype(statsTypeMap)::mutex_type> lock(
        statsTypeMap.mutex());
    if (statsTypeMap.empty()) return;

    for (auto& entry : statsTypeMap) {
      delete entry.second;
    }
    statsTypeMap.clear();

  } catch (const Exception& ex) {
    std::string message = "Geode exception " + ex.getName() +
                          " caught: " + ex.getMessage() +
                          "\n~StatisticsFactory swallowing Geode exception";
    LOG_WARN(message);
  } catch (const std::exception& ex) {
    std::string what = "~GeodeStatisticsFactory swallowing std::exception: ";
    what += ex.what();
    LOG_WARN(what.c_str());
  } catch (...) {
    LOG_ERROR("~GeodeStatisticsFactory swallowing unknown exception");
  }
}

const std::string& GeodeStatisticsFactory::getName() const { return m_name; }

int64_t GeodeStatisticsFactory::getId() const { return m_id; }

Statistics* GeodeStatisticsFactory::createStatistics(StatisticsType* type) {
  return createAtomicStatistics(type, nullptr, 0);
}

Statistics* GeodeStatisticsFactory::createStatistics(
    StatisticsType* type, const std::string& textId) {
  return createAtomicStatistics(type, textId, 0);
}

Statistics* GeodeStatisticsFactory::createStatistics(StatisticsType* type,
                                                     const std::string& textId,
                                                     int64_t numericId) {
  return createAtomicStatistics(type, textId, numericId);
}

Statistics* GeodeStatisticsFactory::createOsStatistics(
    StatisticsType* type, const std::string& textId, int64_t numericId) {
  // Validate input
  if (type == nullptr) {
    throw IllegalArgumentException("StatisticsType* is Null");
  }

  int64_t myUniqueId;
  {
    std::lock_guard<decltype(m_statsListUniqueIdLock)> guard(
        m_statsListUniqueIdLock);
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

Statistics* GeodeStatisticsFactory::createAtomicStatistics(
    StatisticsType* type, const std::string& textId) {
  return createAtomicStatistics(type, textId, 0);
}

Statistics* GeodeStatisticsFactory::createAtomicStatistics(
    StatisticsType* type, const std::string& textId, int64_t numericId) {
  // Validate input
  if (type == nullptr) {
    throw IllegalArgumentException("StatisticsType* is Null");
  }
  int64_t myUniqueId;

  {
    std::lock_guard<decltype(m_statsListUniqueIdLock)> guard(
        m_statsListUniqueIdLock);
    myUniqueId = m_statsListUniqueId++;
  }

  Statistics* result =
      new AtomicStatisticsImpl(type, textId, numericId, myUniqueId, this);

  { m_statMngr->addStatisticsToList(result); }

  return result;
}

Statistics* GeodeStatisticsFactory::findFirstStatisticsByType(
    const StatisticsType* type) const {
  return (m_statMngr->findFirstStatisticsByType(type));
}

StatisticsTypeImpl* GeodeStatisticsFactory::addType(StatisticsTypeImpl* st) {
  const auto& name = st->getName();
  try {
    auto status = statsTypeMap.emplace(name, st);
    if (!status.second) {
      throw IllegalArgumentException(
          "GeodeStatisticsFactory::addType: failed to add new type " + name);
    }
  } catch (const std::exception& ex) {
    throw IllegalArgumentException(ex.what());
  } catch (...) {
    throw IllegalArgumentException("addType: unknown exception");
  }
  return st;
}

/**
 * Creates  a StatisticType for the given shared class.
 */
StatisticsType* GeodeStatisticsFactory::createType(
    const std::string& name, const std::string& description,
    std::vector<std::shared_ptr<StatisticDescriptor>> stats) {
  auto st = new StatisticsTypeImpl(name, description, std::move(stats));

  if (st != nullptr) {
    st = addType(st);
  } else {
    throw OutOfMemoryException(
        "GeodeStatisticsFactory::createType :: out memory");
  }
  return st;
}

StatisticsType* GeodeStatisticsFactory::findType(
    const std::string& name) const {
  auto&& lock = statsTypeMap.make_lock();
  const auto& entry = statsTypeMap.find(name);
  if (entry == statsTypeMap.end()) {
    std::string s = "There is no statistic named \"" + name + "\"";
    return nullptr;
  } else {
    return entry->second;
  }
}

std::shared_ptr<StatisticDescriptor> GeodeStatisticsFactory::createIntCounter(
    const std::string& name, const std::string& description,
    const std::string& units, bool largerBetter) {
  return StatisticDescriptorImpl::createIntCounter(name, description, units,
                                                   largerBetter);
}

std::shared_ptr<StatisticDescriptor> GeodeStatisticsFactory::createLongCounter(
    const std::string& name, const std::string& description,
    const std::string& units, bool largerBetter) {
  return StatisticDescriptorImpl::createLongCounter(name, description, units,
                                                    largerBetter);
}

std::shared_ptr<StatisticDescriptor>
GeodeStatisticsFactory::createDoubleCounter(const std::string& name,
                                            const std::string& description,
                                            const std::string& units,
                                            bool largerBetter) {
  return StatisticDescriptorImpl::createDoubleCounter(name, description, units,
                                                      largerBetter);
}

std::shared_ptr<StatisticDescriptor> GeodeStatisticsFactory::createIntGauge(
    const std::string& name, const std::string& description,
    const std::string& units, bool largerBetter) {
  return StatisticDescriptorImpl::createIntGauge(name, description, units,
                                                 largerBetter);
}

std::shared_ptr<StatisticDescriptor> GeodeStatisticsFactory::createLongGauge(
    const std::string& name, const std::string& description,
    const std::string& units, bool largerBetter) {
  return StatisticDescriptorImpl::createLongGauge(name, description, units,
                                                  largerBetter);
}

std::shared_ptr<StatisticDescriptor> GeodeStatisticsFactory::createDoubleGauge(
    const std::string& name, const std::string& description,
    const std::string& units, bool largerBetter) {
  return StatisticDescriptorImpl::createDoubleGauge(name, description, units,
                                                    largerBetter);
}

}  // namespace statistics
}  // namespace geode
}  // namespace apache
