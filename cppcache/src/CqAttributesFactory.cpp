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

#include <geode/CqAttributesFactory.hpp>
#include <geode/ExceptionTypes.hpp>

#include "CqAttributesImpl.hpp"

namespace apache {
namespace geode {
namespace client {

CqAttributesFactory::CqAttributesFactory() {
  m_cqAttributes = std::make_shared<CqAttributesImpl>();
}
CqAttributesFactory::CqAttributesFactory(
    const std::shared_ptr<CqAttributes> &cqAttributes) {
  auto vl = cqAttributes->getCqListeners();
  m_cqAttributes = std::make_shared<CqAttributesImpl>();
  std::static_pointer_cast<CqAttributesImpl>(m_cqAttributes)
      ->setCqListeners(vl);
}

void CqAttributesFactory::addCqListener(
    const std::shared_ptr<CqListener> &cqListener) {
  if (cqListener == nullptr) {
    throw IllegalArgumentException("addCqListener parameter was null");
  }
  std::static_pointer_cast<CqAttributesImpl>(m_cqAttributes)
      ->addCqListener(cqListener);
}

void CqAttributesFactory::initCqListeners(
    const CqAttributesImpl::listener_container_type &cqListeners) {
  std::static_pointer_cast<CqAttributesImpl>(m_cqAttributes)
      ->setCqListeners(cqListeners);
}

std::shared_ptr<CqAttributes> CqAttributesFactory::create() {
  return std::shared_ptr<CqAttributes>(
      std::static_pointer_cast<CqAttributesImpl>(m_cqAttributes)->clone());
}

}  // namespace client
}  // namespace geode
}  // namespace apache
