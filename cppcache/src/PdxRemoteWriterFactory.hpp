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

#ifndef GEODE_PDXREMOTEWRITERFACTORY_H_
#define GEODE_PDXREMOTEWRITERFACTORY_H_

#include <geode/internal/geode_globals.hpp>

#include <memory>

namespace apache {
namespace geode {
namespace client {

class DataOutput;
class PdxSerializable;
class PdxRemoteWriter;
class PdxType;
class PdxTypeRegistry;

class APACHE_GEODE_EXPORT PdxRemoteWriterFactory {
 public:
  virtual ~PdxRemoteWriterFactory() {}

  virtual std::unique_ptr<PdxRemoteWriter> create(
      DataOutput& output,
      const std::shared_ptr<PdxSerializable>& object,
      const std::shared_ptr<PdxTypeRegistry>& pdxTypeRegistry,
      const std::shared_ptr<PdxType>& localType) = 0;
};

}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_PDXREMOTEWRITERFACTORY_H_
