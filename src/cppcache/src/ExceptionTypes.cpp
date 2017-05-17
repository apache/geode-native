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
      NotConnectedException ex(
          str, (exMsg != nullptr ? exMsg : ": not connected to Geode"));
      setTSSExceptionMessage(nullptr);
      throw ex;
    }
    case GF_MSG: {
      MessageException ex(
          str,
          (exMsg != nullptr ? exMsg
                            : ": message from server could not be handled"));
      setTSSExceptionMessage(nullptr);
      throw ex;
    }
    case GF_CACHESERVER_EXCEPTION: {
      CacheServerException ex(
          str,
          (exMsg != nullptr ? exMsg : ": exception happened at cache server"));
      setTSSExceptionMessage(nullptr);
      throw ex;
    }
    case GF_NOTOWN: {
      NotOwnerException ex(str,
                           (exMsg != nullptr ? exMsg : ": not own the lock"));
      setTSSExceptionMessage(nullptr);
      throw ex;
    }
    case GF_CACHE_REGION_NOT_FOUND: {
      CacheServerException ex(
          str, (exMsg != nullptr ? exMsg : ": region not found on server"));
      setTSSExceptionMessage(nullptr);
      throw ex;
    }
    case GF_CACHE_REGION_NOT_GLOBAL: {
      IllegalStateException ex(
          str, (exMsg != nullptr ? exMsg : ": region not global"));
      setTSSExceptionMessage(nullptr);
      throw ex;
    }
    case GF_CACHE_ILLEGAL_ARGUMENT_EXCEPTION: {
      IllegalArgumentException ex(
          str, (exMsg != nullptr ? exMsg : ": illegal argument"));
      setTSSExceptionMessage(nullptr);
      throw ex;
    }
    case GF_CACHE_ILLEGAL_STATE_EXCEPTION: {
      IllegalStateException ex(str,
                               (exMsg != nullptr ? exMsg : ": illegal State"));
      setTSSExceptionMessage(nullptr);
      throw ex;
    }
    case GF_CACHE_WRITER_EXCEPTION: {
      CacheWriterException ex(
          str,
          (exMsg != nullptr ? exMsg : ": exception on server during write"));
      setTSSExceptionMessage(nullptr);
      throw ex;
    }
    case GF_CACHEWRITER_ERROR: {
      CacheWriterException ex(
          str, (exMsg != nullptr ? exMsg : ": exception in CacheWriter"));
      setTSSExceptionMessage(nullptr);
      throw ex;
    }
    case GF_CACHE_LOADER_EXCEPTION: {
      CacheLoaderException ex(
          str, (exMsg != nullptr ? exMsg : ": exception in CacheLoader"));
      setTSSExceptionMessage(nullptr);
      throw ex;
    }
    case GF_CACHE_LISTENER_EXCEPTION: {
      CacheListenerException ex(
          str, (exMsg != nullptr ? exMsg : ": exception in CacheListener"));
      setTSSExceptionMessage(nullptr);
      throw ex;
    }
    case GF_CACHE_REGION_INVALID: {
      RegionDestroyedException ex(
          str, (exMsg != nullptr ? exMsg : ": region not valid"));
      setTSSExceptionMessage(nullptr);
      throw ex;
    }
    case GF_CACHE_PROXY: {
      CacheProxyException ex(
          str, (exMsg != nullptr ? exMsg : ": error in Cache proxy"));
      setTSSExceptionMessage(nullptr);
      throw ex;
    }
    case GF_IOERR: {
      GeodeIOException ex(
          str,
          (exMsg != nullptr ? exMsg : ": Input/Output error in operation"));
      setTSSExceptionMessage(nullptr);
      throw ex;
    }
    case GF_ENOENT: {
      NoSystemException ex(
          str, (exMsg != nullptr ? exMsg : ": entity does not exist"));
      setTSSExceptionMessage(nullptr);
      throw ex;
    }
    case GF_CACHE_REGION_KEYS_NOT_STRINGS: {
      IllegalArgumentException ex(
          str, (exMsg != nullptr ? exMsg
                                 : ": region entries do not support C access"));
      setTSSExceptionMessage(nullptr);
      throw ex;
    }
    case GF_CACHE_REGION_ENTRY_NOT_BYTES: {
      IllegalArgumentException ex(
          str, (exMsg != nullptr
                    ? exMsg
                    : ": existing non-null values was not a byte array"));
      setTSSExceptionMessage(nullptr);
      throw ex;
    }
    case GF_CACHE_TIMEOUT_EXCEPTION: {
      TimeoutException ex(str, (exMsg != nullptr ? exMsg : ": timed out"));
      setTSSExceptionMessage(nullptr);
      throw ex;
    }
    case GF_TIMOUT: {
      TimeoutException ex(str, (exMsg != nullptr ? exMsg : ": timed out"));
      setTSSExceptionMessage(nullptr);
      throw ex;
    }
    case GF_CLIENT_WAIT_TIMEOUT: {
      TimeoutException ex(
          str, (exMsg != nullptr
                    ? exMsg
                    : ": timed out, possibly bucket is not available."));
      setTSSExceptionMessage(nullptr);
      throw ex;
    }
    case GF_ENOMEM: {
      OutOfMemoryException ex(str,
                              (exMsg != nullptr ? exMsg : ": Out of memory"));
      setTSSExceptionMessage(nullptr);
      throw ex;
    }
    case GF_ERANGE: {
      BufferSizeExceededException ex(
          str, (exMsg != nullptr ? exMsg : ": Buffer Size Exceeded"));
      setTSSExceptionMessage(nullptr);
      throw ex;
    }
    case GF_CACHE_LEASE_EXPIRED_EXCEPTION: {
      LeaseExpiredException ex(
          str, (exMsg != nullptr ? exMsg : ": lock Lease Expired On you"));
      setTSSExceptionMessage(nullptr);
      throw ex;
    }
    case GF_CACHE_REGION_EXISTS_EXCEPTION: {
      RegionExistsException ex(
          str, (exMsg != nullptr ? exMsg : ": Named Region Exists"));
      setTSSExceptionMessage(nullptr);
      throw ex;
    }
    case GF_CACHE_ENTRY_NOT_FOUND: {
      EntryNotFoundException ex(
          str, (exMsg != nullptr ? exMsg : ": Entry not found"));
      setTSSExceptionMessage(nullptr);
      throw ex;
    }
    case GF_CACHE_ENTRY_EXISTS: {
      EntryExistsException ex(
          str,
          (exMsg != nullptr ? exMsg : ": Entry already exists in the region"));
      setTSSExceptionMessage(nullptr);
      throw ex;
    }
    case GF_CACHE_ENTRY_DESTROYED_EXCEPTION: {
      EntryDestroyedException ex(
          str, (exMsg != nullptr ? exMsg : ": Entry has been destroyed"));
      setTSSExceptionMessage(nullptr);
      throw ex;
    }
    case GF_CACHE_REGION_DESTROYED_EXCEPTION: {
      RegionDestroyedException ex(
          str, (exMsg != nullptr ? exMsg : ": Named Region Destroyed"));
      setTSSExceptionMessage(nullptr);
      throw ex;
    }
    case GF_CACHE_CLOSED_EXCEPTION: {
      CacheClosedException ex(
          str, (exMsg != nullptr ? exMsg : ": Cache has been closed"));
      setTSSExceptionMessage(nullptr);
      throw ex;
    }
    case GF_CACHE_STATISTICS_DISABLED_EXCEPTION: {
      StatisticsDisabledException ex(
          str, (exMsg != nullptr
                    ? exMsg
                    : ": Statistics have been disabled for the region"));
      setTSSExceptionMessage(nullptr);
      throw ex;
    }
    case GF_CACHE_CONCURRENT_MODIFICATION_EXCEPTION: {
      ConcurrentModificationException ex(
          str, (exMsg != nullptr ? exMsg
                                 : ": Concurrent modification in the cache"));
      setTSSExceptionMessage(nullptr);
      throw ex;
    }
    case GF_NOT_AUTHORIZED_EXCEPTION: {
      NotAuthorizedException ex(
          str, (exMsg != nullptr ? exMsg : ": unauthorized operation"));
      setTSSExceptionMessage(nullptr);
      throw ex;
    }
    case GF_AUTHENTICATION_FAILED_EXCEPTION: {
      AuthenticationFailedException ex(
          str, (exMsg != nullptr ? exMsg : ": authentication failed"));
      setTSSExceptionMessage(nullptr);
      throw ex;
    }
    case GF_AUTHENTICATION_REQUIRED_EXCEPTION: {
      AuthenticationRequiredException ex(
          str, (exMsg != nullptr ? exMsg : ": no authentication provided"));
      setTSSExceptionMessage(nullptr);
      throw ex;
    }
    case GF_DUPLICATE_DURABLE_CLIENT: {
      DuplicateDurableClientException ex(
          str, (exMsg != nullptr ? exMsg : ": Duplicate Durable Client Id"));
      setTSSExceptionMessage(nullptr);
      throw ex;
    }
    case GF_REMOTE_QUERY_EXCEPTION: {
      QueryException ex(str, (exMsg != nullptr ? exMsg : ": Query failed"));
      setTSSExceptionMessage(nullptr);
      throw ex;
    }
    case GF_CACHE_LOCATOR_EXCEPTION: {
      auto exCause = std::make_shared<NoAvailableLocatorsException>(
          str, (exMsg != nullptr ? exMsg : ": No locators available"));
      NotConnectedException ex(
          str, (exMsg != nullptr ? exMsg : ": No locators available"), false,
          exCause);
      setTSSExceptionMessage(nullptr);
      throw ex;
    }
    case GF_ALL_CONNECTIONS_IN_USE_EXCEPTION: {
      AllConnectionsInUseException ex(
          str, (exMsg != nullptr ? exMsg : ": All connections are in use"));
      setTSSExceptionMessage(nullptr);
      throw ex;
    }
    case GF_FUNCTION_EXCEPTION: {
      FunctionExecutionException ex(
          str, (exMsg != nullptr ? exMsg : ": Function execution failed"));
      setTSSExceptionMessage(nullptr);
      throw ex;
    }
    case GF_DISKFULL: {
      DiskFailureException ex(str, (exMsg != nullptr ? exMsg : ": Disk full"));
      setTSSExceptionMessage(nullptr);
      throw ex;
    }
    case GF_ROLLBACK_EXCEPTION: {
      RollbackException ex(
          str, (exMsg != nullptr ? exMsg : ": Transaction rolled back"));
      setTSSExceptionMessage(nullptr);
      throw ex;
    }
    case GF_COMMIT_CONFLICT_EXCEPTION: {
      CommitConflictException ex(
          str, (exMsg != nullptr ? exMsg : ": Commit conflict exception"));
      setTSSExceptionMessage(nullptr);
      throw ex;
    }
    case GF_TRANSACTION_DATA_REBALANCED_EXCEPTION: {
      TransactionDataRebalancedException ex(
          str, (exMsg != nullptr ? exMsg
                                 : ": Transaction data rebalanced exception"));
      setTSSExceptionMessage(nullptr);
      throw ex;
    }
    case GF_TRANSACTION_DATA_NODE_HAS_DEPARTED_EXCEPTION: {
      TransactionDataNodeHasDepartedException ex(
          str, (exMsg != nullptr
                    ? exMsg
                    : ": Transaction data node has departed exception"));
      setTSSExceptionMessage(nullptr);
      throw ex;
    }
    case GF_PUTALL_PARTIAL_RESULT_EXCEPTION: {
      PutAllPartialResultException ex(
          str, (exMsg != nullptr ? exMsg : ": PutAll Partial exception"));
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
      UnknownException ex(str, exMsg);
      setTSSExceptionMessage(nullptr);
      throw ex;
    }
  }
}
}  // namespace client
}  // namespace geode
}  // namespace apache
