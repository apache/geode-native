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

#include "PdxRemoteWriterFactoryImpl.hpp"

#include "PdxRemoteWriter.hpp"
#include "PdxType.hpp"
#include "PdxTypeRegistry.hpp"
#include "util/cxx_extensions.hpp"

namespace apache {
namespace geode {
namespace client {

PdxRemoteWriterFactoryImpl::~PdxRemoteWriterFactoryImpl() = default;

std::unique_ptr<PdxRemoteWriter> PdxRemoteWriterFactoryImpl::create(
    DataOutput& output, const std::shared_ptr<PdxSerializable>& object,
    const std::shared_ptr<PdxTypeRegistry>& pdxTypeRegistry,
    const std::shared_ptr<PdxType>& localType) {
  if (auto pd = pdxTypeRegistry->getPreserveData(object)) {
    auto mergedType = pdxTypeRegistry->getPdxType(pd->getMergedTypeId());
    return cxx::make_unique<PdxRemoteWriter>(output, mergedType, pd,
                                             pdxTypeRegistry);
  } else {
    return cxx::make_unique<PdxRemoteWriter>(output, localType,
                                             pdxTypeRegistry);
  }
}

}  // namespace client
}  // namespace geode
}  // namespace apache
