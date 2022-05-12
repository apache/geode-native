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

#include <geode/CacheableObjectArray.hpp>

#include "PdxReaderImpl.hpp"
#include "PdxType.hpp"
#include "PdxWriterImpl.hpp"

namespace apache {
namespace geode {
namespace client {

PdxUnreadData::PdxUnreadData() : expiryTaskId_{ExpiryTask::invalid()} {}

PdxUnreadData::PdxUnreadData(std::shared_ptr<PdxType> pdxType,
                             std::vector<int32_t> indexes,
                             std::vector<PdxUnreadField> data)
    : pdxType_{pdxType},
      indexes_{std::move(indexes)},
      data_{std::move(data)},
      expiryTaskId_{ExpiryTask::invalid()} {}

void PdxUnreadData::write(PdxWriterImpl& writer) {
  if (data_.empty()) {
    return;
  }

  for (size_t i = 0, n = data_.size(); i < n;) {
    auto&& field = pdxType_->getField(indexes_[i]);
    auto fieldType = field->getType();
    auto&& value = data_[i++];

    // This way we make sure that in case of having an unread PDX field, it
    // would be serialized again and the PdxType ID is always up-to-date, even
    // if there was a cluster restart in between read/write
    if (fieldType == PdxFieldTypes::OBJECT) {
      writer.writeObject(field->getName(), value.getObject());
    } else if (fieldType == PdxFieldTypes::OBJECT_ARRAY) {
      writer.writeObjectArray(
          field->getName(),
          std::dynamic_pointer_cast<CacheableObjectArray>(value.getObject()));
    } else {
      writer.writeRawField(field, value.getRaw());
    }
  }
}

}  // namespace client
}  // namespace geode
}  // namespace apache
