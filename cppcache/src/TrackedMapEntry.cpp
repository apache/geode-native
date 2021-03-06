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

#include "TrackedMapEntry.hpp"

#include "MapEntryImpl.hpp"

namespace apache {
namespace geode {
namespace client {

void TrackedMapEntry::getKey(std::shared_ptr<CacheableKey>& result) const {
  m_entry->getKeyI(result);
}

void TrackedMapEntry::getValue(std::shared_ptr<Cacheable>& result) const {
  m_entry->getValueI(result);
}

void TrackedMapEntry::setValue(const std::shared_ptr<Cacheable>& value) {
  m_entry->setValueI(value);
}

LRUEntryProperties& TrackedMapEntry::getLRUProperties() {
  return m_entry->getLRUProperties();
}

ExpEntryProperties& TrackedMapEntry::getExpProperties() {
  return m_entry->getExpProperties();
}
VersionStamp& TrackedMapEntry::getVersionStamp() {
  throw FatalInternalException(
      "MapEntry::getVersionStamp for TrackedMapEntry is not applicable");
}
void TrackedMapEntry::cleanup(const CacheEventFlags eventFlags) {
  m_entry->cleanup(eventFlags);
}

}  // namespace client
}  // namespace geode
}  // namespace apache
