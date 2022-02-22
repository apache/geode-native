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

#include "ChunkedFunctionExecutionResponse.hpp"

#include <geode/ResultCollector.hpp>
#include <geode/UserFunctionExecutionException.hpp>

#include "CacheImpl.hpp"
#include "TcrMessage.hpp"

namespace apache {
namespace geode {
namespace client {

void ChunkedFunctionExecutionResponse::reset() {}

void ChunkedFunctionExecutionResponse::handleChunk(
    const uint8_t* chunk, int32_t chunkLen, uint8_t isLastChunkWithSecurity,
    const CacheImpl* cacheImpl) {
  LOGDEBUG("ChunkedFunctionExecutionResponse::handleChunk");
  auto input = cacheImpl->createDataInput(chunk, chunkLen, msg_.getPool());

  uint32_t partLen;

  TcrMessageHelper::ChunkObjectType arrayType;
  if ((arrayType = TcrMessageHelper::readChunkPartHeader(
           msg_, input, "ChunkedFunctionExecutionResponse", partLen,
           isLastChunkWithSecurity)) ==
      TcrMessageHelper::ChunkObjectType::EXCEPTION) {
    // encountered an exception part, so return without reading more
    msg_.readSecureObjectPart(input, false, true, isLastChunkWithSecurity);
    return;
  }

  if (!hasResult_) {
    return;
  }

  if (static_cast<TcrMessageHelper::ChunkObjectType>(arrayType) ==
      TcrMessageHelper::ChunkObjectType::NULL_OBJECT) {
    LOGDEBUG("ChunkedFunctionExecutionResponse::handleChunk nullptr object");
    //	m_functionExecutionResults->push_back(nullptr);
    msg_.readSecureObjectPart(input, false, true, isLastChunkWithSecurity);
    return;
  }

  // from here need to look value part + memberid AND -1 for array type
  auto startLen = static_cast<size_t>(input.getBytesRead() - 1);

  // read and ignore array length
  input.readArrayLength();

  // read a byte to determine whether to read exception part for sendException
  // or read objects.
  auto partType = static_cast<DSCode>(input.read());
  bool isExceptionPart = false;
  // See If partType is JavaSerializable
  const int CHUNK_HDR_LEN = 5;
  const int SECURE_PART_LEN = 5 + 8;
  bool readPart = true;
  LOGDEBUG(
      "ChunkedFunctionExecutionResponse::handleChunk chunkLen = %d & partLen "
      "= "
      "%d ",
      chunkLen, partLen);
  if (partType == DSCode::JavaSerializable) {
    isExceptionPart = true;
    // reset the input.
    input.reset();

    if (((isLastChunkWithSecurity & 0x02) &&
         (chunkLen - static_cast<int32_t>(partLen) <=
          CHUNK_HDR_LEN + SECURE_PART_LEN)) ||
        (((isLastChunkWithSecurity & 0x02) == 0) &&
         (chunkLen - static_cast<int32_t>(partLen) <= CHUNK_HDR_LEN))) {
      readPart = false;
      partLen = input.readInt32();
      input.advanceCursor(1);  // skip isObject byte
      input.advanceCursor(partLen);
    } else {
      // skip first part i.e JavaSerializable.
      TcrMessageHelper::skipParts(msg_, input, 1);

      // read the second part which is string in usual manner, first its
      // length.
      partLen = input.readInt32();

      // then isObject byte
      input.read();  // ignore iSobject

      startLen = input.getBytesRead();  // reset from here need to look value
      // part + memberid AND -1 for array type

      // Since it is contained as a part of other results, read arrayType
      // which is arrayList = 65.
      input.read();

      // read and ignore its len which is 2
      input.readArrayLength();
    }
  } else {
    // rewind cursor by 1 to what we had read a byte to determine whether to
    // read exception part or read objects.
    input.rewindCursor(1);
  }

  // Read either object or exception string from sendException.
  std::shared_ptr<Serializable> value;
  // std::shared_ptr<Cacheable> memberId;
  if (readPart) {
    input.readObject(value);
    // TODO: track this memberId for PrFxHa
    // input.readObject(memberId);
    auto objectlen = input.getBytesRead() - startLen;

    auto memberIdLen = partLen - objectlen;
    input.advanceCursor(memberIdLen);
    LOGDEBUG("function partlen = %d , objectlen = %z,  memberidlen = %z ",
             partLen, objectlen, memberIdLen);
    LOGDEBUG("function input.getBytesRemaining() = %z ",
             input.getBytesRemaining());

  } else {
    value = CacheableString::create("Function exception result.");
  }
  if (resultCollector_) {
    std::shared_ptr<Cacheable> result = nullptr;
    if (isExceptionPart) {
      result = std::make_shared<UserFunctionExecutionException>(
          std::dynamic_pointer_cast<CacheableString>(value)->value());
    } else {
      result = value;
    }
    if (resultCollectorMutex_) {
      std::lock_guard<decltype(*resultCollectorMutex_)> guard(
          *resultCollectorMutex_);
      resultCollector_->addResult(result);
    } else {
      resultCollector_->addResult(result);
    }
  }

  msg_.readSecureObjectPart(input, false, true, isLastChunkWithSecurity);
}

}  // namespace client
}  // namespace geode
}  // namespace apache
