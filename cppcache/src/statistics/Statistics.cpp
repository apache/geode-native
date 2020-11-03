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

#include "Statistics.hpp"

namespace apache {
namespace geode {
namespace statistics {

void Statistics::close() {}

////////////////////////  accessor Methods  ///////////////////////

int32_t Statistics::nameToId(const std::string&) const { return 0; }

std::shared_ptr<StatisticDescriptor> Statistics::nameToDescriptor(
    const std::string&) const {
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

void Statistics::setInt(int32_t, int32_t) {}

void Statistics::setInt(const std::string&, int32_t) {}

void Statistics::setInt(const std::shared_ptr<StatisticDescriptor>, int32_t) {}

void Statistics::setLong(int32_t, int64_t) {}

void Statistics::setLong(const std::shared_ptr<StatisticDescriptor>, int64_t) {}

void Statistics::setLong(const std::string&, int64_t) {}

void Statistics::setDouble(int32_t, double) {}

void Statistics::setDouble(const std::shared_ptr<StatisticDescriptor>, double) {
}

void setDouble(const std::string&, double) {}

///////////////////////  get() Methods  ///////////////////////

int32_t Statistics::getInt(int32_t) const { return 0; }

int32_t Statistics::getInt(const std::shared_ptr<StatisticDescriptor>) const {
  return 0;
}

int32_t Statistics::getInt(const std::string&) const { return 0; }

int64_t Statistics::getLong(int32_t) const { return 0; }

int64_t Statistics::getLong(const std::shared_ptr<StatisticDescriptor>) const {
  return 0;
}

int64_t Statistics::getLong(const std::string&) const { return 0; }

double Statistics::getDouble(int32_t) const { return 0; }

double Statistics::getDouble(const std::shared_ptr<StatisticDescriptor>) const {
  return 0;
}

double Statistics::getDouble(const std::string&) const { return 0; }

////////////////////////  inc() Methods  ////////////////////////

/**
 * Increments the value of the identified statistic of type <code>int</code>
 * by the given amount.
 *
 * @return The value of the statistic after it has been incremented
 *
 * @throws IllegalArgumentException
 *         If the id is invalid.
 */
int32_t Statistics::incInt(int32_t, int32_t) { return 0; }

int32_t Statistics::incInt(const std::shared_ptr<StatisticDescriptor>,
                           int32_t) {
  return 0;
}

int32_t Statistics::incInt(const std::string&, int32_t) { return 0; }

int64_t Statistics::incLong(int32_t, int64_t) { return 0; }

int64_t Statistics::incLong(const std::shared_ptr<StatisticDescriptor>,
                            int64_t) {
  return 0;
}

int64_t Statistics::incLong(const std::string&, int64_t) { return 0; }

double Statistics::incDouble(int32_t, double) { return 0; }

double Statistics::incDouble(const std::shared_ptr<StatisticDescriptor>,
                             double) {
  return 0;
}

double Statistics::incDouble(const std::string&, double) { return 0; }

}  // namespace statistics
}  // namespace geode
}  // namespace apache
