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

#ifndef GEODE_PDXSERIALIZER_H_
#define GEODE_PDXSERIALIZER_H_

#include "PdxReader.hpp"
#include "PdxWriter.hpp"
#include "Serializable.hpp"

namespace apache {
namespace geode {
namespace client {

/**
 * Function pointer type that takes a void pointer to an instance of a user
 * object and a class name to return the size of the user object.
 */
using UserObjectSizer = std::function<size_t(const std::shared_ptr<const void>&,
                                             const std::string&)>;

/**
 * The PdxSerializer class allows domain classes to be
 * serialized and deserialized as PDXs without modification
 * of the domain class.
 * A domain class should register function {@link
 * Serializable::registerPdxSerializer} to create a new
 * instance of type for de-serialization.
 */
class APACHE_GEODE_EXPORT PdxSerializer {
 public:
  PdxSerializer() {}

  virtual ~PdxSerializer() {}

  /**
   * Deserialize this object.
   *
   * @param className the class name whose object needs to be de-serialized
   * @param pdxReader the PdxReader stream to use for reading the object data
   */
  virtual std::shared_ptr<void> fromData(const std::string& className,
                                         PdxReader& pdxReader) = 0;

  /**
   * Serializes this object in Geode PDX format.
   * @param userObject the object which needs to be serialized
   * @param pdxWriter the PdxWriter object to use for serializing the object
   */
  virtual bool toData(const std::shared_ptr<const void>& userObject,
                      const std::string& className, PdxWriter& pdxWriter) = 0;

  /**
   * Get the function pointer to the user function that returns the size of an
   * instance of a user domain object
   * @param className to help select an object sizer for the correct class
   */
  virtual UserObjectSizer getObjectSizer(const std::string& className);
};
}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_PDXSERIALIZER_H_
