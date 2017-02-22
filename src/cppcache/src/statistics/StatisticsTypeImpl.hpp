#pragma once

#ifndef GEODE_STATISTICS_STATISTICSTYPEIMPL_H_
#define GEODE_STATISTICS_STATISTICSTYPEIMPL_H_

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
#include <string>
#include <geode/statistics/StatisticsType.hpp>
#include <geode/statistics/StatisticsFactory.hpp>
#include <geode/ExceptionTypes.hpp>
#include "StatsDef.hpp"

/** @file
*/

namespace apache {
namespace geode {
namespace statistics {

/**
 * Gathers together a number of {@link StatisticDescriptor statistics}
 * into one logical type.
 *
 */

typedef std::map<std::string, StatisticDescriptor*> StatisticsDescMap;

class StatisticsTypeImpl : public StatisticsType {
 private:
  int32_t statsLength;            // Total number of descriptors in the type
  std::string name;             // The name of this statistics type
  std::string description;      // The description of this statistics type
  StatisticDescriptor** stats;  // The array of stat descriptors id order
  StatisticsDescMap statsDescMap;
  int32_t intStatCount;  // Contains the number of 32-bit statistics in this type.
  int32_t longStatCount;  // Contains the number of long statistics in this type.
  int32_t
      doubleStatCount;  // Contains the number of double statistics in this type

 public:
  StatisticsTypeImpl(const char* name, const char* description,
                     StatisticDescriptor** stats, int32_t statsLength);

  ~StatisticsTypeImpl();

  ////////////////  StatisticsType(Base class) Methods ///////////////////

  const char* getName();

  const char* getDescription();

  StatisticDescriptor** getStatistics();

  int32_t nameToId(const char* name);

  StatisticDescriptor* nameToDescriptor(const char* name);

  //////////////////////  Instance Methods  //////////////////////

  /**
   *  Gets the number of statistics in this type that are ints.
   */
  int32_t getIntStatCount();

  /*
   * Gets the number of statistics in this type that are longs.
   */
  int32_t getLongStatCount();

  /*
   * Gets the number of statistics that are doubles.
   */
  int32_t getDoubleStatCount();

  /*
   * Gets the total number of statistic descriptors in the Type
   */
  int32_t getDescriptorsCount();

  // static StatisticsType[] fromXml(Reader reader,
  //                                      StatisticsTypeFactory factory);

};  // class
}  // namespace statistics
}  // namespace geode
}  // namespace apache

#endif  // GEODE_STATISTICS_STATISTICSTYPEIMPL_H_
