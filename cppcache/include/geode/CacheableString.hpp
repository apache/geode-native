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

#include "geode_globals.hpp"
#include "CacheableKey.hpp"
#include "GeodeTypeIds.hpp"
#include "ExceptionTypes.hpp"

/** @file
 */

namespace apache {
namespace geode {
namespace client {

#define GF_STRING (int8_t) GeodeTypeIds::CacheableASCIIString
#define GF_STRING_HUGE (int8_t) GeodeTypeIds::CacheableASCIIStringHuge
#define GF_WIDESTRING (int8_t) GeodeTypeIds::CacheableString
#define GF_WIDESTRING_HUGE (int8_t) GeodeTypeIds::CacheableStringHuge

/**
 * Implement a immutable C string wrapper that can serve as a distributable
 * key object for caching as well as being a string value.
 */
class CPPCACHE_EXPORT CacheableString : public CacheableKey {
 protected:
  void* m_str;
  int8_t m_type;
  uint32_t m_len;
  mutable int m_hashcode;

  FRIEND_STD_SHARED_PTR(CacheableString)

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

  /**
   * Factory method for creating an instance of CacheableString from
   * a null terminated C string optionally giving the length.
   *
   * This should be used only for ASCII strings.
   */
  static std::shared_ptr<CacheableString> create(const char* value,
                                                 int32_t len = 0) {
    if (nullptr == value) {
      return nullptr;
    }

    auto str = std::make_shared<CacheableString>();
    str->initString(value, len);
    return str;
  }

  /**
   * Factory method for creating an instance of CacheableString from
   * a C string of given length by taking ownership of the string without
   * making a copy. The string should have been allocated using
   * the standard C++ new operator.
   *
   * This should be used only for ASCII strings.
   *
   * CAUTION: use this only when you really know what you are doing.
   */
  static std::shared_ptr<CacheableString> createNoCopy(char* value,
                                                       int32_t len = 0) {
    if (nullptr == value) {
      return nullptr;
    }

    auto str = std::make_shared<CacheableString>();
    str->initStringNoCopy(value, len);
    return str;
  }

  /**
   * Factory method for creating an instance of CacheableString from a
   * wide-character null terminated C string optionally giving the length.
   *
   * This should be used for non-ASCII strings.
   */
  static std::shared_ptr<CacheableString> create(const wchar_t* value,
                                                 int32_t len = 0) {
    if (nullptr == value) {
      return nullptr;
    }

    auto str = std::make_shared<CacheableString>();
    str->initString(value, len);
    return str;
  }

  /**
   * Factory method for creating an instance of CacheableString from a
   * wide-character C string of given length by taking ownership of the
   * string without making a copy. The string should have been allocated
   * using the standard C++ new operator.
   *
   * This should be used for non-ASCII strings.
   *
   * CAUTION: use this only when you really know what you are doing.
   */
  static std::shared_ptr<CacheableString> createNoCopy(wchar_t* value,
                                                       int32_t len = 0) {
    if (nullptr == value) {
      return nullptr;
    }

    auto str = std::make_shared<CacheableString>();
    str->initStringNoCopy(value, len);
    return str;
  }

  /** Returns true if the underlying string is a normal C string. */
  inline bool isCString() const {
    return (m_type == GF_STRING || m_type == GF_STRING_HUGE);
  }

  /** Returns true if the underlying string is a wide-character string. */
  inline bool isWideString() const {
    return (m_type == GF_WIDESTRING || m_type == GF_WIDESTRING_HUGE);
  }

  /**
   * Return the string that backs this CacheableString as a char *. This
   * shall throw an exception if the underlying string is a wchar_t* --
   * the caller should use <code>typeId</code> to determine the actual type,
   * or <code>isWideString</code> to find whether this is a wide-character
   * string.
   *
   * @throws IllegalStateException if the underlying string is a wchar_t *
   */
  const char* asChar() const {
    if (isWideString()) {
      throw IllegalStateException(
          "CacheableString::asChar: the string is a "
          "wide character string; use asWChar() to obtain it.");
    }
    return reinterpret_cast<const char*>(m_str);
  }

  /**
   * Return the string that backs this CacheableString as a wchar_t *. This
   * shall throw an exception if the underlying string is a char* --
   * the caller should use <code>typeId</code> to determine the actual type,
   * or <code>isWideString</code> to find whether this is indeed a
   * wide-character string.
   *
   * @throws IllegalStateException if the underlying string is a char *
   */
  const wchar_t* asWChar() const {
    if (isCString()) {
      throw IllegalStateException(
          "CacheableString::asWChar: the string is "
          "not a wide character string; use asChar() to obtain it.");
    }
    return reinterpret_cast<const wchar_t*>(m_str);
  }

  /** Return the length of the contained string. */
  inline uint32_t length() const { return m_len; }

  virtual std::string toString() const override;

  /** Destructor */
  virtual ~CacheableString();

  virtual uint32_t objectSize() const override;

 protected:
  /** Private method to populate the <code>CacheableString</code>. */
  void copyString(const char* value, int32_t len);
  /** Private method to populate the <code>CacheableString</code>. */
  void copyString(const wchar_t* value, int32_t len);
  /** initialize the string, given a value and length. */
  void initString(const char* value, int32_t len);
  /**
   * Initialize the string without making a copy, given a C string
   * and length.
   */
  void initStringNoCopy(char* value, int32_t len);
  /** initialize the string, given a wide-char string and length. */
  void initString(const wchar_t* value, int32_t len);
  /**
   * Initialize the string without making a copy, given a wide-char string
   * and length.
   */
  void initStringNoCopy(wchar_t* value, int32_t len);
  /** Private method to get ASCII string for wide-string if possible. */
  char* getASCIIString(const wchar_t* value, int32_t& len, int32_t& encodedLen);
  /** Default constructor. */
  inline CacheableString(int8_t type = GF_STRING)
      : m_str(nullptr), m_type(type), m_len(0), m_hashcode(0) {}

 private:
  // never implemented.
  void operator=(const CacheableString& other) = delete;
  CacheableString(const CacheableString& other) = delete;
};

/** overload of apache::geode::client::createKeyArr to pass char* */
inline std::shared_ptr<CacheableKey> createKeyArr(const char* value) {
  return CacheableString::create(value);
}

/** overload of apache::geode::client::createKeyArr to pass wchar_t* */
inline std::shared_ptr<CacheableKey> createKeyArr(const wchar_t* value) {
  return CacheableString::create(value);
}

/** overload of apache::geode::client::createValueArr to pass char* */
inline std::shared_ptr<Cacheable> createValueArr(const char* value) {
  return CacheableString::create(value);
}

/** overload of apache::geode::client::createValueArr to pass wchar_t* */
inline std::shared_ptr<Cacheable> createValueArr(const wchar_t* value) {
  return CacheableString::create(value);
}

template <typename TVALUE>
inline std::shared_ptr<Cacheable> createValue(const TVALUE* value) {
  return CacheableString::create(value);
}

template <class TKEY>
inline std::shared_ptr<CacheableKey> createKey(
    const std::shared_ptr<TKEY>& value) {
  return std::shared_ptr<CacheableKey>(value);
}

template <typename TKEY>
inline std::shared_ptr<CacheableKey> createKey(const TKEY* value) {
  return createKeyArr(value);
}

template <class PRIM>
inline std::shared_ptr<CacheableKey> CacheableKey::create(const PRIM value) {
  return createKey(value);
}

template <class PRIM>
inline std::shared_ptr<Serializable> Serializable::create(const PRIM value) {
  return createKey(value);
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
