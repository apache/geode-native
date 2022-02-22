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

#ifndef GEODE_CHUNKEDFUNCTIONEXECUTIONRESPONSE_H_
#define GEODE_CHUNKEDFUNCTIONEXECUTIONRESPONSE_H_

#include <memory>

#include "FunctionAttributes.hpp"
#include "TcrChunkedContext.hpp"

namespace apache {
namespace geode {
namespace client {

class BucketServerLocation;
class CacheableHashSet;
class CacheableString;
class ChunkedFunctionExecutionResponse;
class Region;
class ResultCollector;
class TcrChunkedResult;
class TcrEndpoint;
class TcrMessage;
class TcrMessageReply;
class ThinClientPoolDM;
class UserAttributes;

/**
 * Handle each chunk of the chunked function execution response.
 */
class ChunkedFunctionExecutionResponse : public TcrChunkedResult {
 private:
  TcrMessage& msg_;
  bool hasResult_;
  std::shared_ptr<ResultCollector> resultCollector_;
  std::shared_ptr<std::recursive_mutex> resultCollectorMutex_;

 public:
  inline ChunkedFunctionExecutionResponse(TcrMessage& msg, bool hasResult,
                                          std::shared_ptr<ResultCollector> rc)
      : TcrChunkedResult(), msg_(msg), hasResult_(hasResult),
        resultCollector_(rc) {}

  inline ChunkedFunctionExecutionResponse(
      TcrMessage& msg, bool getResult, std::shared_ptr<ResultCollector> rc,
      const std::shared_ptr<std::recursive_mutex>& resultCollectorLock)
      : TcrChunkedResult(),
        msg_(msg),
        hasResult_(getResult),
        resultCollector_(rc),
        resultCollectorMutex_(resultCollectorLock) {}

  ChunkedFunctionExecutionResponse(const ChunkedFunctionExecutionResponse&) =
      delete;
  ChunkedFunctionExecutionResponse& operator=(
      const ChunkedFunctionExecutionResponse&) = delete;

  ~ChunkedFunctionExecutionResponse() noexcept override = default;

  inline bool getResult() const { return hasResult_; }

  void handleChunk(const uint8_t* chunk, int32_t chunkLen,
                   uint8_t isLastChunkWithSecurity,
                   const CacheImpl* cacheImpl) override;
  void reset() override;
};

}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_CHUNKEDFUNCTIONEXECUTIONRESPONSE_H_
