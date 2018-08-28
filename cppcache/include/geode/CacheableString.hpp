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
#include "internal/DataSerializablePrimitive.hpp"
#include "CacheableKey.hpp"
#include "internal/DSCode.hpp"

/** @file
 */

namespace apache {
namespace geode {
namespace client {

/**
 * Implement a immutable C string wrapper that can serve as a distributable
 * key object for caching as well as being a string value.
 */
class APACHE_GEODE_EXPORT CacheableString
    : public internal::DataSerializablePrimitive,
      public CacheableKey {
 protected:
  std::string m_str;
  internal::DSCode m_type;
  mutable int m_hashcode;

 public:
  void toData(DataOutput& output) const override;

  void fromData(DataInput& input) override;

  DSCode getDsCode() const override { return m_type; }

  /** creation function for strings */
  static std::shared_ptr<Serializable> createDeserializable();

  /** creation function for strings > 64K length */
  static std::shared_ptr<Serializable> createDeserializableHuge();

  /** creation function for wide strings */
  static std::shared_ptr<Serializable> createUTFDeserializable();

  /** creation function for wide strings > 64K length in UTF8 encoding */
  static std::shared_ptr<Serializable> createUTFDeserializableHuge();

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
  ~CacheableString() override = default;

  virtual size_t objectSize() const override;

  /** Default constructor. */
  inline CacheableString(DSCode type = DSCode::CacheableASCIIString)
      : m_str(), m_type(type), m_hashcode(0) {}

  inline CacheableString(const std::string& value)
      : CacheableString(std::string(value)) {}

  inline CacheableString(std::string&& value)
      : m_str(std::move(value)), m_hashcode(0) {
    bool ascii = isAscii(m_str);

    m_type = m_str.length() > std::numeric_limits<uint16_t>::max()
                 ? ascii ? DSCode::CacheableASCIIStringHuge
                         : DSCode::CacheableStringHuge
                 : ascii ? DSCode::CacheableASCIIString
                         : DSCode::CacheableString;
  }

 private:
  void operator=(const CacheableString& other) = delete;
  CacheableString(const CacheableString& other) = delete;

  static bool isAscii(const std::string& str);
};

template <>
inline std::shared_ptr<CacheableKey> CacheableKey::create(std::string value) {
  return CacheableString::create(value);
}

template <>
inline std::shared_ptr<CacheableKey> CacheableKey::create(
    std::u16string value) {
  return CacheableString::create(value);
}

template <>
inline std::shared_ptr<CacheableKey> CacheableKey::create(
    std::u32string value) {
  return CacheableString::create(value);
}

template <>
inline std::shared_ptr<CacheableKey> CacheableKey::create(std::wstring value) {
  return CacheableString::create(value);
}

template <>
inline std::shared_ptr<Cacheable> Serializable::create(std::string value) {
  return CacheableString::create(value);
}

template <>
inline std::shared_ptr<Cacheable> Serializable::create(std::u16string value) {
  return CacheableString::create(value);
}

template <>
inline std::shared_ptr<Cacheable> Serializable::create(std::u32string value) {
  return CacheableString::create(value);
}

template <>
inline std::shared_ptr<Cacheable> Serializable::create(std::wstring value) {
  return CacheableString::create(value);
}

template <>
inline std::shared_ptr<CacheableKey> CacheableKey::create(const char* value) {
  return CacheableString::create(value);
}

template <>
inline std::shared_ptr<Cacheable> Serializable::create(const char* value) {
  return CacheableString::create(value);
}

template <>
inline std::shared_ptr<CacheableKey> CacheableKey::create(char* value) {
  return CacheableString::create(value);
}

template <>
inline std::shared_ptr<Cacheable> Serializable::create(char* value) {
  return CacheableString::create(value);
}

template <>
inline std::shared_ptr<CacheableKey> CacheableKey::create(
    const char16_t* value) {
  return CacheableString::create(value);
}

template <>
inline std::shared_ptr<Cacheable> Serializable::create(const char16_t* value) {
  return CacheableString::create(value);
}

template <>
inline std::shared_ptr<CacheableKey> CacheableKey::create(char16_t* value) {
  return CacheableString::create(value);
}

template <>
inline std::shared_ptr<Cacheable> Serializable::create(char16_t* value) {
  return CacheableString::create(value);
}

template <>
inline std::shared_ptr<CacheableKey> CacheableKey::create(
    const char32_t* value) {
  return CacheableString::create(value);
}

template <>
inline std::shared_ptr<Cacheable> Serializable::create(const char32_t* value) {
  return CacheableString::create(value);
}
template <>
inline std::shared_ptr<CacheableKey> CacheableKey::create(char32_t* value) {
  return CacheableString::create(value);
}

template <>
inline std::shared_ptr<Cacheable> Serializable::create(char32_t* value) {
  return CacheableString::create(value);
}

template <>
inline std::shared_ptr<CacheableKey> CacheableKey::create(
    const wchar_t* value) {
  return CacheableString::create(value);
}

template <>
inline std::shared_ptr<Cacheable> Serializable::create(const wchar_t* value) {
  return CacheableString::create(value);
}

template <>
inline std::shared_ptr<CacheableKey> CacheableKey::create(wchar_t* value) {
  return CacheableString::create(value);
}

template <>
inline std::shared_ptr<Cacheable> Serializable::create(wchar_t* value) {
  return CacheableString::create(value);
}

}  // namespace client
}  // namespace geode
}  // namespace apache

namespace std {

template <>
struct hash<apache::geode::client::CacheableString>
    : hash<apache::geode::client::CacheableKey> {};

}  // namespace std

#endif  // GEODE_CACHEABLESTRING_H_
