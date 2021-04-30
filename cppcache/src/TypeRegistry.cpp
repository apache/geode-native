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

#include "geode/TypeRegistry.hpp"

#include "CacheImpl.hpp"
#include "SerializationRegistry.hpp"

namespace apache {
namespace geode {
namespace client {

TypeRegistry::TypeRegistry(CacheImpl* cache) : m_cache(cache) {}

void TypeRegistry::registerType(TypeFactoryMethod creationFunction,
                                int32_t id) {
  m_cache->getSerializationRegistry()->addDataSerializableType(creationFunction,
                                                               id);
}

void TypeRegistry::registerPdxType(TypeFactoryMethodPdx creationFunction) {
  m_cache->getSerializationRegistry()->addPdxSerializableType(creationFunction);
}

void TypeRegistry::registerPdxSerializer(
    std::shared_ptr<PdxSerializer> pdxSerializer) {
  m_cache->getSerializationRegistry()->setPdxSerializer(pdxSerializer);
}

std::shared_ptr<PdxSerializer> TypeRegistry::getPdxSerializer() {
  return m_cache->getSerializationRegistry()->getPdxSerializer();
}

TypeFactoryMethod TypeRegistry::getCreationFunction(int32_t objectId) {
  return m_cache->getSerializationRegistry()->getDataSerializableCreationMethod(
      objectId);
}

}  // namespace client
}  // namespace geode
}  // namespace apache
