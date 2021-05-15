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

#include "StatisticsTypeImpl.hpp"

#include <string>

#include "../util/Log.hpp"
#include "StatisticDescriptorImpl.hpp"

namespace apache {
namespace geode {
namespace statistics {

using client::IllegalArgumentException;
using client::NullPointerException;

StatisticsTypeImpl::StatisticsTypeImpl(
    std::string nameArg, std::string descriptionArg,
    std::vector<std::shared_ptr<StatisticDescriptor>> statsArg) {
  if (nameArg.empty()) {
    const char* s = "Cannot have an empty statistics type name";
    throw NullPointerException(s);
  }
  if (stats.size() > MAX_DESCRIPTORS_PER_TYPE) {
    throw IllegalArgumentException(
        "The requested descriptor count " + std::to_string(stats.size()) +
        " exceeds the maximum which is " +
        std::to_string(MAX_DESCRIPTORS_PER_TYPE) + ".");
  }
  this->name = nameArg;
  this->description = descriptionArg;
  this->stats = std::move(statsArg);
  int32_t intCount = 0;
  int32_t longCount = 0;
  int32_t doubleCount = 0;
  for (auto stat : stats) {
    // Concrete class required to set the ids only.
    if (auto sd = std::dynamic_pointer_cast<StatisticDescriptorImpl>(stat)) {
      if (sd->getTypeCode() == INT_TYPE) {
        sd->setId(intCount);
        intCount++;
      } else if (sd->getTypeCode() == LONG_TYPE) {
        sd->setId(longCount);
        longCount++;
      } else if (sd->getTypeCode() == DOUBLE_TYPE) {
        sd->setId(doubleCount);
        doubleCount++;
      }
      std::string str = stat->getName();
      StatisticsDescMap::iterator iterFind = statsDescMap.find(str);
      if (iterFind != statsDescMap.end()) {
        throw IllegalArgumentException("Duplicate StatisticDescriptor named " +
                                       sd->getName());
      } else {
        // statsDescMap.insert(make_pair(stats[i]->getName(), stats[i]));
        statsDescMap.insert(
            StatisticsDescMap::value_type(stat->getName(), stat));
      }
    }
  }  // for
  this->intStatCount = intCount;
  this->longStatCount = longCount;
  this->doubleStatCount = doubleCount;
}

StatisticsTypeImpl::~StatisticsTypeImpl() {}

const std::string& StatisticsTypeImpl::getName() const { return name; }

const std::string& StatisticsTypeImpl::getDescription() const {
  return description;
}

const std::vector<std::shared_ptr<StatisticDescriptor>>&
StatisticsTypeImpl::getStatistics() const {
  return stats;
}

int32_t StatisticsTypeImpl::nameToId(const std::string& nameArg) const {
  return nameToDescriptor(nameArg)->getId();
}

std::shared_ptr<StatisticDescriptor> StatisticsTypeImpl::nameToDescriptor(
    const std::string& nameArg) const {
  const auto iterFind = statsDescMap.find(nameArg);
  if (iterFind == statsDescMap.end()) {
    std::string s = "There is no statistic named " + nameArg +
                    " in this statistics instance ";
    LOG_WARN("StatisticsTypeImpl::nameToDescriptor %s", s.c_str());
    throw IllegalArgumentException(s);
  } else {
    return iterFind->second;
  }
}

int32_t StatisticsTypeImpl::getIntStatCount() const { return intStatCount; }

int32_t StatisticsTypeImpl::getLongStatCount() const { return longStatCount; }

int32_t StatisticsTypeImpl::getDoubleStatCount() const {
  return doubleStatCount;
}

size_t StatisticsTypeImpl::getDescriptorsCount() const { return stats.size(); }

}  // namespace statistics
}  // namespace geode
}  // namespace apache
