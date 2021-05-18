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

#ifndef GEODE_STATISTICS_STATISTICS_H_
#define GEODE_STATISTICS_STATISTICS_H_

#include <string>

#include <geode/internal/geode_globals.hpp>

#include "StatisticDescriptor.hpp"
#include "StatisticsType.hpp"

namespace apache {
namespace geode {
namespace statistics {

/**
 * An instantiation of an existing <code>StatisticsType</code> object with
 * methods for
 * setting, incrementing and getting individual <code>StatisticDescriptor</code>
 * values.
 */
class Statistics {
 public:
  /**
   * Closes these statistics.  After statistics have been closed, they
   * are no longer archived.
   * A value access on a closed statistics always results in zero.
   * A value modification on a closed statistics is ignored.
   */
  virtual void close() = 0;

  /**
   * Returns the id of the statistic with the given name in this
   * statistics instance.
   *
   * @param name statistic name
   * @throws IllegalArgumentException
   *         No statistic named <code>name</code> exists in this
   *         statistics instance.
   *
   * @see StatisticsType#nameToDescriptor
   */
  virtual int32_t nameToId(const std::string& name) const = 0;

  /**
   * Returns the descriptor of the statistic with the given name in this
   * statistics instance.
   *
   * @param name statistic name
   * @throws IllegalArgumentException
   *         No statistic named <code>name</code> exists in this
   *         statistics instance.
   *
   * @see StatisticsType#nameToId
   */
  virtual std::shared_ptr<StatisticDescriptor> nameToDescriptor(
      const std::string& name) const = 0;

  /**
   * Gets a value that uniquely identifies this statistics.
   */
  virtual int64_t getUniqueId() const = 0;

  /**
   * Gets the {@link StatisticsType} of this instance.
   */
  virtual StatisticsType* getType() const = 0;

  /**
   * Gets the text associated with this instance that helps identify it.
   */
  virtual const std::string& getTextId() const = 0;

  /**
   * Gets the number associated with this instance that helps identify it.
   */
  virtual int64_t getNumericId() const = 0;

  /**
   * Returns true if modifications are atomic. This means that multiple threads
   * can safely modify this instance without additional synchronization.
   * <p>
   * Returns false if modifications are not atomic. This means that
   * modifications
   * to this instance are cheaper but not thread safe.
   * <P>
   * Note that all instances that are {@link #isShared shared} are also atomic.
   */
  virtual bool isAtomic() const = 0;

  /**
   * Returns true if the data for this instance is stored in shared memory.
   * Returns false if the data is store in local memory.
   * <P>
   * Note that all instances that are {@link #isShared shared} are also atomic.
   */
  virtual bool isShared() const = 0;

  /**
   * Returns true if the instance has been {@link #close closed}.
   */
  virtual bool isClosed() const = 0;

  /**
   * Sets the value of a statistic with the given <code>id</code>
   * whose type is <code>int</code>.
   * @param id a statistic id obtained with {@link #nameToId}
   * or {@link StatisticsType#nameToId}.
   * @param value value to set
   *
   * @throws IllegalArgumentException
   *         If the id is invalid.
   */
  virtual void setInt(int32_t id, int32_t value) = 0;

  /**
   * Sets the value of a named statistic of type <code>int</code>
   *
   * @param name statistic name
   * @param value value to set
   * @throws IllegalArgumentException
   *         If no statistic exists named <code>name</code> or
   *         if the statistic with name <code>name</code> is not of
   *         type <code>int</code>.
   */
  virtual void setInt(const std::string& name, int32_t value) = 0;

  /**
   * Sets the value of a described statistic of type <code>int</code>
   *
   * @param descriptor a statistic descriptor obtained with {@link
   * #nameToDescriptor}
   * or {@link StatisticsType#nameToDescriptor}.
   * @param value value to set
   * @throws IllegalArgumentException
   *         If no statistic exists for the given <code>descriptor</code> or
   *         if the described statistic is not of
   *         type <code>int</code>.
   */
  virtual void setInt(const std::shared_ptr<StatisticDescriptor> descriptor,
                      int32_t value) = 0;

