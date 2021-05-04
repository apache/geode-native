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
#include "util/concurrent/binary_semaphore.hpp"

namespace apache {
namespace geode {
namespace client {

/**
 * Base class for holding chunked results, processing a chunk
 * and signalling end of chunks using semaphore.
 */
class TcrChunkedResult {
 private:
  binary_semaphore* finalize_semaphore_;
  std::shared_ptr<Exception> m_ex;
  bool m_inSameThread;

 protected:
  uint16_t m_dsmemId;

  /** handle a chunk of response message from server */
  virtual void handleChunk(const uint8_t* bytes, int32_t len,
                           uint8_t isLastChunkWithSecurity,
                           const CacheImpl* cacheImpl) = 0;

 public:
  inline TcrChunkedResult()
      : finalize_semaphore_(nullptr),
        m_ex(nullptr),
        m_inSameThread(false),
        m_dsmemId(0) {}
  virtual ~TcrChunkedResult() noexcept {}
  void setFinalizeSemaphore(binary_semaphore* finalizeSema) {
    finalize_semaphore_ = finalizeSema;
  }
  virtual void setEndpointMemId(uint16_t dsmemId) { m_dsmemId = dsmemId; }
  uint16_t getEndpointMemId() { return m_dsmemId; }
  /**
   * Any cleanup to be done before starting chunk processing, or after
   * failover to a new endpoint.
   */
  virtual void reset() = 0;

  void fireHandleChunk(const uint8_t* bytes, int32_t len,
                       uint8_t isLastChunkWithSecurity,
                       const CacheImpl* cacheImpl) {
    handleChunk(bytes, len, isLastChunkWithSecurity, cacheImpl);
  }

  /**
   * Send signal from chunk processor thread that processing of chunks
   * is complete
   */
  virtual void finalize(bool inSamethread) {
    if (inSamethread) {
      m_inSameThread = true;
      return;
    }
    if (finalize_semaphore_ != nullptr) {
      finalize_semaphore_->release();
    } else {
      throw NullPointerException("TcrChunkedResult::finalize: null semaphore");
    }
  }

  /**
   * Wait for the chunk processor thread to complete processing
   * of the chunks
   */
  virtual void waitFinalize() const {
    if (m_inSameThread) return;
    if (finalize_semaphore_ != nullptr) {
      finalize_semaphore_->acquire();
    } else {
      throw NullPointerException(
          "TcrChunkedResult::waitFinalize: null semaphore");
    }
  }

  // getters/setters for the exception, if any, during chunk processing

  inline bool exceptionOccurred() const { return (m_ex != nullptr); }

  inline void setException(std::shared_ptr<Exception> ex) { m_ex = ex; }

  inline std::shared_ptr<Exception>& getException() { return m_ex; }

  inline void clearException() { m_ex = nullptr; }
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
  inline TcrChunkedContext(const std::vector<uint8_t> chunk, int32_t len,
                           TcrChunkedResult* result,
                           uint8_t isLastChunkWithSecurity,
                           const CacheImpl* cacheImpl)
      : m_chunk(chunk),
        m_len(len),
        m_isLastChunkWithSecurity(isLastChunkWithSecurity),
        m_cache(cacheImpl),
        m_result(result) {}

  inline ~TcrChunkedContext() = default;

  inline const uint8_t* getBytes() const { return m_chunk.data(); }

  inline size_t getLen() const { return m_chunk.size(); }

  void handleChunk(bool inSameThread) {
    if (m_chunk.empty()) {
      // this is the last chunk for some set of chunks
      m_result->finalize(inSameThread);
    } else if (!m_result->exceptionOccurred()) {
      try {
        m_result->fireHandleChunk(m_chunk.data(), m_len,
                                  m_isLastChunkWithSecurity, m_cache);
      } catch (Exception& ex) {
        LOG_ERROR("HandleChunk error message %s, name = %s", ex.what(),
                  ex.getName().c_str());
        m_result->setException(std::make_shared<Exception>(ex));
      } catch (std::exception& stdEx) {
        std::string exMsg("HandleChunk exception:: ");
        exMsg += stdEx.what();
        LOG_ERROR("HandleChunk exception: %s", stdEx.what());
        auto ex = std::make_shared<UnknownException>(exMsg.c_str());
        m_result->setException(ex);
      } catch (...) {
        std::string exMsg("Unknown exception in ");
        exMsg += Utils::demangleTypeName(typeid(*m_result).name());
        exMsg +=
            "::handleChunk while processing response, possible serialization "
            "mismatch";
        LOG_ERROR(exMsg.c_str());
        auto ex = std::make_shared<UnknownException>(exMsg.c_str());
        m_result->setException(ex);
      }
    }
  }
};
}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_TCRCHUNKEDCONTEXT_H_
