#pragma once

#ifndef GEODE_STATISTICS_ATOMICSTATISTICSIMPL_H_
#define GEODE_STATISTICS_ATOMICSTATISTICSIMPL_H_

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

#include <atomic>

#include <geode/statistics/Statistics.hpp>
#include "StatisticsTypeImpl.hpp"
#include <geode/statistics/StatisticsFactory.hpp>
#include <string>

#include <NonCopyable.hpp>

using namespace apache::geode::client;

/** @file
*/

namespace apache {
namespace geode {
namespace statistics {

/**
 * An implementation of {@link Statistics} that stores its statistics
 * in local memory and support atomic operations
 *
 */

/* adongre
 * CID 28732: Other violation (MISSING_COPY)
 * Class "apache::geode::statistics::AtomicStatisticsImpl" owns resources that
 * are
 * managed
 * in its constructor and destructor but has no user-written copy constructor.
 *
 * CID 28718: Other violation (MISSING_ASSIGN)
 * Class "apache::geode::statistics::AtomicStatisticsImpl" owns resources that
 * are
 * managed in its constructor and destructor but has no user-written assignment
 * operator.
 *
 * FIX : Make the class non-copyable
 */
class AtomicStatisticsImpl : public Statistics, private NonCopyable {
 private:
  /**********varbs originally kept in statisticsimpl class*****************/
  /** The type of this statistics instance */
  StatisticsTypeImpl* statsType;

  /** The display name of this statistics instance */
  const char* textId;
  std::string textIdStr;

  /** Numeric information display with these statistics */
  int64_t numericId;

  /** Are these statistics closed? */
  bool closed;

  /** Uniquely identifies this instance */
  int64_t uniqueId;

  /****************************************************************************/
  /** An array containing the values of the int32_t statistics */
  std::atomic<int32_t>* intStorage;

  /** An array containing the values of the int64_t statistics */
  std::atomic<int64_t>* longStorage;

  /** An array containing the values of the double statistics */
  std::atomic<double>* doubleStorage;

  ///////////////////////Private Methods//////////////////////////
  bool isOpen();

  int32_t getIntId(StatisticDescriptor* descriptor);

  int32_t getLongId(StatisticDescriptor* descriptor);

  int32_t getDoubleId(StatisticDescriptor* descriptor);

  //////////////////////  Static private Methods  //////////////////////

  int64_t calcNumericId(StatisticsFactory* system, int64_t userValue);

  const char* calcTextId(StatisticsFactory* system, const char* userValue);

  ///////////////////////  Constructors  ///////////////////////

  /**
   * Creates a new statistics instance of the given type
   *
   * @param type
   *        A description of the statistics
   * @param textId
   *        Text that identifies this statistic when it is monitored
   * @param numericId
   *        A number that displayed when this statistic is monitored
   * @param uniqueId
   *        A number that uniquely identifies this instance
   * @param system
   *        The distributed system that determines whether or not these
   *        statistics are stored (and collected) in local memory
   */

 public:
  AtomicStatisticsImpl(StatisticsType* type, const char* textId,
                       int64_t numericId, int64_t uniqueId,
                       StatisticsFactory* system);

  //////////////////////  Instance Methods  //////////////////////
  ~AtomicStatisticsImpl();

  bool usesSystemCalls();

  int32_t nameToId(const char* name);

  StatisticDescriptor* nameToDescriptor(const char* name);

  bool isClosed();

  bool isShared();

  bool isAtomic();

  void close();
  /////////////////////////Attribute methods//////////////////////////

  StatisticsType* getType();

  const char* getTextId();

  int64_t getNumericId();

  int64_t getUniqueId();

  ////////////////////////  set() Methods  ///////////////////////

  void setInt(char* name, int32_t value);

  void setInt(StatisticDescriptor* descriptor, int32_t value);

  void setInt(int32_t offset, int32_t value);

  void setLong(char* name, int64_t value);

  void setLong(StatisticDescriptor* descriptor, int64_t value);

  void setLong(int32_t id, int64_t value);

  void setDouble(char* name, double value);

  void setDouble(StatisticDescriptor* descriptor, double value);

  void setDouble(int32_t id, double value);

  ///////////////////////  get() Methods  ///////////////////////

  int32_t getInt(char* name);

  int32_t getInt(StatisticDescriptor* descriptor);

  int32_t getInt(int32_t offset);

  int64_t getLong(char* name);

  int64_t getLong(StatisticDescriptor* descriptor);

  int64_t getLong(int32_t id);

  double getDouble(char* name);

  double getDouble(StatisticDescriptor* descriptor);

  double getDouble(int32_t id);

  int64_t getRawBits(StatisticDescriptor* descriptor);

  int64_t getRawBits(char* name);

  ////////////////////////  inc() Methods  ////////////////////////

  int32_t incInt(char* name, int32_t delta);

  int32_t incInt(StatisticDescriptor* descriptor, int32_t delta);

  int32_t incInt(int32_t offset, int32_t delta);

  int64_t incLong(char* name, int64_t delta);

  int64_t incLong(StatisticDescriptor* descriptor, int64_t delta);

  int64_t incLong(int32_t id, int64_t delta);

  double incDouble(char* name, double delta);

  double incDouble(StatisticDescriptor* descriptor, double delta);

  double incDouble(int32_t id, double delta);

 protected:
  void _setInt(int32_t offset, int32_t value);

  void _setLong(int32_t offset, int64_t value);

  void _setDouble(int32_t offset, double value);

  int32_t _getInt(int32_t offset);

  int64_t _getLong(int32_t offset);

  double _getDouble(int32_t offset);

  /**
   * Returns the bits that represent the raw value of the
   * specified statistic descriptor.
   */
  int64_t _getRawBits(StatisticDescriptor* stat);

  int32_t _incInt(int32_t offset, int32_t delta);

  int64_t _incLong(int32_t offset, int64_t delta);

  double _incDouble(int32_t offset, double delta);

};  // class

}  // namespace statistics
}  // namespace geode
}  // namespace apache

#endif  // GEODE_STATISTICS_ATOMICSTATISTICSIMPL_H_
