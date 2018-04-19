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

#include <geode/PdxWrapper.hpp>

#include "Utils.hpp"
#include "PdxHelper.hpp"
#include "SerializationRegistry.hpp"

namespace apache {
namespace geode {
namespace client {

PdxWrapper::PdxWrapper(std::shared_ptr<void> userObject, std::string className,
                       std::shared_ptr<PdxSerializer> pdxSerializerPtr)
    : m_userObject(userObject),
      m_className(className),
      m_serializer(pdxSerializerPtr) {
  if (m_serializer == nullptr) {
    LOGERROR("No registered PDX serializer found for PdxWrapper");
    throw IllegalArgumentException(
        "No registered PDX serializer found for PdxWrapper");
  }

  /* m_sizer can be nullptr - required only if heap LRU is enabled */
  m_sizer = m_serializer->getObjectSizer(className);
}

PdxWrapper::PdxWrapper(std::string className,
                       std::shared_ptr<PdxSerializer> pdxSerializerPtr)
    : m_className(className), m_serializer(pdxSerializerPtr) {
  if (m_serializer == nullptr) {
    LOGERROR(
        "No registered PDX serializer found for PdxWrapper deserialization");
    throw IllegalArgumentException(
        "No registered PDX serializer found for PdxWrapper deserialization");
  }

  /* m_sizer can be nullptr - required only if heap LRU is enabled */
  m_sizer = m_serializer->getObjectSizer(className);

  m_userObject = nullptr;
}

std::shared_ptr<void> PdxWrapper::getObject() { return m_userObject; }

const std::string& PdxWrapper::getClassName() const { return m_className; }

bool PdxWrapper::operator==(const CacheableKey& other) const {
  auto wrapper = dynamic_cast<const PdxWrapper*>(&other);
  if (wrapper == nullptr) {
    return false;
  }
  return m_userObject == wrapper->m_userObject;
}

int32_t PdxWrapper::hashcode() const {
  auto hash = static_cast<int64_t>(
      reinterpret_cast<std::uintptr_t>(m_userObject.get()));
  return apache::geode::client::internal::hashcode(hash);
}

void PdxWrapper::toData(PdxWriter& output) const {
  if (m_userObject != nullptr) {
    output.getPdxSerializer()->toData(m_userObject, m_className.c_str(), output);
  } else {
    LOGERROR("User object is nullptr or detached in PdxWrapper toData");
    throw IllegalStateException(
        "User object is nullptr or detached in PdxWrapper toData");
  }
}

void PdxWrapper::fromData(PdxReader& input) {
  m_userObject = m_serializer->fromData(m_className.c_str(), input);
}

void PdxWrapper::toData(DataOutput& output) const {
  PdxHelper::serializePdx(output, *this);
}

void PdxWrapper::fromData(DataInput&) {
  LOGERROR("PdxWrapper fromData should not have been called");
  throw IllegalStateException(
      "PdxWrapper fromData should not have been called");
}

size_t PdxWrapper::objectSize() const {
  if (m_sizer == nullptr || m_userObject == nullptr) {
    return 0;
  } else {
    return m_sizer(m_userObject, m_className.c_str());
  }
}

std::string PdxWrapper::toString() const {
  std::string message = "PdxWrapper for ";
  message += m_className;
  return message.c_str();
}

}  // namespace client
}  // namespace geode
}  // namespace apache
