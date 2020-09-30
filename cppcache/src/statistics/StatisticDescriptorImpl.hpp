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

#ifndef GEODE_STATISTICS_STATISTICDESCRIPTORIMPL_H_
#define GEODE_STATISTICS_STATISTICDESCRIPTORIMPL_H_

#include <string>

#include <geode/ExceptionTypes.hpp>

#include "StatisticDescriptor.hpp"

namespace apache {
namespace geode {
namespace statistics {

typedef enum { INT_TYPE = 5, LONG_TYPE = 6, DOUBLE_TYPE = 8 } FieldType;

/**
 * Describes an individual statistic whose value is updated by an
 * application and may be archived by Geode.  These descriptions are
 * gathered together in a {@link StatisticsType}.
 *
 * <P>
 * To get an instance of this interface use an instance of
 * {@link StatisticsFactory}.
 * <P>
 * StatisticDescriptors are naturally ordered by their name.
 *
 */

class StatisticDescriptorImpl : public StatisticDescriptor {
  /** The name of the statistic */
  std::string name;

  /** A description of the statistic */
  std::string description;

  /** The unit of the statistic */
  std::string unit;

  /** Is the statistic a counter? */
  bool isStatCounter;

  /** Do larger values of the statistic indicate better performance? */
  bool isStatLargerBetter;

  /** The physical offset used to access the data that stores the
   * value for this statistic in an instance of {@link Statistics}
   */
  int32_t id;

  /**
   * Creates a new description of a statistic.
   *
   * @param statName
   *        The name of the statistic (for example,
   *        <code>"numDatabaseLookups"</code>)
   * @param statDescriptorType
   *        The type of the statistic.  This must be either
   *        <code>FieldType::INT_TYPE</code>, <code>FieldType::LONG_TYPE</code>,
   * or
   *        <code>FieldType::DOUBLE_TYPE</code>.
   * @param statDescription
   *        A description of the statistic (for example, <code>"The
   *        number of database lookups"</code>
   * @param statUnit
   *        The units that this statistic is measured in (for example,
   *        <code>"milliseconds"</code>)
   * @param statIsCounter
   *        Is this statistic a counter?  That is, does its value
   *        change monotonically (always increases or always
   *        decreases)?
   * @param statIsLargerBetter
   *        True if larger values indicate better performance.
   *
   */
  StatisticDescriptorImpl(const std::string& statName,
                          FieldType statDescriptorType,
                          const std::string& statDescription,
                          const std::string& statUnit, bool statIsCounter,
                          bool statIsLargerBetter);

  static std::shared_ptr<StatisticDescriptor> createCounter(
      const std::string& statName, FieldType fieldType,
      const std::string& description, const std::string& units,
      bool statIsCounter, bool isLargerBetter);

 public:
  /** GfFieldType defined in geode.h.
   * It describes the date type of an individual descriptor.
   * Supported date types are INT, LONG, and DOUBLE.
   */
  FieldType descriptorType;

  ~StatisticDescriptorImpl() override;

  /**
   * Returns the name of the given type code
   * Returns "int" for int_t, "long" for Long, "double" for Double
   * @throws IllegalArgumentException
   * <code>code</code> is an unknown type
   */
  static const std::string& getTypeCodeName(FieldType code);

  /**
   * Returns the number of bits needed to represent a value of the given type
   * Currently the supported types and their values are int_t :32 , Long :64,
   * Double:64
   * @throws IllegalArgumentException
   *         <code>code</code> is an unknown type
   */
  static int32_t getTypeCodeBits(FieldType code);

  /**
   * Creates a descriptor of Integer type
   * whose value behaves like a counter
   * @throws OutOfMemoryException
   */
  static std::shared_ptr<StatisticDescriptor> createIntCounter(
      const std::string& name, const std::string& description,
      const std::string& units, bool isLargerBetter);
  /**
   * Creates a descriptor of Long type
   * whose value behaves like a counter
   * @throws OutOfMemoryException
   */

  static std::shared_ptr<StatisticDescriptor> createLongCounter(
      const std::string& name, const std::string& description,
      const std::string& units, bool isLargerBetter);

  /**
   * Creates a descriptor of Double type
   * whose value behaves like a counter
   * @throws OutOfMemoryException
   */
  static std::shared_ptr<StatisticDescriptor> createDoubleCounter(
      const std::string& name, const std::string& description,
      const std::string& units, bool isLargerBetter);

  /**
   * Creates a descriptor of Integer type
   * whose value behaves like a gauge
   * @throws OutOfMemoryException
   */
  static std::shared_ptr<StatisticDescriptor> createIntGauge(
      const std::string& name, const std::string& description,
      const std::string& units, bool isLargerBetter);

  /**
   * Creates a descriptor of Long type
   * whose value behaves like a gauge
   * @throws OutOfMemoryException
   */
  static std::shared_ptr<StatisticDescriptor> createLongGauge(
      const std::string& name, const std::string& description,
      const std::string& units, bool isLargerBetter);

  /**
   * Creates a descriptor of Double type
   * whose value behaves like a gauge
   * @throws OutOfMemoryException
   */
  static std::shared_ptr<StatisticDescriptor> createDoubleGauge(
      const std::string& name, const std::string& description,
      const std::string& units, bool isLargerBetter);

  const std::string& getName() const override;

  const std::string& getDescription() const override;

  int32_t getStorageBits();

  bool isCounter() const override;

  bool isLargerBetter() const override;

  const std::string& getUnit() const override;

  int32_t getId() const override;

  /**
   * Returns the type code of this statistic
   * Possible values are:
   * GF_FIELDTYPE_INT
   * GF_FIELDTYPE_LONG
   * GF_FIELDTYPE_DOUBLE
   */
  FieldType getTypeCode() const;

  /**
   * Sets the id of this descriptor
   * An uninitialized id will be -1
   */
  void setId(int32_t statId);

  /**
   *  Checks whether the descriptor is of type int and returns the id if it is
   *  @throws IllegalArgumentException
   */
  int32_t checkInt() const;

  /**
   *  Checks whether the descriptor is of type long and returns the id if it is
   *  @throws IllegalArgumentException
   */
  int32_t checkLong() const;

  /**
   *  Checks whether the descriptor is of type double and returns the id if it i
s
   *  @throws IllegalArgumentException
   */
  int32_t checkDouble() const;

 private:
  static const std::string IntTypeName;
  static const std::string LongTypeName;
  static const std::string DoubleTypeName;

};  // class

}  // namespace statistics
}  // namespace geode
}  // namespace apache

#endif  // GEODE_STATISTICS_STATISTICDESCRIPTORIMPL_H_
