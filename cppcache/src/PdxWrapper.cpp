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

#include "PdxHelper.hpp"
#include "SerializationRegistry.hpp"
#include "Utils.hpp"

namespace apache {
namespace geode {
namespace client {

PdxWrapper::PdxWrapper(std::shared_ptr<void> userObject, std::string className)
    : m_userObject(userObject), m_className(className) {}

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
    output.getPdxSerializer()->toData(m_userObject, m_className, output);
  } else {
    LOG_ERROR("User object is nullptr or detached in PdxWrapper toData");
    throw IllegalStateException(
        "User object is nullptr or detached in PdxWrapper toData");
  }
}

void PdxWrapper::fromData(PdxReader& input) {
  if (auto pdxSerializer = input.getPdxSerializer()) {
    m_userObject = pdxSerializer->fromData(m_className, input);
  }
}

std::string PdxWrapper::toString() const {
  return "PdxWrapper for " + m_className;
}

}  // namespace client
}  // namespace geode
}  // namespace apache
