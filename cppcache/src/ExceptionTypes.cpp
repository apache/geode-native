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

#include <cstdio>
#include <sstream>
#include <string>

#include <geode/ExceptionTypes.hpp>

#include "ErrType.hpp"
#include "TSSTXStateWrapper.hpp"
#include "TXState.hpp"
#include "util/Log.hpp"

namespace apache {
namespace geode {
namespace client {

void setTSSExceptionMessage(const char* exMsg);
const char* getTSSExceptionMessage();

[[noreturn]] void GfErrTypeThrowException(const char* str, GfErrType err) {
  std::string func;
  std::string message;
  const char* exMsg = getTSSExceptionMessage();
  if (exMsg != nullptr && exMsg[0] == '\0') {
    exMsg = nullptr;
  } else {
    func.append(str);
    func.append(": ");
    str = func.c_str();
  }
  switch (err) {
    case GF_NOTCON: {
      message.append(exMsg != nullptr ? exMsg : ": not connected to Geode");
      NotConnectedException ex(message);
      setTSSExceptionMessage(nullptr);
      throw ex;
    }
    case GF_MSG: {
      message.append(exMsg != nullptr
                         ? exMsg
                         : ": message from server could not be handled");

      MessageException ex(message);
      setTSSExceptionMessage(nullptr);
      throw ex;
    }
    case GF_CACHESERVER_EXCEPTION: {
      message.append(exMsg != nullptr ? exMsg
                                      : ": exception happened at cache server");
      CacheServerException ex(message);
      setTSSExceptionMessage(nullptr);
      throw ex;
    }
    case GF_NOTOWN: {
      message.append(exMsg != nullptr ? exMsg : ": not own the lock");
      NotOwnerException ex(message);
      setTSSExceptionMessage(nullptr);
      throw ex;
    }
    case GF_CACHE_REGION_NOT_FOUND: {
      message.append(exMsg != nullptr ? exMsg : ": region not found on server");
      CacheServerException ex(message);
      setTSSExceptionMessage(nullptr);
      throw ex;
    }
    case GF_CACHE_REGION_NOT_GLOBAL: {
      message.append(exMsg != nullptr ? exMsg : ": region not global");
      IllegalStateException ex(message);
      setTSSExceptionMessage(nullptr);
      throw ex;
    }
    case GF_CACHE_ILLEGAL_ARGUMENT_EXCEPTION: {
      message.append(exMsg != nullptr ? exMsg : ": illegal argument");
      IllegalArgumentException ex(message);
      setTSSExceptionMessage(nullptr);
      throw ex;
    }
    case GF_CACHE_ILLEGAL_STATE_EXCEPTION: {
      message.append(exMsg != nullptr ? exMsg : ": illegal State");
      IllegalStateException ex(message);
      setTSSExceptionMessage(nullptr);
      throw ex;
    }
    case GF_CACHE_WRITER_EXCEPTION: {
      message.append(exMsg != nullptr ? exMsg
                                      : ": exception on server during write");
      CacheWriterException ex(message);
      setTSSExceptionMessage(nullptr);
      throw ex;
    }
    case GF_CACHEWRITER_ERROR: {
      message.append(exMsg != nullptr ? exMsg : ": exception in CacheWriter");
      CacheWriterException ex(message);
      setTSSExceptionMessage(nullptr);
      throw ex;
    }
    case GF_CACHE_LOADER_EXCEPTION: {
      message.append(exMsg != nullptr ? exMsg : ": exception in CacheLoader");
      CacheLoaderException ex(message);
      setTSSExceptionMessage(nullptr);
      throw ex;
    }
    case GF_CACHE_LISTENER_EXCEPTION: {
      message.append(exMsg != nullptr ? exMsg : ": exception in CacheListener");
      CacheListenerException ex(message);
      setTSSExceptionMessage(nullptr);
      throw ex;
    }
    case GF_CACHE_REGION_INVALID: {
      message.append(exMsg != nullptr ? exMsg : ": region not valid");
      RegionDestroyedException ex(message);
      setTSSExceptionMessage(nullptr);
      throw ex;
    }
    case GF_CACHE_PROXY: {
      message.append(exMsg != nullptr ? exMsg : ": error in Cache proxy");
      CacheProxyException ex(message);
      setTSSExceptionMessage(nullptr);
      throw ex;
    }
    case GF_IOERR: {
      message.append(exMsg != nullptr ? exMsg
                                      : ": Input/Output error in operation");
      GeodeIOException ex(message);
      setTSSExceptionMessage(nullptr);
      throw ex;
    }
    case GF_ENOENT: {
      message.append(exMsg != nullptr ? exMsg : ": entity does not exist");
      NoSystemException ex(message);
      setTSSExceptionMessage(nullptr);
      throw ex;
    }
    case GF_CACHE_REGION_KEYS_NOT_STRINGS: {
      message.append(exMsg != nullptr
                         ? exMsg
                         : ": region entries do not support C access");
      IllegalArgumentException ex(message);
      setTSSExceptionMessage(nullptr);
      throw ex;
    }
    case GF_CACHE_REGION_ENTRY_NOT_BYTES: {
      message.append(exMsg != nullptr
                         ? exMsg
                         : ": existing non-null values was not a byte array");
      IllegalArgumentException ex(message);
      setTSSExceptionMessage(nullptr);
      throw ex;
    }
    case GF_CACHE_TIMEOUT_EXCEPTION: {
      message.append(exMsg != nullptr ? exMsg : ": timed out");
      TimeoutException ex(message);
      setTSSExceptionMessage(nullptr);
      throw ex;
    }
    case GF_TIMOUT: {
      message.append(exMsg != nullptr ? exMsg : ": timed out");
      TimeoutException ex(message);
      setTSSExceptionMessage(nullptr);
      throw ex;
    }
    case GF_CLIENT_WAIT_TIMEOUT: {
      message.append(exMsg != nullptr
                         ? exMsg
                         : ": timed out, possibly bucket is not available.");
      TimeoutException ex(message);
      setTSSExceptionMessage(nullptr);
      throw ex;
    }
    case GF_ENOMEM: {
      message.append(exMsg != nullptr ? exMsg : ": Out of memory");
      OutOfMemoryException ex(message);
      setTSSExceptionMessage(nullptr);
      throw ex;
    }
    case GF_ERANGE: {
      message.append(exMsg != nullptr ? exMsg : ": Buffer Size Exceeded");
      BufferSizeExceededException ex(message);
      setTSSExceptionMessage(nullptr);
      throw ex;
    }
    case GF_CACHE_LEASE_EXPIRED_EXCEPTION: {
      message.append(exMsg != nullptr ? exMsg : ": lock Lease Expired On you");
      LeaseExpiredException ex(message);
      setTSSExceptionMessage(nullptr);
      throw ex;
    }
    case GF_CACHE_REGION_EXISTS_EXCEPTION: {
      message.append(exMsg != nullptr ? exMsg : ": Named Region Exists");
      RegionExistsException ex(message);
      setTSSExceptionMessage(nullptr);
      throw ex;
    }
    case GF_CACHE_ENTRY_NOT_FOUND: {
      message.append(exMsg != nullptr ? exMsg : ": Entry not found");
      EntryNotFoundException ex(message);
      setTSSExceptionMessage(nullptr);
      throw ex;
    }
    case GF_CACHE_ENTRY_EXISTS: {
      message.append(exMsg != nullptr ? exMsg
                                      : ": Entry already exists in the region");
      EntryExistsException ex(message);
      setTSSExceptionMessage(nullptr);
      throw ex;
    }
    case GF_CACHE_ENTRY_DESTROYED_EXCEPTION: {
      message.append(exMsg != nullptr ? exMsg : ": Entry has been destroyed");
      EntryDestroyedException ex(message);
      setTSSExceptionMessage(nullptr);
      throw ex;
    }
    case GF_CACHE_REGION_DESTROYED_EXCEPTION: {
      message.append(exMsg != nullptr ? exMsg : ": Named Region Destroyed");
      RegionDestroyedException ex(message);
      setTSSExceptionMessage(nullptr);
      throw ex;
    }
    case GF_CACHE_CLOSED_EXCEPTION: {
      message.append(exMsg != nullptr ? exMsg : ": Cache has been closed");
      CacheClosedException ex(message);
      setTSSExceptionMessage(nullptr);
      throw ex;
    }
    case GF_CACHE_STATISTICS_DISABLED_EXCEPTION: {
      message.append(exMsg != nullptr
                         ? exMsg
                         : ": Statistics have been disabled for the region");
      StatisticsDisabledException ex(message);
      setTSSExceptionMessage(nullptr);
      throw ex;
    }
    case GF_CACHE_CONCURRENT_MODIFICATION_EXCEPTION: {
      message.append(
          exMsg != nullptr ? exMsg : ": Concurrent modification in the cache");
      ConcurrentModificationException ex(message);
      setTSSExceptionMessage(nullptr);
      throw ex;
    }
    case GF_NOT_AUTHORIZED_EXCEPTION: {
      message.append(exMsg != nullptr ? exMsg : ": unauthorized operation");
      NotAuthorizedException ex(message);
      setTSSExceptionMessage(nullptr);
      throw ex;
    }
    case GF_AUTHENTICATION_FAILED_EXCEPTION: {
      message.append(exMsg != nullptr ? exMsg : ": authentication failed");
      AuthenticationFailedException ex(message);
      setTSSExceptionMessage(nullptr);
      throw ex;
    }
    case GF_AUTHENTICATION_REQUIRED_EXCEPTION: {
      message.append(exMsg != nullptr ? exMsg : ": no authentication provided");
      AuthenticationRequiredException ex(message);
      setTSSExceptionMessage(nullptr);
      throw ex;
    }
    case GF_DUPLICATE_DURABLE_CLIENT: {
      message.append(exMsg != nullptr ? exMsg
                                      : ": Duplicate Durable Client Id");
      DuplicateDurableClientException ex(message);
      setTSSExceptionMessage(nullptr);
      throw ex;
    }
    case GF_REMOTE_QUERY_EXCEPTION: {
      message.append(exMsg != nullptr ? exMsg : ": Query failed");
      QueryException ex(message);
      setTSSExceptionMessage(nullptr);
      throw ex;
    }
    case GF_CACHE_LOCATOR_EXCEPTION: {
      NoAvailableLocatorsException cause(
          std::string(str) +
          (exMsg != nullptr ? exMsg : ": No locators available"));
      setTSSExceptionMessage(nullptr);
      try {
        throw cause;
      } catch (...) {
        NotConnectedException ex(
            std::string(str) +
            (exMsg != nullptr ? exMsg : ": No locators available"));
        std::throw_with_nested(ex);
      }
    }
    case GF_ALL_CONNECTIONS_IN_USE_EXCEPTION: {
      message.append(exMsg != nullptr ? exMsg : ": All connections are in use");
      AllConnectionsInUseException ex(message);
      setTSSExceptionMessage(nullptr);
      throw ex;
    }
    case GF_FUNCTION_EXCEPTION: {
      message.append(exMsg != nullptr ? exMsg : ": Function execution failed");
      FunctionExecutionException ex(message);
      setTSSExceptionMessage(nullptr);
      throw ex;
    }
    case GF_DISKFULL: {
      message.append(exMsg != nullptr ? exMsg : ": Disk full");
      DiskFailureException ex(message);
      setTSSExceptionMessage(nullptr);
      throw ex;
    }
    case GF_ROLLBACK_EXCEPTION: {
      message.append(exMsg != nullptr ? exMsg : ": Transaction rolled back");
      RollbackException ex(message);
      setTSSExceptionMessage(nullptr);
      throw ex;
    }
    case GF_COMMIT_CONFLICT_EXCEPTION: {
      message.append(exMsg != nullptr ? exMsg : ": Commit conflict exception");
      CommitConflictException ex(message);
      setTSSExceptionMessage(nullptr);
      throw ex;
    }
    case GF_TRANSACTION_DATA_REBALANCED_EXCEPTION: {
      message.append(
          exMsg != nullptr ? exMsg : ": Transaction data rebalanced exception");
      TransactionDataRebalancedException ex(message);
      setTSSExceptionMessage(nullptr);
      throw ex;
    }
    case GF_TRANSACTION_DATA_NODE_HAS_DEPARTED_EXCEPTION: {
      message.append(exMsg != nullptr
                         ? exMsg
                         : ": Transaction data node has departed exception");
      TransactionDataNodeHasDepartedException ex(message);
      setTSSExceptionMessage(nullptr);
      throw ex;
    }
    case GF_PUTALL_PARTIAL_RESULT_EXCEPTION: {
      message.append(exMsg != nullptr ? exMsg : ": PutAll Partial exception");
      PutAllPartialResultException ex(message);
      setTSSExceptionMessage(nullptr);
      throw ex;
    }
    default: {
      char buf[64];
      LOGINFO("error code: %d", err);
      if (exMsg == nullptr) {
        ACE_OS::snprintf(buf, 64, "Unknown error code[0x%X]", err);
        exMsg = buf;
      }
      std::string message = std::string(str) + exMsg;
      UnknownException ex(message);
      setTSSExceptionMessage(nullptr);
      throw ex;
    }
  }
}
}  // namespace client
}  // namespace geode
}  // namespace apache
