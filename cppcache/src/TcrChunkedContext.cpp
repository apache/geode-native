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

#include "TcrChunkedContext.hpp"

namespace apache {
namespace geode {
namespace client {

TcrChunkedResult::TcrChunkedResult()
    : m_finalizeSema(nullptr),
      m_ex(nullptr),
      m_inSameThread(false),
      m_dsmemId(0) {}
TcrChunkedResult::~TcrChunkedResult() {}
void TcrChunkedResult::setFinalizeSemaphore(ACE_Semaphore* finalizeSema) {
  m_finalizeSema = finalizeSema;
}
void TcrChunkedResult::setEndpointMemId(uint16_t dsmemId) {
  m_dsmemId = dsmemId;
}
uint16_t TcrChunkedResult::getEndpointMemId() { return m_dsmemId; }

void TcrChunkedResult::fireHandleChunk(const uint8_t* bytes, int32_t len,
                                       uint8_t isLastChunkWithSecurity,
                                       const CacheImpl* cacheImpl) {
  handleChunk(bytes, len, isLastChunkWithSecurity, cacheImpl);
}

void TcrChunkedResult::finalize(bool inSamethread) {
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

void TcrChunkedResult::waitFinalize() const {
  if (m_inSameThread) return;
  if (m_finalizeSema != nullptr) {
    m_finalizeSema->acquire();
  } else {
    throw NullPointerException(
        "TcrChunkedResult::waitFinalize: null semaphore");
  }
}

bool TcrChunkedResult::exceptionOccurred() const { return (m_ex != nullptr); }

void TcrChunkedResult::setException(std::shared_ptr<Exception> ex) {
  m_ex = ex;
}

std::shared_ptr<Exception>& TcrChunkedResult::getException() { return m_ex; }

void TcrChunkedResult::clearException() { m_ex = nullptr; }

TcrChunkedContext::TcrChunkedContext(const std::vector<uint8_t> chunk,
                                     int32_t len, TcrChunkedResult* result,
                                     uint8_t isLastChunkWithSecurity,
                                     const CacheImpl* cacheImpl)
    : m_chunk(chunk),
      m_len(len),
      m_isLastChunkWithSecurity(isLastChunkWithSecurity),
      m_cache(cacheImpl),
      m_result(result) {}

const uint8_t* TcrChunkedContext::getBytes() const { return m_chunk.data(); }

size_t TcrChunkedContext::getLen() const { return m_chunk.size(); }

void TcrChunkedContext::handleChunk(bool inSameThread) {
  if (m_chunk.empty()) {
    // this is the last chunk for some set of chunks
    m_result->finalize(inSameThread);
  } else if (!m_result->exceptionOccurred()) {
    try {
      m_result->fireHandleChunk(m_chunk.data(), m_len,
                                m_isLastChunkWithSecurity, m_cache);
    } catch (Exception& ex) {
      LOGERROR("HandleChunk error message %s, name = %s", ex.what(),
               ex.getName().c_str());
      m_result->setException(std::make_shared<Exception>(ex));
    } catch (std::exception& stdEx) {
      std::string exMsg("HandleChunk exception:: ");
      exMsg += stdEx.what();
      LOGERROR("HandleChunk exception: %s", stdEx.what());
      auto ex = std::make_shared<UnknownException>(exMsg.c_str());
      m_result->setException(ex);
    } catch (...) {
      std::string exMsg("Unknown exception in ");
      exMsg += Utils::demangleTypeName(typeid(*m_result).name());
      exMsg +=
          "::handleChunk while processing response, possible serialization "
          "mismatch";
      LOGERROR(exMsg.c_str());
      auto ex = std::make_shared<UnknownException>(exMsg.c_str());
      m_result->setException(ex);
    }
  }
}
}  // namespace client
}  // namespace geode
}  // namespace apache
