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

#include "OsStatisticsImpl.hpp"
#include "StatisticsTypeImpl.hpp"
#include "StatisticDescriptorImpl.hpp"

#include "ace/OS.h"

using namespace apache::geode::statistics;

/**
 * An implementation of {@link Statistics} that stores its statistics
 * in local  memory and does not support atomic operations.
 *
 */

//////////////////////  Static Methods  //////////////////////

int64_t OsStatisticsImpl::calcNumericId(StatisticsFactory* system,
                                      int64_t userValue) {
  int64_t result;
  if (userValue != 0) {
    result = userValue;
  } else {
    result = system->getId();
  }
  return result;
}

const char* OsStatisticsImpl::calcTextId(StatisticsFactory* system,
                                         const char* userValue) {
  if (userValue != NULL && strcmp(userValue, "") != 0) {
    return userValue;
  } else {
    if (system != NULL) {
      return system->getName();
    } else {
      return "";
    }
  }
}

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
OsStatisticsImpl::OsStatisticsImpl(StatisticsType* typeArg,
                                   const char* textIdArg, int64_t numericIdArg,
                                   int64_t uniqueIdArg, StatisticsFactory* system)

{
  this->textId = calcTextId(system, textIdArg);
  this->numericId = calcNumericId(system, numericIdArg);
  this->uniqueId = uniqueIdArg;
  this->closed = false;
  statsType = dynamic_cast<StatisticsTypeImpl*>(typeArg);
  /* adongre
   * CID 28981: Uninitialized pointer field (UNINIT_CTOR)
   */
  doubleStorage = (double*)0;
  intStorage = (int32_t*)0;
  longStorage = (int64_t*)0;

  if (statsType != NULL) {
    int32_t intCount = statsType->getIntStatCount();
    int32_t longCount = statsType->getLongStatCount();
    int32_t doubleCount = statsType->getDoubleStatCount();
    if (intCount > 0) {
      intStorage = new int32_t[intCount];
      for (int32_t i = 0; i < intCount; i++) {
        intStorage[i] = 0;  // Un-initialized state
      }
    } else {
      intStorage = NULL;
    }
    if (longCount > 0) {
      longStorage = new int64_t[longCount];
      for (int32_t i = 0; i < longCount; i++) {
        longStorage[i] = 0;  // Un-initialized state
      }
    } else {
      longStorage = NULL;
    }
    if (doubleCount > 0) {
      doubleStorage = new double[doubleCount];
      for (int32_t i = 0; i < doubleCount; i++) {
        doubleStorage[i] = 0;  // Un-initialized state
      }
    } else {
      doubleStorage = NULL;
    }
  }  // if(statsType == NULL)
}

OsStatisticsImpl::~OsStatisticsImpl() {
  try {
    statsType = NULL;
    if (intStorage != NULL) {
      delete[] intStorage;
      intStorage = NULL;
    }
    if (longStorage != NULL) {
      delete[] longStorage;
      longStorage = NULL;
    }
    if (doubleStorage != NULL) {
      delete[] doubleStorage;
      doubleStorage = NULL;
    }
  } catch (...) {
    LOGERROR("Exception in ~OsStatisticsImpl");
  }
}

//////////////////////  Instance Methods  //////////////////////

bool OsStatisticsImpl::isShared() { return false; }

bool OsStatisticsImpl::isAtomic() {
  return false;  // will always be false for this class
}

void OsStatisticsImpl::close() {
  // Just mark closed,Will be actually deleted when token written in archive
  // file.
  this->closed = true;
}

////////////////////////  store() Methods  ///////////////////////

void OsStatisticsImpl::_setInt(int32_t offset, int32_t value) {
  if (offset >= statsType->getIntStatCount()) {
    char s[128] = {'\0'};
    ACE_OS::snprintf(
        s, 128, "setInt:The id (%d) of the Statistic Descriptor is not valid ",
        offset);
    throw IllegalArgumentException(s);
  }
  intStorage[offset] = value;
}

