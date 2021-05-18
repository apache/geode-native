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

#ifndef GEODE_STATISTICS_STATISTICSFACTORY_H_
#define GEODE_STATISTICS_STATISTICSFACTORY_H_

#include <vector>

#include <geode/ExceptionTypes.hpp>
#include <geode/internal/geode_globals.hpp>

#include "StatisticDescriptor.hpp"
#include "Statistics.hpp"
#include "StatisticsType.hpp"

namespace apache {
namespace geode {
namespace statistics {

/**
 * Instances of this interface provide methods that create instances
 * of {@link StatisticDescriptor} and {@link StatisticsType}.
 * Every {@link StatisticsFactory} is also a type factory.
 *
 * <P>
 *
 * A <code>StatisticsFactory</code> can create a {@link
 * StatisticDescriptor statistic} of three numeric types:
 * <code>int</code>, <code>long</code>, and <code>double</code>.  A
 * statistic (<code>StatisticDescriptor</code>) can either be a
 * <I>gauge</I> meaning that its value can increase and decrease or a
 * <I>counter</I> meaning that its value is strictly increasing.
 * Marking a statistic as a counter allows the Geode Manager Console
 * to properly display a statistics whose value "wraps around" (that
 * is, exceeds its maximum value).
 *
 */

class StatisticsFactory {
 protected:
  StatisticsFactory() = default;
  StatisticsFactory(const StatisticsFactory&) = delete;

 public:
  virtual ~StatisticsFactory() = default;

  /**
   * Creates and returns a long counter {@link StatisticDescriptor}
   * with the given <code>name</code>, <code>description</code>,
   * <code>units</code>,and with larger values indicating better performance.
   */

  virtual std::shared_ptr<StatisticDescriptor> createIntCounter(
      const std::string& name, const std::string& description,
      const std::string& units, bool largerBetter = true) = 0;

  /**
   * Creates and returns a double counter {@link StatisticDescriptor}
   * with the given <code>name</code>, <code>description</code>,
   *<code>units</code>, and with larger values indicating better performance.
   */

  virtual std::shared_ptr<StatisticDescriptor> createLongCounter(
      const std::string& name, const std::string& description,
      const std::string& units, bool largerBetter = true) = 0;

  /**
   * Creates and returns an int gauge {@link StatisticDescriptor}
   * with the given <code>name</code>, <code>description</code>,
   * <code>units</code>,  and with smaller values indicating better
   * performance.
   */

  virtual std::shared_ptr<StatisticDescriptor> createDoubleCounter(
      const std::string& name, const std::string& description,
      const std::string& units, bool largerBetter = true) = 0;

  /**
   * Creates and returns an int gauge {@link StatisticDescriptor}
   * with the given <code>name</code>, <code>description</code>,
   * <code>units</code>,  and with smaller values indicating better performance.
   */
  virtual std::shared_ptr<StatisticDescriptor> createIntGauge(
      const std::string& name, const std::string& description,
      const std::string& units, bool largerBetter = false) = 0;

  /**
   * Creates and returns an long gauge {@link StatisticDescriptor}
   * with the given <code>name</code>, <code>description</code>,
   * <code>units</code>,  and with smaller values indicating better performance.
   */
  virtual std::shared_ptr<StatisticDescriptor> createLongGauge(
      const std::string& name, const std::string& description,
      const std::string& units, bool largerBetter = false) = 0;

  /**
   * Creates and returns an double gauge {@link StatisticDescriptor}
   * with the given <code>name</code>, <code>description</code>,
   * <code>units</code>,  and with smaller values indicating better performance.
   */
  virtual std::shared_ptr<StatisticDescriptor> createDoubleGauge(
      const std::string& name, const std::string& description,
      const std::string& units, bool largerBetter = false) = 0;

  /**
   * Creates  and returns a {@link StatisticsType}
   * with the given <code>name</code>, <code>description</code>,
   * and {@link StatisticDescriptor}.
   * @throws IllegalArgumentException
   * if a type with the given <code>name</code> already exists.
   */
  virtual StatisticsType* createType(
      const std::string& name, const std::string& description,
      std::vector<std::shared_ptr<StatisticDescriptor>> stats) = 0;

  /**
   * Finds and returns an already created {@link StatisticsType}
   * with the given <code>name</code>.
   * Returns <code>null</code> if the type does not exist.
   */
  virtual StatisticsType* findType(const std::string& name) const = 0;

  /**
   * Creates and returns a {@link Statistics} instance of the given {@link
   * StatisticsType type} with default ids.
   * <p>
   * The created instance may not be {@link Statistics#isAtomic atomic}.
   */
  virtual Statistics* createStatistics(StatisticsType* type) = 0;

  /**
   * Creates and returns a {@link Statistics} instance of the given {@link
   * StatisticsType type}, <code>textId</code>, and with a default numeric id.
   * <p>
   * The created instance may not be {@link Statistics#isAtomic atomic}.
   */
  virtual Statistics* createStatistics(StatisticsType* type,
                                       const std::string& textId) = 0;

  /**
   * Creates and returns a {@link Statistics} instance of the given {@link
   * StatisticsType type}, <code>textId</code>, and <code>numericId</code>.
   * <p>
   * The created instance may not be {@link Statistics#isAtomic atomic}.
   */
  virtual Statistics* createStatistics(StatisticsType* type,
                                       const std::string& textId,
                                       int64_t numericId) = 0;

  /**
   * Creates and returns a {@link Statistics} instance of the given {@link
   * StatisticsType type} with default ids.
   * <p>
   * The created instance will be {@link Statistics#isAtomic atomic}.
   */
  virtual Statistics* createAtomicStatistics(StatisticsType* type) = 0;

  /**
   * Creates and returns a {@link Statistics} instance of the given {@link
   * StatisticsType type}, <code>textId</code>, and with a default numeric id.
   * <p>
   * The created instance will be {@link Statistics#isAtomic atomic}.
   */
  virtual Statistics* createAtomicStatistics(StatisticsType* type,
                                             const std::string& textId) = 0;

  /**
   * Creates and returns a {@link Statistics} instance of the given {@link
   * StatisticsType type}, <code>textId</code>, and <code>numericId</code>.
   * <p>
   * The created instance will be {@link Statistics#isAtomic atomic}.
   */
  virtual Statistics* createAtomicStatistics(StatisticsType* type,
                                             const std::string& textId,
                                             int64_t numericId) = 0;

  /** Return the first instance that matches the type, or nullptr */
  virtual Statistics* findFirstStatisticsByType(
      const StatisticsType* type) const = 0;

  /**
   * Returns a name that can be used to identify the manager
   */
  virtual const std::string& getName() const = 0;

  /**
   * Returns a numeric id that can be used to identify the manager
   */
  virtual int64_t getId() const = 0;

  StatisticsFactory& operator=(const StatisticsFactory&) = delete;

};  // class
}  // namespace statistics
}  // namespace geode
}  // namespace apache

#endif  // GEODE_STATISTICS_STATISTICSFACTORY_H_
