
#pragma once

#ifndef GEODE_TYPEREGISTRY_H_
#define GEODE_TYPEREGISTRY_H_

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

#include "geode_globals.hpp"
#include "geode_types.hpp"
#include "Serializable.hpp"
namespace apache {
namespace geode {
namespace client {

class TypeRegistry {
 public:
  TypeRegistry(const Cache& cache);

  /**
   * @brief register an instance factory method for a given type.
   * During registration the factory will be invoked to extract the typeId
   * to associate with this function.
   * @throws IllegalStateException if the typeId has already been
   * registered, or there is an error in registering the type; check errno
   * for more information in the latter case.
   */
  void registerType(TypeFactoryMethod creationFunction);

  /**
   * @brief register an Pdx instance factory method for a given type.
   * @throws IllegalStateException if the typeName has already been registered,
   *         or there is an error in registering the type; check errno for
   *         more information in the latter case.
   */
  void registerPdxType(TypeFactoryMethodPdx creationFunction);

  /**
   * Register the PDX serializer which can handle serialization for instances of
   * user domain classes.
   * @see PdxSerializer
   */
  void registerPdxSerializer(PdxSerializerPtr pdxSerializer);

 protected:
 private:
  const Cache& m_cache;
};
}  // namespace client
}  // namespace geode
}  // namespace apache

#endif //GEODE_TYPEREGISTRY_H_