void OsStatisticsImpl::_setLong(int32_t offset, int64_t value) {
  if (offset >= statsType->getLongStatCount()) {
    char s[128] = {'\0'};
    /* adongre  - Coverity II
     * CID 29275: Calling risky function (SECURE_CODING)[VERY RISKY]. Using
     * "sprintf" can cause a
     * buffer overflow when done incorrectly. Because sprintf() assumes an
     * arbitrarily long string,
     * callers must be careful not to overflow the actual space of the
     * destination.
     * Use snprintf() instead, or correct precision specifiers.
     * Fix : using ACE_OS::snprintf
     */
    // sprintf(s, "setLong:The id (%d) of the Statistic Descriptor is not valid
    // ", offset);
    ACE_OS::snprintf(
        s, 128, "setLong:The id (%d) of the Statistic Descriptor is not valid ",
        offset);
    throw IllegalArgumentException(s);
  }
  longStorage[offset] = value;
}

void OsStatisticsImpl::_setDouble(int32_t offset, double value) {
  if (offset >= statsType->getDoubleStatCount()) {
    char s[128] = {'\0'};
    ACE_OS::snprintf(
        s, 128,
        "setDouble:The id (%d) of the Statistic Descriptor is not valid ",
        offset);
    throw IllegalArgumentException(s);
  }
  doubleStorage[offset] = value;
}

///////////////////////  get() Methods  ///////////////////////

int32_t OsStatisticsImpl::_getInt(int32_t offset) {
  if (offset >= statsType->getIntStatCount()) {
    char s[128] = {'\0'};
    ACE_OS::snprintf(
        s, 128, "getInt:The id (%d) of the Statistic Descriptor is not valid ",
        offset);
    throw IllegalArgumentException(s);
  }

  return intStorage[offset];
}

int64_t OsStatisticsImpl::_getLong(int32_t offset) {
  if (offset >= statsType->getLongStatCount()) {
    char s[128] = {'\0'};
    ACE_OS::snprintf(
        s, 128, "getLong:The id (%d) of the Statistic Descriptor is not valid ",
        offset);
    throw IllegalArgumentException(s);
  }

  return longStorage[offset];
}

double OsStatisticsImpl::_getDouble(int32_t offset) {
  if (offset >= statsType->getDoubleStatCount()) {
    char s[128] = {'\0'};
    ACE_OS::snprintf(
        s, 128,
        "getDouble:The id (%d) of the Statistic Descriptor is not valid ",
        offset);
    throw IllegalArgumentException(s);
  }

  return doubleStorage[offset];
}

int64_t OsStatisticsImpl::_getRawBits(StatisticDescriptor* statDscp) {
  StatisticDescriptorImpl* stat =
      dynamic_cast<StatisticDescriptorImpl*>(statDscp);
  // dynamic cast is giving problems , so a normal cast was used
  // StatisticDescriptorImpl* stat  = (StatisticDescriptorImpl*)statDscp;
  switch (stat->getTypeCode()) {
    case INT_TYPE:
      return _getInt(stat->getId());
    case LONG_TYPE:
      return _getLong(stat->getId());

    case DOUBLE_TYPE: {
      double value = _getDouble(stat->getId());
      int64_t* temp = reinterpret_cast<int64_t*>(&value);
      return *temp;
    }

    default:
      return 0;
      /*throw RuntimeException("unexpected stat descriptor type code: " +
                                stat->getTypeCode());*/
  }
}
////////////////////////  inc() Methods  ////////////////////////

int32_t OsStatisticsImpl::_incInt(int32_t offset, int32_t delta) {
  if (offset >= statsType->getIntStatCount()) {
    char s[128] = {'\0'};
    ACE_OS::snprintf(
        s, 128, "incInt:The id (%d) of the Statistic Descriptor is not valid ",
        offset);
    throw IllegalArgumentException(s);
  }

  intStorage[offset] += delta;
  return intStorage[offset];
}

