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
using namespace apache::geode::statistics;

void Statistics::close() {}

////////////////////////  accessor Methods  ///////////////////////

int32_t Statistics::nameToId(const char* name) { return 0; }

StatisticDescriptor* Statistics::nameToDescriptor(const char* name) {
  return NULL;
}

int64_t Statistics::getUniqueId() { return 0; }

StatisticsType* Statistics::getType() { return NULL; }

const char* Statistics::getTextId() { return ""; }

int64_t Statistics::getNumericId() { return 0; }

bool Statistics::isAtomic() { return 0; }

bool Statistics::isShared() { return 0; }

bool Statistics::isClosed() { return 0; }

////////////////////////  set() Methods  ///////////////////////

void Statistics::setInt(int32_t id, int32_t value) {}

void Statistics::setInt(char* name, int32_t value) {}

void Statistics::setInt(StatisticDescriptor* descriptor, int32_t value) {}

void Statistics::setLong(int32_t id, int64_t value) {}

void Statistics::setLong(StatisticDescriptor* descriptor, int64_t value) {}

void Statistics::setLong(char* name, int64_t value) {}

void Statistics::setDouble(int32_t id, double value) {}

void Statistics::setDouble(StatisticDescriptor* descriptor, double value) {}

void setDouble(char* name, double value) {}

///////////////////////  get() Methods  ///////////////////////

int32_t Statistics::getInt(int32_t id) { return 0; }

int32_t Statistics::getInt(StatisticDescriptor* descriptor) { return 0; }

int32_t Statistics::getInt(char* name) { return 0; }

int64_t Statistics::getLong(int32_t id) { return 0; }

int64_t Statistics::getLong(StatisticDescriptor* descriptor) { return 0; }

int64_t Statistics::getLong(char* name) { return 0; }

double Statistics::getDouble(int32_t id) { return 0; }

double Statistics::getDouble(StatisticDescriptor* descriptor) { return 0; }

double Statistics::getDouble(char* name) { return 0; }

// Number Statistics::get(StatisticDescriptor* descriptor){ return }

// Number Statistics::get(char* name){ return }

/**
 * Returns the bits that represent the raw value of the described statistic.
 *
 * @param descriptor a statistic descriptor obtained with {@link
 * #nameToDescriptor}
 * or {@link StatisticsType#nameToDescriptor}.
 * @throws IllegalArgumentException
 *         If the described statistic does not exist
 */
//  int64_t Statistics::getRawBits(StatisticDescriptor* descriptor){ return }

/**
 * Returns the bits that represent the raw value of the named statistic.
 *
 * @throws IllegalArgumentException
 *         If the named statistic does not exist
 //  int64_t  Statistics::getRawBits(char* name){ return }
 */

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

int32_t Statistics::incInt(StatisticDescriptor* descriptor, int32_t delta) {
  return 0;
}

int32_t Statistics::incInt(char* name, int32_t delta) { return 0; }

int64_t Statistics::incLong(int32_t id, int64_t delta) { return 0; }

int64_t Statistics::incLong(StatisticDescriptor* descriptor, int64_t delta) {
  return 0;
}

int64_t Statistics::incLong(char* name, int64_t delta) { return 0; }

double Statistics::incDouble(int32_t id, double delta) { return 0; }

double Statistics::incDouble(StatisticDescriptor* descriptor, double delta) {
  return 0;
}

double Statistics::incDouble(char* name, double delta) { return 0; }

Statistics::~Statistics() {}
