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

#include "PdxUnreadData.hpp"

#include "PdxReaderImpl.hpp"
#include "PdxType.hpp"
#include "PdxWriterImpl.hpp"

namespace apache {
namespace geode {
namespace client {

PdxUnreadData::PdxUnreadData()
    : expiryTaskId_{ExpiryTask::invalid()} {}

PdxUnreadData::PdxUnreadData(std::shared_ptr<PdxType> pdxType,
                             std::vector<int32_t> indexes,
                             std::vector<std::vector<uint8_t>> data)
    : pdxType_{pdxType},
      indexes_{std::move(indexes)},
      data_{std::move(data)},
      expiryTaskId_{ExpiryTask::invalid()} {}

void PdxUnreadData::write(PdxWriterImpl &writer) {
  if (data_.empty()) {
    return;
  }

  for (size_t i = 0, n = data_.size(); i < n; ++i) {
    writer.writeRawField(pdxType_->getField(indexes_[i]), data_[i]);
  }
}

}  // namespace client
}  // namespace geode
}  // namespace apache