int64_t OsStatisticsImpl::_incLong(int32_t offset, int64_t delta) {
  if (offset >= statsType->getLongStatCount()) {
    char s[128] = {'\0'};
    /* adongre  - Coverity II
     * CID 29274: Calling risky function (SECURE_CODING)[VERY RISKY]. Using
     * "sprintf" can cause a
     * buffer overflow when done incorrectly. Because sprintf() assumes an
     * arbitrarily long string,
     * callers must be careful not to overflow the actual space of the
     * destination.
     * Use snprintf() instead, or correct precision specifiers.
     * Fix : using ACE_OS::snprintf
     */
    // sprintf(s, "incLong:The id (%d) of the Statistic Descriptor is not valid
    // ", offset);

    ACE_OS::snprintf(
        s, 128, "incLong:The id (%d) of the Statistic Descriptor is not valid ",
        offset);
    throw IllegalArgumentException(s);
  }
  longStorage[offset] += delta;
  return longStorage[offset];
}

double OsStatisticsImpl::_incDouble(int32_t offset, double delta) {
  if (offset >= statsType->getDoubleStatCount()) {
    char s[128] = {'\0'};
    ACE_OS::snprintf(
        s, 128,
        "incDouble:The id (%d) of the Statistic Descriptor is not valid ",
        offset);
    throw IllegalArgumentException(s);
  }
  doubleStorage[offset] += delta;
  return doubleStorage[offset];
}

//////////////////////  Instance Methods  //////////////////////

int32_t OsStatisticsImpl::nameToId(const char* name) {
  return statsType->nameToId(name);
}

StatisticDescriptor* OsStatisticsImpl::nameToDescriptor(const char* name) {
  return statsType->nameToDescriptor(name);
}

bool OsStatisticsImpl::isClosed() { return closed; }

bool OsStatisticsImpl::isOpen() { return !closed; }

////////////////////////  attribute Methods  ///////////////////////

StatisticsType* OsStatisticsImpl::getType() { return statsType; }

const char* OsStatisticsImpl::getTextId() { return textId; }

int64_t OsStatisticsImpl::getNumericId() { return numericId; }

/**
 * Gets the unique id for this resource
 */
int64_t OsStatisticsImpl::getUniqueId() { return uniqueId; }
////////////////////////  set() Methods  ///////////////////////

void OsStatisticsImpl::setInt(char* name, int32_t value) {
  setInt(nameToDescriptor(name), value);
}

void OsStatisticsImpl::setInt(StatisticDescriptor* descriptor, int32_t value) {
  setInt(getIntId(descriptor), value);
}

void OsStatisticsImpl::setInt(int32_t id, int32_t value) {
  if (isOpen()) {
    _setInt(id, value);
  }
}
////////////////////////////////LONG METHODS/////////////////////////////

void OsStatisticsImpl::setLong(char* name, int64_t value) {
  setLong(nameToDescriptor(name), value);
}

void OsStatisticsImpl::setLong(StatisticDescriptor* descriptor, int64_t value) {
  setLong(getLongId(descriptor), value);
}

void OsStatisticsImpl::setLong(int32_t id, int64_t value) {
  if (isOpen()) {
    _setLong(id, value);
  }
}
////////////////////////////////////////DOUBLE METHODS////////////////////

void OsStatisticsImpl::setDouble(char* name, double value) {
  setDouble(nameToDescriptor(name), value);
}

void OsStatisticsImpl::setDouble(StatisticDescriptor* descriptor,
                                 double value) {
  setDouble(getDoubleId(descriptor), value);
}

void OsStatisticsImpl::setDouble(int32_t id, double value) {
  if (isOpen()) {
    _setDouble(id, value);
  }
}
//////////////////////////Get INT Methods/////////////////////////////////////
int32_t OsStatisticsImpl::getInt(char* name) {
  return getInt(nameToDescriptor(name));
}

int32_t OsStatisticsImpl::getInt(StatisticDescriptor* descriptor) {
  return getInt(getIntId(descriptor));
}

