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

#include "StatisticDescriptorImpl.hpp"

namespace apache {
namespace geode {
namespace statistics {

using client::IllegalArgumentException;
using client::IllegalStateException;
using client::OutOfMemoryException;

const std::string StatisticDescriptorImpl::IntTypeName = "int_t";
const std::string StatisticDescriptorImpl::LongTypeName = "Long";
const std::string StatisticDescriptorImpl::DoubleTypeName = "Float";

/**
 * Describes an individual statistic whose value is updated by an
 * application and may be archived by Geode.  These descriptions are
 * gathered together in a {@link StatisticsType}.
 * <P>
 * To get an instance of this interface use an instance of
 * {@link StatisticsFactory}.
 * <P>
 * StatisticDescriptors are naturally ordered by their name.
 *
 */

StatisticDescriptorImpl::StatisticDescriptorImpl(
    const std::string& statName, FieldType statDescriptorType,
    const std::string& statDescription, const std::string& statUnit,
    bool statIsStatCounter, bool statIsStatLargerBetter)
    : name(statName),
      description(statDescription),
      unit(statUnit),
      isStatCounter(statIsStatCounter),
      isStatLargerBetter(statIsStatLargerBetter),
      id(-1),
      descriptorType(statDescriptorType) {}

StatisticDescriptorImpl::~StatisticDescriptorImpl() {}

/////////////////////// Create functions ///////////////////////////////////

std::shared_ptr<StatisticDescriptor> StatisticDescriptorImpl::createCounter(
    const std::string& statName, FieldType fieldType,
    const std::string& description, const std::string& units,
    bool statIsCounter, bool isLargerBetter) {
  auto sdi = new StatisticDescriptorImpl(statName, fieldType, description,
                                         units, statIsCounter, isLargerBetter);
  return std::shared_ptr<StatisticDescriptorImpl>(sdi);
}

std::shared_ptr<StatisticDescriptor> StatisticDescriptorImpl::createIntCounter(
    const std::string& statName, const std::string& description,
    const std::string& units, bool isLargerBetter) {
  FieldType fieldType = INT_TYPE;
  return createCounter(statName, fieldType, description, units, true,
                       isLargerBetter);
}

std::shared_ptr<StatisticDescriptor> StatisticDescriptorImpl::createLongCounter(
    const std::string& name, const std::string& description,
    const std::string& units, bool isLargerBetter) {
  FieldType fieldType = LONG_TYPE;
  return createCounter(name, fieldType, description, units, true,
                       isLargerBetter);
}

std::shared_ptr<StatisticDescriptor>
StatisticDescriptorImpl::createDoubleCounter(const std::string& name,
                                             const std::string& description,
                                             const std::string& units,
                                             bool isLargerBetter) {
  FieldType fieldType = DOUBLE_TYPE;
  return createCounter(name, fieldType, description, units, true,
                       isLargerBetter);
}

std::shared_ptr<StatisticDescriptor> StatisticDescriptorImpl::createIntGauge(
    const std::string& name, const std::string& description,
    const std::string& units, bool isLargerBetter) {
  FieldType fieldType = INT_TYPE;
  return createCounter(name, fieldType, description, units, false,
                       isLargerBetter);
}

std::shared_ptr<StatisticDescriptor> StatisticDescriptorImpl::createLongGauge(
    const std::string& name, const std::string& description,
    const std::string& units, bool isLargerBetter) {
  FieldType fieldType = LONG_TYPE;
  return createCounter(name, fieldType, description, units, false,
                       isLargerBetter);
}

std::shared_ptr<StatisticDescriptor> StatisticDescriptorImpl::createDoubleGauge(
    const std::string& name, const std::string& description,
    const std::string& units, bool isLargerBetter) {
  FieldType fieldType = DOUBLE_TYPE;
  return createCounter(name, fieldType, description, units, false,
                       isLargerBetter);
}

/////////////////////// StatisticDescriptor(Base class)
/// Methods///////////////////////////

const std::string& StatisticDescriptorImpl::getName() const { return name; }

const std::string& StatisticDescriptorImpl::getDescription() const {
  return description;
}

int32_t StatisticDescriptorImpl::getStorageBits() {
  return getTypeCodeBits(descriptorType);
}

const std::string& StatisticDescriptorImpl::getTypeCodeName(FieldType code) {
  switch (code) {
    case INT_TYPE:
      return IntTypeName;
    case LONG_TYPE:
      return LongTypeName;
    case DOUBLE_TYPE:
      return DoubleTypeName;
  }
  std::string s = "Unknown type code:" + std::to_string(code);
  throw IllegalArgumentException(s.c_str());
}

/**
 * Returns the number of bits needed to represent a value of the given type
 * @throws IllegalArgumentException/
 *         <code>code</code> is an unknown type
 */
int32_t StatisticDescriptorImpl::getTypeCodeBits(FieldType code) {
  switch (code) {
    case INT_TYPE:
      return 32;
    case LONG_TYPE:
      return 64;
    case DOUBLE_TYPE:
      return 64;
  }
  std::string temp(getTypeCodeName(code));
  std::string s = "Unknown type code: " + temp;
  throw IllegalArgumentException(s.c_str());
}

bool StatisticDescriptorImpl::isCounter() const { return isStatCounter; }

bool StatisticDescriptorImpl::isLargerBetter() const {
  return isStatLargerBetter;
}

const std::string& StatisticDescriptorImpl::getUnit() const { return unit; }

int32_t StatisticDescriptorImpl::getId() const {
  if (id == -1) {
    std::string s = "The id has not been initialized yet.";
    throw IllegalStateException(s.c_str());
  }
  return id;
}

///////////////////////// Instance Methods///////////////// ////////////////////

FieldType StatisticDescriptorImpl::getTypeCode() const {
  return descriptorType;
}

void StatisticDescriptorImpl::setId(int32_t statId) { id = statId; }

int32_t StatisticDescriptorImpl::checkInt() const {
  if (descriptorType != INT_TYPE) {
    std::string sb;
    std::string typeCode(getTypeCodeName(getTypeCode()));
    sb = "The statistic " + name;
    sb += " is of type " + typeCode;
    sb += " and it was expected to be an int";
    throw IllegalArgumentException(sb.c_str());
  }
  return id;
}
int32_t StatisticDescriptorImpl::checkLong() const {
  if (descriptorType != LONG_TYPE) {
    std::string sb;
    std::string typeCode(getTypeCodeName(getTypeCode()));
    sb = "The statistic " + name;
    sb += " is of type " + typeCode;
    sb += " and it was expected to be an long";
    throw IllegalArgumentException(sb.c_str());
  }
  return id;
}

int32_t StatisticDescriptorImpl::checkDouble() const {
  if (descriptorType != DOUBLE_TYPE) {
    std::string sb;
    std::string typeCode(getTypeCodeName(getTypeCode()));

    sb = "The statistic " + name;
    sb += " is of type " + typeCode;
    sb += " and it was expected to be an long";
    throw IllegalArgumentException(sb.c_str());
  }
  return id;
}

}  // namespace statistics
}  // namespace geode
}  // namespace apache
