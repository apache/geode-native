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

/**
 * @file
 */

#include <geode/SelectResultsIterator.hpp>

namespace apache {
namespace geode {
namespace client {

SelectResultsIterator::SelectResultsIterator(
    const std::shared_ptr<CacheableVector>& vectorSR,
    std::shared_ptr<SelectResults> srp)
    : m_vectorSR(vectorSR), m_nextIndex(0), m_srp(srp) {}

bool SelectResultsIterator::hasNext() const {
  return m_nextIndex < m_vectorSR->size();
}

const std::shared_ptr<Serializable> SelectResultsIterator::next() {
  if (!hasNext()) return nullptr;

  return m_vectorSR->operator[](m_nextIndex++);
}

bool SelectResultsIterator::moveNext() {
  if (hasNext()) {
    m_nextIndex++;
    return true;
  } else {
    return false;
  }
}

const std::shared_ptr<Serializable> SelectResultsIterator::current() const {
  if (m_nextIndex == 0 || m_nextIndex > m_vectorSR->size()) return nullptr;

  return m_vectorSR->operator[](m_nextIndex - 1);
}

void SelectResultsIterator::reset() { m_nextIndex = 0; }
}  // namespace client
}  // namespace geode
}  // namespace apache