int32_t OsStatisticsImpl::getInt(int32_t id) {
  if (isOpen()) {
    return _getInt(id);
  } else {
    LOGWARN("os statistics is closed");
    return 0;
  }
}

/////////////////////////////////////////Get Long
/// Methods///////////////////////////////

int64_t OsStatisticsImpl::getLong(char* name) {
  return getLong(nameToDescriptor(name));
}

int64_t OsStatisticsImpl::getLong(StatisticDescriptor* descriptor) {
  return getLong(getLongId(descriptor));
}

int64_t OsStatisticsImpl::getLong(int32_t id) {
  if (isOpen()) {
    return _getLong(id);
  } else {
    return 0;
  }
}
/////////////////////////////////Get DOUBLE Methods
/////////////////////////////////
double OsStatisticsImpl::getDouble(char* name) {
  return getDouble(nameToDescriptor(name));
}

double OsStatisticsImpl::getDouble(StatisticDescriptor* descriptor) {
  return getDouble(getDoubleId(descriptor));
}

double OsStatisticsImpl::getDouble(int32_t id) {
  if (isOpen()) {
    return _getDouble(id);
  } else {
    return 0;
  }
}
//////////////////////////////Get RAW BIT
/// methods////////////////////////////////

int64_t OsStatisticsImpl::getRawBits(StatisticDescriptor* descriptor) {
  if (isOpen()) {
    return _getRawBits(descriptor);
  } else {
    return 0;
  }
}

int64_t OsStatisticsImpl::getRawBits(char* name) {
  return getRawBits(nameToDescriptor(name));
}

///////////////////////// INC INT //////////////////////////////////////////////
int32_t OsStatisticsImpl::incInt(char* name, int32_t delta) {
  return incInt(nameToDescriptor(name), delta);
}

int32_t OsStatisticsImpl::incInt(StatisticDescriptor* descriptor, int32_t delta) {
  return incInt(getIntId(descriptor), delta);
}

int32_t OsStatisticsImpl::incInt(int32_t id, int32_t delta) {
  if (isOpen()) {
    return _incInt(id, delta);
  } else {
    return 0;
  }
}

//// //////////////// INC LONG ///////////////////////////////////

int64_t OsStatisticsImpl::incLong(char* name, int64_t delta) {
  return incLong(nameToDescriptor(name), delta);
}

int64_t OsStatisticsImpl::incLong(StatisticDescriptor* descriptor, int64_t delta) {
  return incLong(getLongId(descriptor), delta);
}

int64_t OsStatisticsImpl::incLong(int32_t id, int64_t delta) {
  if (isOpen()) {
    return _incLong(id, delta);
  } else {
    return 0;
  }
}
////////////////////////////  INC DOUBLE //////////////////////////////////////

double OsStatisticsImpl::incDouble(char* name, double delta) {
  return incDouble(nameToDescriptor(name), delta);
}

double OsStatisticsImpl::incDouble(StatisticDescriptor* descriptor,
                                   double delta) {
  return incDouble(getDoubleId(descriptor), delta);
}

double OsStatisticsImpl::incDouble(int32_t id, double delta) {
  if (isOpen()) {
    return _incDouble(id, delta);
  } else {
    return 0;
  }
}
/////////////////////////// GET ID /////////////////////////////////////////

int32_t OsStatisticsImpl::getIntId(StatisticDescriptor* descriptor) {
  StatisticDescriptorImpl* realDescriptor =
      dynamic_cast<StatisticDescriptorImpl*>(descriptor);
  return realDescriptor->checkInt();
}

int32_t OsStatisticsImpl::getLongId(StatisticDescriptor* descriptor) {
  StatisticDescriptorImpl* realDescriptor =
      dynamic_cast<StatisticDescriptorImpl*>(descriptor);
  return realDescriptor->checkLong();
}

int32_t OsStatisticsImpl::getDoubleId(StatisticDescriptor* descriptor) {
  StatisticDescriptorImpl* realDescriptor =
      dynamic_cast<StatisticDescriptorImpl*>(descriptor);
  return realDescriptor->checkDouble();
}
