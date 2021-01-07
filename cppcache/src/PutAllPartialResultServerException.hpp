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

#ifndef GEODE_PUTALLPARTIALRESULTSERVEREXCEPTION_H_
#define GEODE_PUTALLPARTIALRESULTSERVEREXCEPTION_H_

#include <geode/CacheableString.hpp>
#include <geode/Serializable.hpp>

#include "PutAllPartialResult.hpp"
#include "VersionedCacheableObjectPartList.hpp"

namespace apache {
namespace geode {
namespace client {

class PutAllPartialResultServerException;

/**
 * @brief PutAllPartialResultServerException class is used to encapsulate
 *geode PutAllPartialResultServerException in case of PutAll execution.
 **/
class PutAllPartialResultServerException : public Serializable {
  /**
   * @brief public methods
   */
 public:
  explicit PutAllPartialResultServerException(
      std::shared_ptr<PutAllPartialResult> result);

  PutAllPartialResultServerException();

  // Consolidate exceptions
  void consolidate(std::shared_ptr<PutAllPartialResultServerException> pre);

  void consolidate(std::shared_ptr<PutAllPartialResult> otherResult);

  std::shared_ptr<PutAllPartialResult> getResult();

  /**
   * Returns the key set in exception
   */
  std::shared_ptr<VersionedCacheableObjectPartList>
  getSucceededKeysAndVersions();

  std::shared_ptr<Exception> getFailure();

  // Returns there's failedKeys
  bool hasFailure();

  std::shared_ptr<CacheableKey> getFirstFailedKey();

  std::shared_ptr<CacheableString> what();

  /**
   * @brief destructor
   */
  ~PutAllPartialResultServerException() override = default;

  /**
   * @brief constructors
   */
  explicit PutAllPartialResultServerException(
      std::shared_ptr<CacheableString> msg);

  /**
   *@brief return as std::shared_ptr<CacheableString> the Exception name
   *returned from geode sendException api.
   **/
  std::shared_ptr<CacheableString> getName() {
    const char* msg = "PutAllPartialResultServerException";
    auto str = CacheableString::create(msg);
    return str;
  }

 private:
  // never implemented.
  PutAllPartialResultServerException(
      const PutAllPartialResultServerException& other);
  void operator=(const PutAllPartialResultServerException& other);

  std::shared_ptr<CacheableString> m_message;  // error message
  std::shared_ptr<PutAllPartialResult> m_result;
};

}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_PUTALLPARTIALRESULTSERVEREXCEPTION_H_
