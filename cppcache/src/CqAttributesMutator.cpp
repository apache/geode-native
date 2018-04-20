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
#include <geode/CqAttributesMutator.hpp>

#include "CqAttributesImpl.hpp"

using namespace apache::geode::client;
CqAttributesMutator::CqAttributesMutator(const std::shared_ptr<CqAttributes>& impl)
    : m_cqAttributes(impl) {}

void CqAttributesMutator::addCqListener(
    const std::shared_ptr<CqListener>& aListener) {
  std::static_pointer_cast<CqAttributesImpl>(m_cqAttributes)
      ->addCqListener(aListener);
}

void CqAttributesMutator::removeCqListener(
    const std::shared_ptr<CqListener>& aListener) {
  std::static_pointer_cast<CqAttributesImpl>(m_cqAttributes)
      ->removeCqListener(aListener);
}

void CqAttributesMutator::setCqListeners(
    const CqAttributesImpl::listener_container_type& newListeners) {
  std::static_pointer_cast<CqAttributesImpl>(m_cqAttributes)
      ->setCqListeners(newListeners);
}