  /**
   * Sets the value of a statistic with the given <code>id</code>
   * whose type is <code>long</code>.
   * @param id a statistic id obtained with {@link #nameToId}
   * or {@link StatisticsType#nameToId}.
   * @param value value to set
   *
   * @throws IllegalArgumentException
   *         If the id is invalid.
   */

  virtual void setLong(int32_t id, int64_t value) = 0;
  /**
   * Sets the value of a described statistic of type <code>long</code>
   *
   * @param descriptor a statistic descriptor obtained with {@link
   * #nameToDescriptor}
   * or {@link StatisticsType#nameToDescriptor}.
   * @param value value to set
   * @throws IllegalArgumentException
   *         If no statistic exists for the given <code>descriptor</code> or
   *         if the described statistic is not of
   *         type <code>long</code>.
   */
  virtual void setLong(const std::shared_ptr<StatisticDescriptor> descriptor,
                       int64_t value) = 0;

  /**
   * Sets the value of a named statistic of type <code>long</code>.
   *
   * @param name statistic name
   * @param value value to set
   * @throws IllegalArgumentException
   *         If no statistic exists named <code>name</code> or
   *         if the statistic with name <code>name</code> is not of
   *         type <code>long</code>.
   */
  virtual void setLong(const std::string& name, int64_t value) = 0;

  /**
   * Sets the value of a statistic with the given <code>id</code>
   * whose type is <code>double</code>.
   * @param id a statistic id obtained with {@link #nameToId}
   * or {@link StatisticsType#nameToId}.
   * @param value value to set
   *
   * @throws IllegalArgumentException
   *         If the id is invalid.
   */
  virtual void setDouble(int32_t id, double value) = 0;

  /**
   * Sets the value of a described statistic of type <code>double</code>
   *
   * @param descriptor a statistic descriptor obtained with {@link
   * #nameToDescriptor}
   * or {@link StatisticsType#nameToDescriptor}.
   * @param value value to set
   * @throws IllegalArgumentException
   *         If no statistic exists for the given <code>descriptor</code> or
   *         if the described statistic is not of
   *         type <code>double</code>.
   */
  virtual void setDouble(const std::shared_ptr<StatisticDescriptor> descriptor,
                         double value) = 0;

  /**
   * Sets the value of a named statistic of type <code>double</code>.
   *
   * @param name statistic name
   * @param value value to set
   * @throws IllegalArgumentException
   *         If no statistic exists named <code>name</code> or
   *         if the statistic with name <code>name</code> is not of
   *         type <code>double</code>.
   */
  virtual void setDouble(const std::string& name, double value) = 0;

  ///////////////////////  get() Methods  ///////////////////////

  /**
   * Returns the value of the identified statistic of type <code>int</code>.
   *
   * @param id a statistic id obtained with {@link #nameToId}
   * or {@link StatisticsType#nameToId}.
   * @throws IllegalArgumentException
   *         If the id is invalid.
   */
  virtual int32_t getInt(int32_t id) const = 0;

  /**
   * Returns the value of the described statistic of type <code>int</code>.
   *
   * @param descriptor a statistic descriptor obtained with {@link
   * #nameToDescriptor}
   * or {@link StatisticsType#nameToDescriptor}.
   * @throws IllegalArgumentException
   *         If no statistic exists with the specified <code>descriptor</code>
   * or
   *         if the described statistic is not of
   *         type <code>int</code>.
   */
  virtual int32_t getInt(
      const std::shared_ptr<StatisticDescriptor> descriptor) const = 0;

  /**
   * Returns the value of the statistic of type <code>int</code> at
   * the given name.
   * @param name statistic name
   *
   * @throws IllegalArgumentException
   *         If no statistic exists with name <code>name</code> or
   *         if the statistic named <code>name</code> is not of
   *         type <code>int</code>.
   */
  virtual int32_t getInt(const std::string& name) const = 0;

