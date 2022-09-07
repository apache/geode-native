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

#ifndef GEODE_TCR_MESSAGE_DESTROY_H_
#define GEODE_TCR_MESSAGE_DESTROY_H_

#include <cinttypes>
#include <memory>

#include "EventOperation.hpp"
#include "TcrMessage.hpp"

namespace apache {
namespace geode {
namespace client {

class TcrMessageDestroy : public TcrMessage {
 public:
  TcrMessageDestroy(DataOutput* dataOutput, const Region* region,
                    const std::shared_ptr<CacheableKey>& key,
                    const std::shared_ptr<Cacheable>& value,
                    const std::shared_ptr<Serializable>& cbArg,
                    EventOperation operation,
                    ThinClientBaseDM* dm = nullptr);

  ~TcrMessageDestroy() override = default;
};


}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_TCR_MESSAGE_DESTROY_H_
