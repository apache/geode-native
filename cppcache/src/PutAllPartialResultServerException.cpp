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

#include "PutAllPartialResultServerException.hpp"

namespace apache {
namespace geode {
namespace client {

PutAllPartialResultServerException::PutAllPartialResultServerException(
    std::shared_ptr<PutAllPartialResult> result) {
  LOGDEBUG("Partial keys are processed in putAll");
  m_result = result;
}

PutAllPartialResultServerException::PutAllPartialResultServerException() {
  LOGDEBUG("Partial keys are processed in putAll");
  ACE_Recursive_Thread_Mutex responseLock;
  m_result = std::make_shared<PutAllPartialResult>(-1, responseLock);
}

void PutAllPartialResultServerException::consolidate(
    std::shared_ptr<PutAllPartialResultServerException> pre) {
  m_result->consolidate(pre->getResult());
}

void PutAllPartialResultServerException::consolidate(
    std::shared_ptr<PutAllPartialResult> otherResult) {
  m_result->consolidate(otherResult);
}

std::shared_ptr<PutAllPartialResult> PutAllPartialResultServerException::getResult() {
  return m_result;
}

std::shared_ptr<VersionedCacheableObjectPartList>
PutAllPartialResultServerException::getSucceededKeysAndVersions() {
  return m_result->getSucceededKeysAndVersions();
}

std::shared_ptr<Exception> PutAllPartialResultServerException::getFailure() {
  return m_result->getFailure();
}

bool PutAllPartialResultServerException::hasFailure() {
  return m_result->hasFailure();
}

std::shared_ptr<CacheableKey> PutAllPartialResultServerException::getFirstFailedKey() {
  return m_result->getFirstFailedKey();
}

std::shared_ptr<CacheableString> PutAllPartialResultServerException::getMessage() {
  return m_result->toString();
}

void PutAllPartialResultServerException::toData(DataOutput& output) const {
  throw IllegalStateException(
      "PutAllPartialResultServerException::toData is not intended for use.");
}

void PutAllPartialResultServerException::fromData(DataInput& input) {
  throw IllegalStateException(
      "PutAllPartialResultServerException::fromData is not intended for use.");
}

int32_t PutAllPartialResultServerException::classId() const {
  throw IllegalStateException(
      "PutAllPartialResultServerException::classId is not intended for use.");
  return 0;
}

uint32_t PutAllPartialResultServerException::objectSize() const {
  throw IllegalStateException(
      "PutAllPartialResultServerException::objectSize is not intended for "
      "use.");
  return 0;
}

int8_t PutAllPartialResultServerException::typeId() const {
  return static_cast<int8_t>(0);
}

PutAllPartialResultServerException::PutAllPartialResultServerException(
    std::shared_ptr<CacheableString> msg)
    : m_message(msg) {}

}  // namespace client
}  // namespace geode
}  // namespace apache