  /**
   * Returns the value of the identified statistic of type <code>long</code>.
   *
   * @param id a statistic id obtained with {@link #nameToId}
   * or {@link StatisticsType#nameToId}.
   * @throws IllegalArgumentException
   *         If the id is invalid.
   */
  virtual int64_t getLong(int32_t id) const = 0;

  /**
   * Returns the value of the described statistic of type <code>long</code>.
   *
   * @param descriptor a statistic descriptor obtained with {@link
   * #nameToDescriptor}
   * or {@link StatisticsType#nameToDescriptor}.
   * @throws IllegalArgumentException
   *         If no statistic exists with the specified <code>descriptor</code>
   * or
   *         if the described statistic is not of
   *         type <code>long</code>.
   */
  virtual int64_t getLong(
      const std::shared_ptr<StatisticDescriptor> descriptor) const = 0;

  /**
   * Returns the value of the statistic of type <code>long</code> at
   * the given name.
   *
   * @param name statistic name
   * @throws IllegalArgumentException
   *         If no statistic exists with name <code>name</code> or
   *         if the statistic named <code>name</code> is not of
   *         type <code>long</code>.
   */
  virtual int64_t getLong(const std::string& name) const = 0;

  /**
   * Returns the value of the identified statistic of type <code>double</code>.
   *
   * @param id a statistic id obtained with {@link #nameToId}
   * or {@link StatisticsType#nameToId}.
   * @throws IllegalArgumentException
   *         If the id is invalid.
   */
  virtual double getDouble(int32_t id) const = 0;

  /**
   * Returns the value of the described statistic of type <code>double</code>.
   *
   * @param descriptor a statistic descriptor obtained with {@link
   * #nameToDescriptor}
   * or {@link StatisticsType#nameToDescriptor}.
   * @throws IllegalArgumentException
   *         If no statistic exists with the specified <code>descriptor</code>
   * or
   *         if the described statistic is not of
   *         type <code>double</code>.
   */
  virtual double getDouble(
      const std::shared_ptr<StatisticDescriptor> descriptor) const = 0;

  /**
   * Returns the value of the statistic of type <code>double</code> at
   * the given name.
   * @param name statistic name
   *
   * @throws IllegalArgumentException
   *         If no statistic exists with name <code>name</code> or
   *         if the statistic named <code>name</code> is not of
   *         type <code>double</code>.
   */
  virtual double getDouble(const std::string& name) const = 0;

  /**
   * Returns the bits that represent the raw value of the described statistic.
   *
   * @param descriptor a statistic descriptor obtained with {@link
   * #nameToDescriptor}
   * or {@link StatisticsType#nameToDescriptor}.
   * @throws IllegalArgumentException
   *         If the described statistic does not exist
   */
  virtual int64_t getRawBits(
      const std::shared_ptr<StatisticDescriptor> descriptor) const = 0;

  ////////////////////////  inc() Methods  ////////////////////////

  /**
   * Increments the value of the identified statistic of type <code>int</code>
   * by the given amount.
   *
   * @param id a statistic id obtained with {@link #nameToId}
   * or {@link StatisticsType#nameToId}.
   * @param delta change value to be added
   *
   * @return The value of the statistic after it has been incremented
   *
   * @throws IllegalArgumentException
   *         If the id is invalid.
   */
  virtual int32_t incInt(int32_t id, int32_t delta) = 0;

  /**
   * Increments the value of the described statistic of type <code>int</code>
   * by the given amount.
   * @param descriptor a statistic descriptor obtained with {@link
   * #nameToDescriptor}
   * or {@link StatisticsType#nameToDescriptor}.
   * @param delta change value to be added
   * @return The value of the statistic after it has been incremented
   *
   * @throws IllegalArgumentException
   *         If no statistic exists with the given <code>descriptor</code> or
   *         if the described statistic is not of
   *         type <code>int</code>.
   */
  virtual int32_t incInt(const std::shared_ptr<StatisticDescriptor> descriptor,
                         int32_t delta) = 0;

