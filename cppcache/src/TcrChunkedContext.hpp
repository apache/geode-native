#pragma once

#ifndef GEODE_TCRCHUNKEDCONTEXT_H_
#define GEODE_TCRCHUNKEDCONTEXT_H_

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
/**
 * @file TcrChunkedContext.hpp
 *
 *
 */

#include <memory>

#include <ace/Semaphore.h>
#include <string>
#include <geode/geode_types.hpp>
#include "Utils.hpp"
#include "AppDomainContext.hpp"

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
  ExceptionPtr m_ex;
  bool m_inSameThread;
  std::unique_ptr<AppDomainContext> appDomainContext;

 protected:
  uint16_t m_dsmemId;

  /** handle a chunk of response message from server */
  virtual void handleChunk(const uint8_t* bytes, int32_t len,
                           uint8_t isLastChunkWithSecurity,
                           const Cache* cache) = 0;

 public:
  inline TcrChunkedResult()
      : m_finalizeSema(nullptr),
        m_ex(nullptr),
        m_inSameThread(false),
        appDomainContext(createAppDomainContext()),
        m_dsmemId(0) {}
  virtual ~TcrChunkedResult() {}
  void setFinalizeSemaphore(ACE_Semaphore* finalizeSema) {
    m_finalizeSema = finalizeSema;
  }
  virtual void setEndpointMemId(uint16_t dsmemId) { m_dsmemId = dsmemId; }
  uint16_t getEndpointMemId() { return m_dsmemId; }
  /**
   * Any cleanup to be done before starting chunk processing, or after
   * failover to a new endpoint.
   */
  virtual void reset() = 0;

  void fireHandleChunk(const uint8_t* bytes, int32_t len,
                       uint8_t isLastChunkWithSecurity, const Cache* cache) {
    if (appDomainContext) {
      appDomainContext->run(
          [this, bytes, len, isLastChunkWithSecurity, &cache]() {
            handleChunk(bytes, len, isLastChunkWithSecurity, cache);
          });
    } else {
      handleChunk(bytes, len, isLastChunkWithSecurity, cache);
    }
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
    if (m_finalizeSema != nullptr) {
      m_finalizeSema->release();
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
    if (m_finalizeSema != nullptr) {
      m_finalizeSema->acquire();
    } else {
      throw NullPointerException(
          "TcrChunkedResult::waitFinalize: null semaphore");
    }
  }

  // getters/setters for the exception, if any, during chunk processing

  inline bool exceptionOccurred() const { return (m_ex != nullptr); }

  inline void setException(Exception& ex) { m_ex.reset(ex.clone()); }

  inline ExceptionPtr& getException() { return m_ex; }
};

typedef std::shared_ptr<TcrChunkedResult> TcrChunkedResultPtr;

/**
 * Holds the context for a chunk including the chunk bytes, length and the
 * {@link TcrChunkedResult} object.
 */
class TcrChunkedContext {
 private:
  const uint8_t* m_bytes;
  const int32_t m_len;
  const uint8_t m_isLastChunkWithSecurity;
  const Cache* m_cache;
  TcrChunkedResult* m_result;

 public:
  inline TcrChunkedContext(const uint8_t* bytes, int32_t len,
                           TcrChunkedResult* result,
                           uint8_t isLastChunkWithSecurity, const Cache* cache)
      : m_bytes(bytes),
        m_len(len),
        m_isLastChunkWithSecurity(isLastChunkWithSecurity),
        m_cache(cache),
        m_result(result) {}

  inline ~TcrChunkedContext() { GF_SAFE_DELETE_ARRAY(m_bytes); }

  inline const uint8_t* getBytes() const { return m_bytes; }

  inline int32_t getLen() const { return m_len; }

  void handleChunk(bool inSameThread) {
    if (m_bytes == nullptr) {
      // this is the last chunk for some set of chunks
      m_result->finalize(inSameThread);
    } else if (!m_result->exceptionOccurred()) {
      try {
        m_result->fireHandleChunk(m_bytes, m_len, m_isLastChunkWithSecurity,
                                  m_cache);
      } catch (Exception& ex) {
        LOGERROR("HandleChunk error message %s, name = %s", ex.getMessage(),
                 ex.getName());
        m_result->setException(ex);
      } catch (std::exception& stdEx) {
        std::string exMsg("HandleChunk exception:: ");
        exMsg += stdEx.what();
        LOGERROR("HandleChunk exception: %s", stdEx.what());
        UnknownException ex(exMsg.c_str());
        m_result->setException(ex);
      } catch (...) {
        std::string exMsg("Unknown exception in ");
        exMsg += Utils::demangleTypeName(typeid(*m_result).name())->asChar();
        exMsg +=
            "::handleChunk while processing response, possible serialization "
            "mismatch";
        LOGERROR(exMsg.c_str());
        UnknownException ex(exMsg.c_str());
        m_result->setException(ex);
      }
    }
  }
};
}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_TCRCHUNKEDCONTEXT_H_
