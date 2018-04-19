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

#ifndef GEODE_PDXWRAPPER_H_
#define GEODE_PDXWRAPPER_H_

#include <iosfwd>
#include <memory>
#include <string>

#include "PdxSerializable.hpp"
#include "PdxSerializer.hpp"
#include "internal/geode_base.hpp"

namespace apache {
namespace geode {
namespace client {

class CacheableKey;
class DataInput;
class DataOutput;
class PdxReader;
class PdxWriter;

/**
 * The PdxWrapper class allows domain classes to be used in Region operations.
 * A user domain object should be wrapped in an instance of a PdxWrapper with
 * a PdxSerializer registered that can handle the user domain class.
 */
class APACHE_GEODE_EXPORT PdxWrapper : public PdxSerializable {
 public:
  /**
   * Constructor which takes the address of the user object to contain for PDX
   * serialization.
   * @param userObject the void pointer to an instance of a user object - NOTE:
   * PdxWrapper takes ownership.
   * @param className the fully qualified class name to map this user object to
   * the Java side.
   */
  PdxWrapper(std::shared_ptr<void> userObject, std::string className);

  /**
   * Returns the pointer to the user object which is deserialized with a
   * PdxSerializer.
   * User code (such as in PdxSerializer) should cast it to a pointer of the
   * known user class.
   * @param detach if set to true will release ownership of the object and
   * future calls to getObject() return nullptr.
   */
  std::shared_ptr<void> getObject();

  /**
   * Get the class name for the user domain object.
   */
  const std::string& getClassName() const override;

  /** return true if this key matches other. */
  bool operator==(const CacheableKey& other) const override;

  /** return the hashcode for this key. */
  int32_t hashcode() const override;

  /**
   *@brief serialize this object in geode PDX format
   *@param PdxWriter to serialize the PDX object
   **/
  void toData(PdxWriter& output) const override;

  /**
   *@brief Deserialize this object
   *@param PdxReader to Deserialize the PDX object
   **/
  void fromData(PdxReader& input) override;

  /**
   *@brief serialize this object
   **/
  void toData(DataOutput& output) const override;

  /**
   *@brief deserialize this object, typical implementation should return
   * the 'this' pointer.
   **/
  void fromData(DataInput& input) override;

  /**
   *@brief return the classId of the instance being serialized.
   * This is used by deserialization to determine what instance
   * type to create and derserialize into.
   */
  int32_t classId() const override { return 0; }

  /**
   *@brief return the size in bytes of the instance being serialized.
   * This is used to determine whether the cache is using up more
   * physical memory than it has been configured to use. The method can
   * return zero if the user does not require the ability to control
   * cache memory utilization.
   * Note that you must implement this only if you use the HeapLRU feature.
   */
  size_t objectSize() const override;

  /**
   * Display this object as 'string', which depends on the implementation in
   * the subclasses.
   * The default implementation renders the classname.
   */
  std::string toString() const override;

  ~PdxWrapper() noexcept override {}

 private:
  PdxWrapper() = delete;
  PdxWrapper(std::string className);

  _GEODE_FRIEND_STD_SHARED_PTR(PdxWrapper)

  std::shared_ptr<void> m_userObject;
  std::string m_className;
  UserObjectSizer m_sizer;
};
}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_PDXWRAPPER_H_