  /**
   * Increments the value of the statistic of type <code>int</code> with
   * the given name by a given amount.
   * @param name statistic name
   * @param delta change value to be added
   * @return The value of the statistic after it has been incremented
   *
   * @throws IllegalArgumentException
   *         If no statistic exists with name <code>name</code> or
   *         if the statistic named <code>name</code> is not of
   *         type <code>int</code>.
   */
  virtual int32_t incInt(const std::string& name, int32_t delta) = 0;

  /**
   * Increments the value of the identified statistic of type <code>long</code>
   * by the given amount.
   *
   * @param id a statistic id obtained with {@link #nameToId}
   * or {@link StatisticsType#nameToId}.
   * @param delta change value to be added
   *
   * @return The value of the statistic after it has been incremented
   *
   * @throws IllegalArgumentException
   *         If the id is invalid.
   */
  virtual int64_t incLong(int32_t id, int64_t delta) = 0;

  /**
   * Increments the value of the described statistic of type <code>long</code>
   * by the given amount.
   *
   * @param descriptor a statistic descriptor obtained with {@link
   * #nameToDescriptor}
   * or {@link StatisticsType#nameToDescriptor}.
   * @param delta change value to be added
   * @return The value of the statistic after it has been incremented
   *
   * @throws IllegalArgumentException
   *         If no statistic exists with the given <code>descriptor</code> or
   *         if the described statistic is not of
   *         type <code>long</code>.
   */
  virtual int64_t incLong(const std::shared_ptr<StatisticDescriptor> descriptor,
                          int64_t delta) = 0;
  /**
   * Increments the value of the statistic of type <code>long</code> with
   * the given name by a given amount.
   *
   * @param name statistic name
   * @param delta change value to be added
   * @return The value of the statistic after it has been incremented
   *
   * @throws IllegalArgumentException
   *         If no statistic exists with name <code>name</code> or
   *         if the statistic named <code>name</code> is not of
   *         type <code>long</code>.
   */
  virtual int64_t incLong(const std::string& name, int64_t delta) = 0;

  /**
   * Increments the value of the identified statistic of type
   * <code>double</code>
   * by the given amount.
   *
   * @param id a statistic id obtained with {@link #nameToId}
   * or {@link StatisticsType#nameToId}.
   * @param delta change value to be added
   *
   * @return The value of the statistic after it has been incremented
   *
   * @throws IllegalArgumentException
   *         If the id is invalid.
   */
  virtual double incDouble(int32_t id, double delta) = 0;

  /**
   * Increments the value of the described statistic of type <code>double</code>
   * by the given amount.
   * @param descriptor a statistic descriptor obtained with {@link
   * #nameToDescriptor}
   * or {@link StatisticsType#nameToDescriptor}.
   * @param delta change value to be added
   *
   * @return The value of the statistic after it has been incremented
   *
   * @throws IllegalArgumentException
   *         If no statistic exists with the given <code>descriptor</code> or
   *         if the described statistic is not of
   *         type <code>double</code>.
   */
  virtual double incDouble(
      const std::shared_ptr<StatisticDescriptor> descriptor, double delta) = 0;
  /**
   * Increments the value of the statistic of type <code>double</code> with
   * the given name by a given amount.
   * @param name statistic name
   * @param delta change value to be added
   *
   * @return The value of the statistic after it has been incremented
   *
   * @throws IllegalArgumentException
   *         If no statistic exists with name <code>name</code> or
   *         if the statistic named <code>name</code> is not of
   *         type <code>double</code>.
   */
  virtual double incDouble(const std::string& name, double delta) = 0;

 protected:
  virtual ~Statistics() = default;
};  // class

}  // namespace statistics
}  // namespace geode
}  // namespace apache

#endif  // GEODE_STATISTICS_STATISTICS_H_
