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

#include <geode/Delta.hpp>
#include <geode/Cache.hpp>

namespace apache {
namespace geode {
namespace client {

Delta::Delta(Cache* cache) : m_cache(cache) {}

std::shared_ptr<Delta> Delta::clone() const {
  auto out = m_cache->createDataOutput();
  auto ptr = dynamic_cast<const Cacheable*>(this);
  out->writeObject(ptr);
  auto in = m_cache->createDataInput(out->getBuffer(), out->getBufferLength());
  std::shared_ptr<Cacheable> theClonePtr;
  in->readObject(theClonePtr);
  return std::dynamic_pointer_cast<Delta>(theClonePtr);
}

}  // namespace client
}  // namespace geode
}  // namespace apache
