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

#ifndef GEODE_CACHEABLESTRING_H_
#define GEODE_CACHEABLESTRING_H_

#include "internal/geode_globals.hpp"
#include "CacheableKey.hpp"
#include "GeodeTypeIds.hpp"

/** @file
 */

namespace apache {
namespace geode {
namespace client {

/**
 * Implement a immutable C string wrapper that can serve as a distributable
 * key object for caching as well as being a string value.
 */
class _GEODE_EXPORT CacheableString : public CacheableKey {
 protected:
  std::string m_str;
  int8_t m_type;
  mutable int m_hashcode;

  _GEODE_FRIEND_STD_SHARED_PTR(CacheableString)

 public:
  /**
   *@brief serialize this object
   **/
  virtual void toData(DataOutput& output) const override;

  /**
   *@brief deserialize this object
   * Throw IllegalArgumentException if the packed CacheableString is not less
   * than 64K bytes.
   **/
  virtual void fromData(DataInput& input) override;

  /** creation function for strings */
  static Serializable* createDeserializable();

  /** creation function for strings > 64K length */
  static Serializable* createDeserializableHuge();

  /** creation function for wide strings */
  static Serializable* createUTFDeserializable();

  /** creation function for wide strings > 64K length in UTF8 encoding */
  static Serializable* createUTFDeserializableHuge();

  /**
   *@brief Return the classId of the instance being serialized.
   * This is used by deserialization to determine what instance
   * type to create and deserialize into.
   */
  virtual int32_t classId() const override;

  /**
   * Return the typeId byte of the instance being serialized.
   * This is used by deserialization to determine what instance
   * type to create and deserialize into.
   *
   * For a <code>CacheableString</code> this shall return
   * <code>GeodeTypeIds::CacheableNullString</code> if the underlying
   * string is null, <code>GeodeTypeIds::CacheableASCIIString</code>
   * if the underlying string is a char*, and
   * <code>GeodeTypeIds::CacheableString</code> if it is a wchar_t*.
   * For strings larger than 64K it will return
   * <code>GeodeTypeIds::CacheableASCIIStringHuge</code> and
   * <code>GeodeTypeIds::CacheableStringHuge</code> for char* and wchar_t*
   * respectively.
   */
  virtual int8_t typeId() const override;

  /** return true if this key matches other. */
  virtual bool operator==(const CacheableKey& other) const override;

  /** return the hashcode for this key. */
  virtual int32_t hashcode() const override;

  inline static std::shared_ptr<CacheableString> create(
      const std::string& value) {
    return std::make_shared<CacheableString>(value);
  }

  inline static std::shared_ptr<CacheableString> create(std::string&& value) {
    return std::make_shared<CacheableString>(std::move(value));
  }

  static std::shared_ptr<CacheableString> create(const std::u16string& value);

  static std::shared_ptr<CacheableString> create(std::u16string&& value);

  static std::shared_ptr<CacheableString> create(const std::u32string& value);

  static std::shared_ptr<CacheableString> create(std::u32string&& value);

  inline static std::shared_ptr<CacheableString> create(
      const std::wstring& value) {
    return std::make_shared<CacheableString>(
        std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t>{}
            .to_bytes(value));
  }

  inline static std::shared_ptr<CacheableString> create(std::wstring&& value) {
    return std::make_shared<CacheableString>(
        std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t>{}
            .to_bytes(std::move(value)));
  }

  /** Return the length of the contained string. */
  inline std::string::size_type length() const { return m_str.length(); }

  inline const std::string& value() const { return m_str; }

  virtual std::string toString() const override;

  /** Destructor */
  virtual ~CacheableString();

  virtual size_t objectSize() const override;

 protected:
  /** Default constructor. */
  inline CacheableString(int8_t type = GeodeTypeIds::CacheableASCIIString)
      : m_str(), m_type(type), m_hashcode(0) {}

  inline CacheableString(const std::string& value)
      : CacheableString(std::string(value)) {}

  inline CacheableString(std::string&& value)
      : m_str(std::move(value)), m_hashcode(0) {
    bool ascii = isAscii(m_str);

    m_type = m_str.length() > std::numeric_limits<uint16_t>::max()
                 ? ascii ? GeodeTypeIds::CacheableASCIIStringHuge
                         : GeodeTypeIds::CacheableStringHuge
                 : ascii ? GeodeTypeIds::CacheableASCIIString
                         : GeodeTypeIds::CacheableString;
  }

 private:
  void operator=(const CacheableString& other) = delete;
  CacheableString(const CacheableString& other) = delete;

  static bool isAscii(const std::string& str);
};

}  // namespace client
}  // namespace geode
}  // namespace apache

namespace std {

template <>
struct hash<apache::geode::client::CacheableString>
    : hash<apache::geode::client::CacheableKey> {};

}  // namespace std

#endif  // GEODE_CACHEABLESTRING_H_
