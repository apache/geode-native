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

#ifndef GEODE_STATISTICS_STATISTICSTYPEIMPL_H_
#define GEODE_STATISTICS_STATISTICSTYPEIMPL_H_

#include <map>
#include <string>
#include <vector>

#include <geode/ExceptionTypes.hpp>

#include "StatisticsFactory.hpp"
#include "StatisticsType.hpp"
#include "StatsDef.hpp"

namespace apache {
namespace geode {
namespace statistics {

/**
 * Gathers together a number of {@link StatisticDescriptor statistics}
 * into one logical type.
 *
 */

typedef std::map<std::string, std::shared_ptr<StatisticDescriptor>>
    StatisticsDescMap;

class StatisticsTypeImpl : public StatisticsType {
  std::string name;
  std::string description;
  std::vector<std::shared_ptr<StatisticDescriptor>> stats;
  StatisticsDescMap statsDescMap;
  int32_t intStatCount;
  int32_t longStatCount;
  int32_t doubleStatCount;

 public:
  StatisticsTypeImpl(std::string name, std::string description,
                     std::vector<std::shared_ptr<StatisticDescriptor>> stats);

  ~StatisticsTypeImpl() override;

  const std::string& getName() const override;

  const std::string& getDescription() const override;

  const std::vector<std::shared_ptr<StatisticDescriptor>>& getStatistics()
      const override;

  int32_t nameToId(const std::string& name) const override;

  std::shared_ptr<StatisticDescriptor> nameToDescriptor(
      const std::string& name) const override;

  /**
   *  Gets the number of statistics in this type that are ints.
   */
  int32_t getIntStatCount() const;

  /*
   * Gets the number of statistics in this type that are longs.
   */
  int32_t getLongStatCount() const;

  /*
   * Gets the number of statistics that are doubles.
   */
  int32_t getDoubleStatCount() const;

  /*
   * Gets the total number of statistic descriptors in the Type
   */
  size_t getDescriptorsCount() const override;
};  // class
}  // namespace statistics
}  // namespace geode
}  // namespace apache

#endif  // GEODE_STATISTICS_STATISTICSTYPEIMPL_H_
