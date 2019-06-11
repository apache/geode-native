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

#include "EntriesMap.hpp"

namespace apache {
namespace geode {
namespace client {
EntriesMap::EntriesMap(std::unique_ptr<EntryFactory> entryFactory)
    : m_entryFactory(std::move(entryFactory)) {}
EntriesMap::~EntriesMap() {}

std::shared_ptr<Cacheable> EntriesMap::getFromDisk(
    const std::shared_ptr<CacheableKey>&,
    std::shared_ptr<MapEntryImpl>&) const {
  return nullptr;
}

const EntryFactory* EntriesMap::getEntryFactory() const {
  return m_entryFactory.get();
}

}  // namespace client
}  // namespace geode
}  // namespace apache
