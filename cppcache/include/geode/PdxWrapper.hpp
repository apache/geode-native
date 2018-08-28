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

  const std::string& getClassName() const override;

  bool operator==(const CacheableKey& other) const override;

  int32_t hashcode() const override;

  void toData(PdxWriter& output) const override;

  void fromData(PdxReader& input) override;

  std::string toString() const override;

  ~PdxWrapper() noexcept override {}

 private:
  PdxWrapper() = delete;

  std::shared_ptr<void> m_userObject;
  std::string m_className;
};
}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_PDXWRAPPER_H_
