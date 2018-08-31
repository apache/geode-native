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

#ifndef GEODE_CACHEABLEDATE_H_
#define GEODE_CACHEABLEDATE_H_

#include "internal/geode_globals.hpp"
#include "CacheableKey.hpp"
#include "CacheableString.hpp"
#include "ExceptionTypes.hpp"

#include <string>
#include <chrono>
#include <ctime>

/** @file
 */
namespace apache {
namespace geode {
namespace client {

/**
 * Implement a date object based on epoch of January 1, 1970 00:00:00 GMT that
 * can serve as a distributable key object for caching as well as being a date
 * value.
 */
class APACHE_GEODE_EXPORT CacheableDate : public DataSerializablePrimitive,
                                          public CacheableKey {
 private:
  /**
   * Milliseconds since January 1, 1970, 00:00:00 GMT to be consistent with Java
   * Date.
   */
  int64_t m_timevalue;

 public:
  typedef std::chrono::system_clock clock;
  typedef std::chrono::time_point<clock> time_point;
  typedef std::chrono::milliseconds duration;

  void toData(DataOutput& output) const override;

  virtual void fromData(DataInput& input) override;

  /**
   * @brief creation function for dates.
   */
  static std::shared_ptr<Serializable> createDeserializable();

  virtual DSCode getDsCode() const override;

  /** @return the size of the object in bytes */
  virtual size_t objectSize() const override { return sizeof(CacheableDate); }

  /** @return true if this key matches other. */
  virtual bool operator==(const CacheableKey& other) const override;

  /** @return milliseconds elapsed since January 1, 1970, 00:00:00 GMT. */
  virtual int64_t milliseconds() const;

  /**
   * Returns a hash code value for this object. The result is the exclusive OR
   * of the two halves of the primitive long value returned by the
   * milliseconds() method.
   *
   * @return the hashcode for this object. */
  virtual int32_t hashcode() const override;

  operator time_t() const { return m_timevalue / 1000; }
  operator time_point() const {
    return clock::from_time_t(0) + duration(m_timevalue);
  }
  operator duration() const { return duration(m_timevalue); }

  /**
   * Factory method for creating an instance of CacheableDate
   */
  static std::shared_ptr<CacheableDate> create() {
    return std::make_shared<CacheableDate>();
  }

  static std::shared_ptr<CacheableDate> create(const time_t& value) {
    return std::make_shared<CacheableDate>(value);
  }

  static std::shared_ptr<CacheableDate> create(const time_point& value) {
    return std::make_shared<CacheableDate>(value);
  }

  static std::shared_ptr<CacheableDate> create(const duration& value) {
    return std::make_shared<CacheableDate>(value);
  }

  std::string toString() const override;

  /** Destructor */
  ~CacheableDate() override = default;

  /** Constructor, used for deserialization. */
  CacheableDate(const time_t value = 0);

  /**
   * Construct from std::chrono::time_point<std::chrono::system_clock>.
   */
  CacheableDate(const time_point& value);

  /**
   * Construct from std::chrono::seconds since POSIX epoch.
   */
  CacheableDate(const duration& value);

 private:
  // never implemented.
  void operator=(const CacheableDate& other);
  CacheableDate(const CacheableDate& other);
};

template <>
inline std::shared_ptr<CacheableKey> CacheableKey::create(
    CacheableDate::time_point value) {
  return CacheableDate::create(value);
}

template <>
inline std::shared_ptr<Serializable> Serializable::create(
    CacheableDate::time_point value) {
  return CacheableDate::create(value);
}

}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_CACHEABLEDATE_H_
