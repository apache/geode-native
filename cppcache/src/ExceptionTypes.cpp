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
#include <string>
#include <sstream>
#include <geode/ExceptionTypes.hpp>
#include <TXState.hpp>
#include <TSSTXStateWrapper.hpp>

namespace apache {
namespace geode {
namespace client {
void setTSSExceptionMessage(const char* exMsg);
const char* getTSSExceptionMessage();

void GfErrTypeThrowException(const char* str, GfErrType err) {
  std::string func;
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
      std::stringstream ss;
      ss << str << (exMsg != nullptr ? exMsg : ": not connected to Geode");
      NotConnectedException ex(ss.str());
      setTSSExceptionMessage(nullptr);
      throw ex;
    }
    case GF_MSG: {
      std::stringstream ss;
      ss << str << (exMsg != nullptr ? exMsg
                    : ": message from server could not be handled");

      MessageException ex(ss.str());
      setTSSExceptionMessage(nullptr);
      throw ex;
    }
    case GF_CACHESERVER_EXCEPTION: {
      std::stringstream ss;
      ss << str << (exMsg != nullptr ? exMsg : ": exception happened at cache server");
      CacheServerException ex(ss.str());
      setTSSExceptionMessage(nullptr);
      throw ex;
    }
    case GF_NOTOWN: {
      std::stringstream ss;
      ss << str << (exMsg != nullptr ? exMsg : ": not own the lock");
      NotOwnerException ex(ss.str());
      setTSSExceptionMessage(nullptr);
      throw ex;
    }
    case GF_CACHE_REGION_NOT_FOUND: {
      std::stringstream ss;
      ss << str << (exMsg != nullptr ? exMsg : ": region not found on server");
      CacheServerException ex(ss.str());
      setTSSExceptionMessage(nullptr);
      throw ex;
    }
    case GF_CACHE_REGION_NOT_GLOBAL: {
      std::stringstream ss;
      ss << str << (exMsg != nullptr ? exMsg : ": region not global");
      IllegalStateException ex(ss.str());
      setTSSExceptionMessage(nullptr);
      throw ex;
    }
    case GF_CACHE_ILLEGAL_ARGUMENT_EXCEPTION: {
      std::stringstream ss;
      ss << str << (exMsg != nullptr ? exMsg : ": illegal argument");
      IllegalArgumentException ex(ss.str());
      setTSSExceptionMessage(nullptr);
      throw ex;
    }
    case GF_CACHE_ILLEGAL_STATE_EXCEPTION: {
      std::stringstream ss;
      ss << str << (exMsg != nullptr ? exMsg : ": illegal State");
      IllegalStateException ex(ss.str());
      setTSSExceptionMessage(nullptr);
      throw ex;
    }
    case GF_CACHE_WRITER_EXCEPTION: {
      std::stringstream ss;
      ss << str << (exMsg != nullptr ? exMsg : ": exception on server during write");
      CacheWriterException ex(ss.str());
      setTSSExceptionMessage(nullptr);
      throw ex;
    }
    case GF_CACHEWRITER_ERROR: {
      std::stringstream ss;
      ss << str << (exMsg != nullptr ? exMsg : ": exception in CacheWriter");
      CacheWriterException ex(ss.str());
      setTSSExceptionMessage(nullptr);
      throw ex;
    }
    case GF_CACHE_LOADER_EXCEPTION: {
      std::stringstream ss;
      ss << str << (exMsg != nullptr ? exMsg : ": exception in CacheLoader");
      CacheLoaderException ex(ss.str());
      setTSSExceptionMessage(nullptr);
      throw ex;
    }
    case GF_CACHE_LISTENER_EXCEPTION: {
      std::stringstream ss;
      ss << str << (exMsg != nullptr ? exMsg : ": exception in CacheListener");
      CacheListenerException ex(ss.str());
      setTSSExceptionMessage(nullptr);
      throw ex;
    }
    case GF_CACHE_REGION_INVALID: {
      std::stringstream ss;
      ss << str << (exMsg != nullptr ? exMsg : ": region not valid");
      RegionDestroyedException ex(ss.str());
      setTSSExceptionMessage(nullptr);
      throw ex;
    }
    case GF_CACHE_PROXY: {
      std::stringstream ss;
      ss << str <<  (exMsg != nullptr ? exMsg : ": error in Cache proxy");
      CacheProxyException ex(ss.str());
      setTSSExceptionMessage(nullptr);
      throw ex;
    }
    case GF_IOERR: {
      std::stringstream ss;
      ss << str << (exMsg != nullptr ? exMsg : ": Input/Output error in operation");
      GeodeIOException ex(ss.str());
      setTSSExceptionMessage(nullptr);
      throw ex;
    }
    case GF_ENOENT: {
      std::stringstream ss;
      ss << str << (exMsg != nullptr ? exMsg : ": entity does not exist");
      NoSystemException ex(ss.str());
      setTSSExceptionMessage(nullptr);
      throw ex;
    }
    case GF_CACHE_REGION_KEYS_NOT_STRINGS: {
      std::stringstream ss;
      ss << str << (exMsg != nullptr ? exMsg
                    : ": region entries do not support C access");
      IllegalArgumentException ex(ss.str());
      setTSSExceptionMessage(nullptr);
      throw ex;
    }
    case GF_CACHE_REGION_ENTRY_NOT_BYTES: {
      std::stringstream ss;
      ss << str << (exMsg != nullptr
                    ? exMsg
                    : ": existing non-null values was not a byte array");
      IllegalArgumentException ex(ss.str());
      setTSSExceptionMessage(nullptr);
      throw ex;
    }
    case GF_CACHE_TIMEOUT_EXCEPTION: {
      std::stringstream ss;
      ss << str << (exMsg != nullptr ? exMsg : ": timed out");
      TimeoutException ex(ss.str());
      setTSSExceptionMessage(nullptr);
      throw ex;
    }
    case GF_TIMOUT: {
      std::stringstream ss;
      ss << str << (exMsg != nullptr ? exMsg : ": timed out");
      TimeoutException ex(ss.str());
      setTSSExceptionMessage(nullptr);
      throw ex;
    }
    case GF_CLIENT_WAIT_TIMEOUT: {
      std::stringstream ss;
      ss << str << (exMsg != nullptr
                    ? exMsg
                    : ": timed out, possibly bucket is not available.");
      TimeoutException ex(ss.str());
      setTSSExceptionMessage(nullptr);
      throw ex;
    }
    case GF_ENOMEM: {
      std::stringstream ss;
      ss << str << (exMsg != nullptr ? exMsg : ": Out of memory");
      OutOfMemoryException ex(ss.str());
      setTSSExceptionMessage(nullptr);
      throw ex;
    }
    case GF_ERANGE: {
      std::stringstream ss;
      ss << str << (exMsg != nullptr ? exMsg : ": Buffer Size Exceeded");
      BufferSizeExceededException ex(ss.str());
      setTSSExceptionMessage(nullptr);
      throw ex;
    }
    case GF_CACHE_LEASE_EXPIRED_EXCEPTION: {
      std::stringstream ss;
      ss << str << (exMsg != nullptr ? exMsg : ": lock Lease Expired On you");
      LeaseExpiredException ex(ss.str());
      setTSSExceptionMessage(nullptr);
      throw ex;
    }
    case GF_CACHE_REGION_EXISTS_EXCEPTION: {
      std::stringstream ss;
      ss << str << (exMsg != nullptr ? exMsg : ": Named Region Exists");
      RegionExistsException ex(ss.str());
      setTSSExceptionMessage(nullptr);
      throw ex;
    }
    case GF_CACHE_ENTRY_NOT_FOUND: {
      std::stringstream ss;
      ss << str << (exMsg != nullptr ? exMsg : ": Entry not found");
      EntryNotFoundException ex(ss.str());
      setTSSExceptionMessage(nullptr);
      throw ex;
    }
    case GF_CACHE_ENTRY_EXISTS: {
      std::stringstream ss;
      ss << str
         << (exMsg != nullptr ? exMsg : ": Entry already exists in the region");
      EntryExistsException ex(ss.str());
      setTSSExceptionMessage(nullptr);
      throw ex;
    }
    case GF_CACHE_ENTRY_DESTROYED_EXCEPTION: {
      std::stringstream ss;
      ss << str << (exMsg != nullptr ? exMsg : ": Entry has been destroyed");
      EntryDestroyedException ex(ss.str());
      setTSSExceptionMessage(nullptr);
      throw ex;
    }
    case GF_CACHE_REGION_DESTROYED_EXCEPTION: {
      std::stringstream ss;
      ss << str << (exMsg != nullptr ? exMsg : ": Named Region Destroyed");
      RegionDestroyedException ex(ss.str());
      setTSSExceptionMessage(nullptr);
      throw ex;
    }
    case GF_CACHE_CLOSED_EXCEPTION: {
      std::stringstream ss;
      ss << str << (exMsg != nullptr ? exMsg : ": Cache has been closed");
      CacheClosedException ex(ss.str());
      setTSSExceptionMessage(nullptr);
      throw ex;
    }
    case GF_CACHE_STATISTICS_DISABLED_EXCEPTION: {
      std::stringstream ss;
      ss << str << (exMsg != nullptr
                    ? exMsg
                    : ": Statistics have been disabled for the region");
      StatisticsDisabledException ex(ss.str());
      setTSSExceptionMessage(nullptr);
      throw ex;
    }
    case GF_CACHE_CONCURRENT_MODIFICATION_EXCEPTION: {
      std::stringstream ss;
      ss << str << (exMsg != nullptr ? exMsg
                    : ": Concurrent modification in the cache");
      ConcurrentModificationException ex(ss.str());
      setTSSExceptionMessage(nullptr);
      throw ex;
    }
    case GF_NOT_AUTHORIZED_EXCEPTION: {
      std::stringstream ss;
      ss << str << (exMsg != nullptr ? exMsg : ": unauthorized operation");
      NotAuthorizedException ex(ss.str());
      setTSSExceptionMessage(nullptr);
      throw ex;
    }
    case GF_AUTHENTICATION_FAILED_EXCEPTION: {
      std::stringstream ss;
      ss << str << (exMsg != nullptr ? exMsg : ": authentication failed");
      AuthenticationFailedException ex(ss.str());
      setTSSExceptionMessage(nullptr);
      throw ex;
    }
    case GF_AUTHENTICATION_REQUIRED_EXCEPTION: {
      std::stringstream ss;
      ss << str <<  (exMsg != nullptr ? exMsg : ": no authentication provided");
      AuthenticationRequiredException ex(ss.str());
      setTSSExceptionMessage(nullptr);
      throw ex;
    }
    case GF_DUPLICATE_DURABLE_CLIENT: {
      std::stringstream ss;
      ss << str << (exMsg != nullptr ? exMsg : ": Duplicate Durable Client Id");
      DuplicateDurableClientException ex(ss.str());
      setTSSExceptionMessage(nullptr);
      throw ex;
    }
    case GF_REMOTE_QUERY_EXCEPTION: {
      std::stringstream ss;
      ss << str << (exMsg != nullptr ? exMsg : ": Query failed");
      QueryException ex(ss.str());
      setTSSExceptionMessage(nullptr);
      throw ex;
    }
    case GF_CACHE_LOCATOR_EXCEPTION: {
      std::stringstream ss;
      ss << str << (exMsg != nullptr ? exMsg : ": No locators available");
      NotConnectedException ex(ss.str());
      setTSSExceptionMessage(nullptr);
      throw ex;
    }
    case GF_ALL_CONNECTIONS_IN_USE_EXCEPTION: {
      std::stringstream ss;
      ss << str << (exMsg != nullptr ? exMsg : ": All connections are in use");
      AllConnectionsInUseException ex(ss.str());
      setTSSExceptionMessage(nullptr);
      throw ex;
    }
    case GF_FUNCTION_EXCEPTION: {
      std::stringstream ss;
      ss << str << (exMsg != nullptr ? exMsg : ": Function execution failed");
      FunctionExecutionException ex(ss.str());
      setTSSExceptionMessage(nullptr);
      throw ex;
    }
    case GF_DISKFULL: {
      std::stringstream ss;
      ss << str << (exMsg != nullptr ? exMsg : ": Disk full");
      DiskFailureException ex(ss.str());
      setTSSExceptionMessage(nullptr);
      throw ex;
    }
    case GF_ROLLBACK_EXCEPTION: {
      std::stringstream ss;
      ss << str << (exMsg != nullptr ? exMsg : ": Transaction rolled back");
      RollbackException ex(ss.str());
      setTSSExceptionMessage(nullptr);
      throw ex;
    }
    case GF_COMMIT_CONFLICT_EXCEPTION: {
      std::stringstream ss;
      ss << str << (exMsg != nullptr ? exMsg : ": Commit conflict exception");
      CommitConflictException ex(ss.str());
      setTSSExceptionMessage(nullptr);
      throw ex;
    }
    case GF_TRANSACTION_DATA_REBALANCED_EXCEPTION: {
      std::stringstream ss;
      ss << str << (exMsg != nullptr ? exMsg
                    : ": Transaction data rebalanced exception");
      TransactionDataRebalancedException ex(ss.str());
      setTSSExceptionMessage(nullptr);
      throw ex;
    }
    case GF_TRANSACTION_DATA_NODE_HAS_DEPARTED_EXCEPTION: {
      std::stringstream ss;
      ss << str << (exMsg != nullptr
                    ? exMsg
                    : ": Transaction data node has departed exception");
      TransactionDataNodeHasDepartedException ex(ss.str());
      setTSSExceptionMessage(nullptr);
      throw ex;
    }
    case GF_PUTALL_PARTIAL_RESULT_EXCEPTION: {
      std::stringstream ss;
      ss << str << (exMsg != nullptr ? exMsg : ": PutAll Partial exception");
      PutAllPartialResultException ex(ss.str());
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
