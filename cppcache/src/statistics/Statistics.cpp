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

#include <geode/statistics/Statistics.hpp>

namespace apache {
namespace geode {
namespace statistics {

void Statistics::close() {}

////////////////////////  accessor Methods  ///////////////////////

int32_t Statistics::nameToId(const std::string& name) const { return 0; }

StatisticDescriptor* Statistics::nameToDescriptor(
    const std::string& name) const {
  return nullptr;
}

int64_t Statistics::getUniqueId() const { return 0; }

StatisticsType* Statistics::getType() const { return nullptr; }

const std::string& Statistics::getTextId() const {
  static std::string empty;
  return empty;
}

int64_t Statistics::getNumericId() const { return 0; }

bool Statistics::isAtomic() const { return 0; }

bool Statistics::isShared() const { return 0; }

bool Statistics::isClosed() const { return 0; }

////////////////////////  set() Methods  ///////////////////////

void Statistics::setInt(int32_t id, int32_t value) {}

void Statistics::setInt(const std::string& name, int32_t value) {}

void Statistics::setInt(const StatisticDescriptor* descriptor, int32_t value) {}

void Statistics::setLong(int32_t id, int64_t value) {}

void Statistics::setLong(const StatisticDescriptor* descriptor, int64_t value) {
}

void Statistics::setLong(const std::string& name, int64_t value) {}

void Statistics::setDouble(int32_t id, double value) {}

void Statistics::setDouble(const StatisticDescriptor* descriptor,
                           double value) {}

void setDouble(const std::string& name, double value) {}

///////////////////////  get() Methods  ///////////////////////

int32_t Statistics::getInt(int32_t id) const { return 0; }

int32_t Statistics::getInt(const StatisticDescriptor* descriptor) const {
  return 0;
}

int32_t Statistics::getInt(const std::string& name) const { return 0; }

int64_t Statistics::getLong(int32_t id) const { return 0; }

int64_t Statistics::getLong(const StatisticDescriptor* descriptor) const {
  return 0;
}

int64_t Statistics::getLong(const std::string& name) const { return 0; }

double Statistics::getDouble(int32_t id) const { return 0; }

double Statistics::getDouble(const StatisticDescriptor* descriptor) const {
  return 0;
}

double Statistics::getDouble(const std::string& name) const { return 0; }

////////////////////////  inc() Methods  ////////////////////////

/**
 * Increments the value of the identified statistic of type <code>int</code>
 * by the given amount.
 *
 * @param id a statistic id obtained with {@link #nameToId}
 * or {@link StatisticsType#nameToId}.
 *
 * @return The value of the statistic after it has been incremented
 *
 * @throws IllegalArgumentException
 *         If the id is invalid.
 */
int32_t Statistics::incInt(int32_t id, int32_t delta) { return 0; }

int32_t Statistics::incInt(const StatisticDescriptor* descriptor,
                           int32_t delta) {
  return 0;
}

int32_t Statistics::incInt(const std::string& name, int32_t delta) { return 0; }

int64_t Statistics::incLong(int32_t id, int64_t delta) { return 0; }

int64_t Statistics::incLong(const StatisticDescriptor* descriptor,
                            int64_t delta) {
  return 0;
}

int64_t Statistics::incLong(const std::string& name, int64_t delta) {
  return 0;
}

double Statistics::incDouble(int32_t id, double delta) { return 0; }

double Statistics::incDouble(const StatisticDescriptor* descriptor,
                             double delta) {
  return 0;
}

double Statistics::incDouble(const std::string& name, double delta) {
  return 0;
}

}  // namespace statistics
}  // namespace geode
}  // namespace apache
