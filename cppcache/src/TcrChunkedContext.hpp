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

#ifndef GEODE_TCRCHUNKEDCONTEXT_H_
#define GEODE_TCRCHUNKEDCONTEXT_H_

#include <memory>
#include <string>

#include <ace/Semaphore.h>

#include "AppDomainContext.hpp"
#include "Utils.hpp"

namespace apache {
namespace geode {
namespace client {

/**
 * Base class for holding chunked results, processing a chunk
 * and signalling end of chunks using semaphore.
 */
class TcrChunkedResult {
 private:
  ACE_Semaphore* m_finalizeSema;
  std::shared_ptr<Exception> m_ex;
  bool m_inSameThread;

 protected:
  uint16_t m_dsmemId;

  /** handle a chunk of response message from server */
  virtual void handleChunk(const uint8_t* bytes, int32_t len,
                           uint8_t isLastChunkWithSecurity,
                           const CacheImpl* cacheImpl) = 0;

 public:
  TcrChunkedResult();
  virtual ~TcrChunkedResult() = default;
  void setFinalizeSemaphore(ACE_Semaphore* finalizeSema);
  virtual void setEndpointMemId(uint16_t dsmemId);
  uint16_t getEndpointMemId();
  /**
   * Any cleanup to be done before starting chunk processing, or after
   * failover to a new endpoint.
   */
  virtual void reset() = 0;

  void fireHandleChunk(const uint8_t* bytes, int32_t len,
                       uint8_t isLastChunkWithSecurity,
                       const CacheImpl* cacheImpl);

  /**
   * Send signal from chunk processor thread that processing of chunks
   * is complete
   */
  virtual void finalize(bool inSamethread);

  /**
   * Wait for the chunk processor thread to complete processing
   * of the chunks
   */
  virtual void waitFinalize() const;

  // getters/setters for the exception, if any, during chunk processing

  bool exceptionOccurred() const;

  void setException(std::shared_ptr<Exception> ex);

  std::shared_ptr<Exception>& getException();

  void clearException();
};

/**
 * Holds the context for a chunk including the chunk bytes, length and the
 * {@link TcrChunkedResult} object.
 */
class TcrChunkedContext {
 private:
  const std::vector<uint8_t> m_chunk;
  const int32_t m_len;
  const uint8_t m_isLastChunkWithSecurity;
  const CacheImpl* m_cache;
  TcrChunkedResult* m_result;

 public:
  TcrChunkedContext(const std::vector<uint8_t> chunk, int32_t len,
                    TcrChunkedResult* result, uint8_t isLastChunkWithSecurity,
                    const CacheImpl* cacheImpl);

  ~TcrChunkedContext() = default;

  const uint8_t* getBytes() const;

  size_t getLen() const;

  void handleChunk(bool inSameThread);
};
}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_TCRCHUNKEDCONTEXT_H_
