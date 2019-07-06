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

void setThreadLocalExceptionMessage(const char* exMsg);
const std::string& getThreadLocalExceptionMessage();

[[noreturn]] void GfErrTypeThrowException(const char* str, GfErrType err) {
  std::string func;
  std::string message;
  const auto& exMsg = getThreadLocalExceptionMessage();
  if (!exMsg.empty()) {
    func.append(str);
    func.append(": ");
  }
  switch (err) {
    case GF_NOTCON: {
      message.append(!exMsg.empty() ? exMsg : ": not connected to Geode");
      NotConnectedException ex(message);
      setThreadLocalExceptionMessage(nullptr);
      throw ex;
    }
    case GF_MSG: {
      message.append(!exMsg.empty()
                         ? exMsg
                         : ": message from server could not be handled");

      MessageException ex(message);
      setThreadLocalExceptionMessage(nullptr);
      throw ex;
    }
    case GF_CACHESERVER_EXCEPTION: {
      message.append(!exMsg.empty() ? exMsg
                                    : ": exception happened at cache server");
      CacheServerException ex(message);
      setThreadLocalExceptionMessage(nullptr);
      throw ex;
    }
    case GF_NOTOWN: {
      message.append(!exMsg.empty() ? exMsg : ": not own the lock");
      NotOwnerException ex(message);
      setThreadLocalExceptionMessage(nullptr);
      throw ex;
    }
    case GF_CACHE_REGION_NOT_FOUND: {
      message.append(!exMsg.empty() ? exMsg : ": region not found on server");
      CacheServerException ex(message);
      setThreadLocalExceptionMessage(nullptr);
      throw ex;
    }
    case GF_CACHE_REGION_NOT_GLOBAL: {
      message.append(!exMsg.empty() ? exMsg : ": region not global");
      IllegalStateException ex(message);
      setThreadLocalExceptionMessage(nullptr);
      throw ex;
    }
    case GF_CACHE_ILLEGAL_ARGUMENT_EXCEPTION: {
      message.append(!exMsg.empty() ? exMsg : ": illegal argument");
      IllegalArgumentException ex(message);
      setThreadLocalExceptionMessage(nullptr);
      throw ex;
    }
    case GF_CACHE_ILLEGAL_STATE_EXCEPTION: {
      message.append(!exMsg.empty() ? exMsg : ": illegal State");
      IllegalStateException ex(message);
      setThreadLocalExceptionMessage(nullptr);
      throw ex;
    }
    case GF_CACHE_WRITER_EXCEPTION: {
      message.append(!exMsg.empty() ? exMsg
                                    : ": exception on server during write");
      CacheWriterException ex(message);
      setThreadLocalExceptionMessage(nullptr);
      throw ex;
    }
    case GF_CACHEWRITER_ERROR: {
      message.append(!exMsg.empty() ? exMsg : ": exception in CacheWriter");
      CacheWriterException ex(message);
      setThreadLocalExceptionMessage(nullptr);
      throw ex;
    }
    case GF_CACHE_LOADER_EXCEPTION: {
      message.append(!exMsg.empty() ? exMsg : ": exception in CacheLoader");
      CacheLoaderException ex(message);
      setThreadLocalExceptionMessage(nullptr);
      throw ex;
    }
    case GF_CACHE_LISTENER_EXCEPTION: {
      message.append(!exMsg.empty() ? exMsg : ": exception in CacheListener");
      CacheListenerException ex(message);
      setThreadLocalExceptionMessage(nullptr);
      throw ex;
    }
    case GF_CACHE_REGION_INVALID: {
      message.append(!exMsg.empty() ? exMsg : ": region not valid");
      RegionDestroyedException ex(message);
      setThreadLocalExceptionMessage(nullptr);
      throw ex;
    }
    case GF_CACHE_PROXY: {
      message.append(!exMsg.empty() ? exMsg : ": error in Cache proxy");
      CacheProxyException ex(message);
      setThreadLocalExceptionMessage(nullptr);
      throw ex;
    }
    case GF_IOERR: {
      message.append(!exMsg.empty() ? exMsg
                                    : ": Input/Output error in operation");
      GeodeIOException ex(message);
      setThreadLocalExceptionMessage(nullptr);
      throw ex;
    }
    case GF_ENOENT: {
      message.append(!exMsg.empty() ? exMsg : ": entity does not exist");
      NoSystemException ex(message);
      setThreadLocalExceptionMessage(nullptr);
      throw ex;
    }
    case GF_CACHE_REGION_KEYS_NOT_STRINGS: {
      message.append(
          !exMsg.empty() ? exMsg : ": region entries do not support C access");
      IllegalArgumentException ex(message);
      setThreadLocalExceptionMessage(nullptr);
      throw ex;
    }
    case GF_CACHE_REGION_ENTRY_NOT_BYTES: {
      message.append(!exMsg.empty()
                         ? exMsg
                         : ": existing non-null values was not a byte array");
      IllegalArgumentException ex(message);
      setThreadLocalExceptionMessage(nullptr);
      throw ex;
    }
    case GF_CACHE_TIMEOUT_EXCEPTION: {
      message.append(!exMsg.empty() ? exMsg : ": timed out");
      TimeoutException ex(message);
      setThreadLocalExceptionMessage(nullptr);
      throw ex;
    }
    case GF_TIMEOUT: {
      message.append(!exMsg.empty() ? exMsg : ": timed out");
      TimeoutException ex(message);
      setThreadLocalExceptionMessage(nullptr);
      throw ex;
    }
    case GF_CLIENT_WAIT_TIMEOUT: {
      message.append(!exMsg.empty()
                         ? exMsg
                         : ": timed out, possibly bucket is not available.");
      TimeoutException ex(message);
      setThreadLocalExceptionMessage(nullptr);
      throw ex;
    }
    case GF_ENOMEM: {
      message.append(!exMsg.empty() ? exMsg : ": Out of memory");
      OutOfMemoryException ex(message);
      setThreadLocalExceptionMessage(nullptr);
      throw ex;
    }
    case GF_ERANGE: {
      message.append(!exMsg.empty() ? exMsg : ": Buffer Size Exceeded");
      BufferSizeExceededException ex(message);
      setThreadLocalExceptionMessage(nullptr);
      throw ex;
    }
    case GF_CACHE_LEASE_EXPIRED_EXCEPTION: {
      message.append(!exMsg.empty() ? exMsg : ": lock Lease Expired On you");
      LeaseExpiredException ex(message);
      setThreadLocalExceptionMessage(nullptr);
      throw ex;
    }
    case GF_CACHE_REGION_EXISTS_EXCEPTION: {
      message.append(!exMsg.empty() ? exMsg : ": Named Region Exists");
      RegionExistsException ex(message);
      setThreadLocalExceptionMessage(nullptr);
      throw ex;
    }
    case GF_CACHE_ENTRY_NOT_FOUND: {
      message.append(!exMsg.empty() ? exMsg : ": Entry not found");
      EntryNotFoundException ex(message);
      setThreadLocalExceptionMessage(nullptr);
      throw ex;
    }
    case GF_CACHE_ENTRY_EXISTS: {
      message.append(!exMsg.empty() ? exMsg
                                    : ": Entry already exists in the region");
      EntryExistsException ex(message);
      setThreadLocalExceptionMessage(nullptr);
      throw ex;
    }
    case GF_CACHE_ENTRY_DESTROYED_EXCEPTION: {
      message.append(!exMsg.empty() ? exMsg : ": Entry has been destroyed");
      EntryDestroyedException ex(message);
      setThreadLocalExceptionMessage(nullptr);
      throw ex;
    }
    case GF_CACHE_REGION_DESTROYED_EXCEPTION: {
      message.append(!exMsg.empty() ? exMsg : ": Named Region Destroyed");
      RegionDestroyedException ex(message);
      setThreadLocalExceptionMessage(nullptr);
      throw ex;
    }
    case GF_CACHE_CLOSED_EXCEPTION: {
      message.append(!exMsg.empty() ? exMsg : ": Cache has been closed");
      CacheClosedException ex(message);
      setThreadLocalExceptionMessage(nullptr);
      throw ex;
    }
    case GF_CACHE_STATISTICS_DISABLED_EXCEPTION: {
      message.append(!exMsg.empty()
                         ? exMsg
                         : ": Statistics have been disabled for the region");
      StatisticsDisabledException ex(message);
      setThreadLocalExceptionMessage(nullptr);
      throw ex;
    }
    case GF_CACHE_CONCURRENT_MODIFICATION_EXCEPTION: {
      message.append(!exMsg.empty() ? exMsg
                                    : ": Concurrent modification in the cache");
      ConcurrentModificationException ex(message);
      setThreadLocalExceptionMessage(nullptr);
      throw ex;
    }
    case GF_NOT_AUTHORIZED_EXCEPTION: {
      message.append(!exMsg.empty() ? exMsg : ": unauthorized operation");
      NotAuthorizedException ex(message);
      setThreadLocalExceptionMessage(nullptr);
      throw ex;
    }
    case GF_AUTHENTICATION_FAILED_EXCEPTION: {
      message.append(!exMsg.empty() ? exMsg : ": authentication failed");
      AuthenticationFailedException ex(message);
      setThreadLocalExceptionMessage(nullptr);
      throw ex;
    }
    case GF_AUTHENTICATION_REQUIRED_EXCEPTION: {
      message.append(!exMsg.empty() ? exMsg : ": no authentication provided");
      AuthenticationRequiredException ex(message);
      setThreadLocalExceptionMessage(nullptr);
      throw ex;
    }
    case GF_DUPLICATE_DURABLE_CLIENT: {
      message.append(!exMsg.empty() ? exMsg : ": Duplicate Durable Client Id");
      DuplicateDurableClientException ex(message);
      setThreadLocalExceptionMessage(nullptr);
      throw ex;
    }
    case GF_REMOTE_QUERY_EXCEPTION: {
      message.append(!exMsg.empty() ? exMsg : ": Query failed");
      QueryException ex(message);
      setThreadLocalExceptionMessage(nullptr);
      throw ex;
    }
    case GF_CACHE_LOCATOR_EXCEPTION: {
      NoAvailableLocatorsException cause(
          func + (!exMsg.empty() ? exMsg : ": No locators available"));
      setThreadLocalExceptionMessage(nullptr);
      try {
        throw cause;
      } catch (...) {
        NotConnectedException ex(
            func + (!exMsg.empty() ? exMsg : ": No locators available"));
        std::throw_with_nested(ex);
      }
    }
    case GF_ALL_CONNECTIONS_IN_USE_EXCEPTION: {
      message.append(!exMsg.empty() ? exMsg : ": All connections are in use");
      AllConnectionsInUseException ex(message);
      setThreadLocalExceptionMessage(nullptr);
      throw ex;
    }
    case GF_FUNCTION_EXCEPTION: {
      message.append(!exMsg.empty() ? exMsg : ": Function execution failed");
      FunctionExecutionException ex(message);
      setThreadLocalExceptionMessage(nullptr);
      throw ex;
    }
    case GF_DISKFULL: {
      message.append(!exMsg.empty() ? exMsg : ": Disk full");
      DiskFailureException ex(message);
      setThreadLocalExceptionMessage(nullptr);
      throw ex;
    }
    case GF_ROLLBACK_EXCEPTION: {
      message.append(!exMsg.empty() ? exMsg : ": Transaction rolled back");
      RollbackException ex(message);
      setThreadLocalExceptionMessage(nullptr);
      throw ex;
    }
    case GF_COMMIT_CONFLICT_EXCEPTION: {
      message.append(!exMsg.empty() ? exMsg : ": Commit conflict exception");
      CommitConflictException ex(message);
      setThreadLocalExceptionMessage(nullptr);
      throw ex;
    }
    case GF_TRANSACTION_DATA_REBALANCED_EXCEPTION: {
      message.append(
          !exMsg.empty() ? exMsg : ": Transaction data rebalanced exception");
      TransactionDataRebalancedException ex(message);
      setThreadLocalExceptionMessage(nullptr);
      throw ex;
    }
    case GF_TRANSACTION_DATA_NODE_HAS_DEPARTED_EXCEPTION: {
      message.append(!exMsg.empty()
                         ? exMsg
                         : ": Transaction data node has departed exception");
      TransactionDataNodeHasDepartedException ex(message);
      setThreadLocalExceptionMessage(nullptr);
      throw ex;
    }
    case GF_PUTALL_PARTIAL_RESULT_EXCEPTION: {
      message.append(!exMsg.empty() ? exMsg : ": PutAll Partial exception");
      PutAllPartialResultException ex(message);
      setThreadLocalExceptionMessage(nullptr);
      throw ex;
    }
    default: {
      LOGINFO("error code: %d", err);
      std::string message = func;
      if (exMsg.empty()) {
        message.append("Unknown error code ").append(std::to_string(err));
      } else {
        message.append(exMsg);
      }
      UnknownException ex(message);
      setThreadLocalExceptionMessage(nullptr);
      throw ex;
    }
  }
}
}  // namespace client
}  // namespace geode
}  // namespace apache
