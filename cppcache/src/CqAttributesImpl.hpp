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

#ifndef GEODE_CQATTRIBUTESIMPL_H_
#define GEODE_CQATTRIBUTESIMPL_H_

#include <mutex>

#include <geode/CqAttributes.hpp>
#include <geode/CqAttributesMutator.hpp>

namespace apache {
namespace geode {
namespace client {

/**
 * @class CqAttributesImpl CqAttributesImpl.hpp
 *
 * This interface holds all attribute values for a CQ and provides methods for
 * retrieving all attribute settings. This interface can be modified only by
 * the CqAttributesFactory class (before CQ creation) and the
 * CqAttributesMutator
 * interface (after CQ creation).
 *
 * For compatibility rules and default values, see {@link CqAttributesFactory}.
 */
class CqAttributesImpl : public CqAttributes {
 public:
  ~CqAttributesImpl() noexcept override {}

  listener_container_type getCqListeners() override;

  /**
   * Get the CqListener set with the CQ.
   * Returns the CqListener associated with the CQ.
   * If there are more than one CqListener throws IllegalStateException.
   * @see CqListener
   * @return CqListener Object, returns null if there is no CqListener.
   */
  std::shared_ptr<CqListener> getCqListener();
  void addCqListener(const std::shared_ptr<CqListener>& cql);
  void setCqListeners(const listener_container_type& addedListeners);
  void removeCqListener(const std::shared_ptr<CqListener>& cql);
  CqAttributesImpl* clone();

 private:
  listener_container_type m_cqListeners;
  bool m_dataPolicyHasBeenSet;
  std::recursive_mutex m_mutex;
};

}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_CQATTRIBUTESIMPL_H_
