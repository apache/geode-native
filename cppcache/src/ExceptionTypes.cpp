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

#include <map>
#include <string>

#include <geode/ExceptionTypes.hpp>

#include "ErrType.hpp"
#include "util/Log.hpp"

namespace apache {
namespace geode {
namespace client {

void setThreadLocalExceptionMessage(std::string exMsg);
const std::string& getThreadLocalExceptionMessage();

static std::map<GfErrType, std::function<void(std::string, std::string&,
                                              GfErrType, std::string)>>
    error_map;

[[noreturn]] void notConnectedException(std::string message, std::string& exMsg,
                                        GfErrType, std::string) {
  message.append(!exMsg.empty() ? exMsg : ": not connected to Geode");
  NotConnectedException ex(message);
  throw ex;
}

    [[noreturn]] void messageException(std::string message, std::string& exMsg,
                                       GfErrType, std::string) {
  message.append(!exMsg.empty() ? exMsg
                                : ": message from server could not be handled");
  MessageException ex(message);
  throw ex;
}

[[noreturn]] void cacheServerException(std::string message, std::string& exMsg,
                                       GfErrType err, std::string) {
  if (err == GF_CACHESERVER_EXCEPTION) {
    message.append(!exMsg.empty() ? exMsg
                                  : ": exception happened at cache server");
  }
  if (err == GF_CACHE_REGION_NOT_FOUND) {
    message.append(!exMsg.empty() ? exMsg : ": region not found on server");
  }
  CacheServerException ex(message);
  throw ex;
}

    [[noreturn]] void notOwnerException(std::string message, std::string& exMsg,
                                        GfErrType, std::string) {
  message.append(!exMsg.empty() ? exMsg : ": not own the lock");
  NotOwnerException ex(message);
  throw ex;
}

[[noreturn]] void illegalStateException(std::string message, std::string& exMsg,
                                        GfErrType err, std::string) {
  if (err == GF_CACHE_REGION_NOT_GLOBAL) {
    message.append(!exMsg.empty() ? exMsg : ": region not global");
  }
  if (err == GF_CACHE_ILLEGAL_STATE_EXCEPTION) {
    message.append(!exMsg.empty() ? exMsg : ": illegal State");
  }
  IllegalStateException ex(message);
  throw ex;
}

    [[noreturn]] void illegalArgumentException(std::string message,
                                               std::string& exMsg,
                                               GfErrType err, std::string) {
  if (err == GF_CACHE_ILLEGAL_ARGUMENT_EXCEPTION) {
    message.append(!exMsg.empty() ? exMsg : ": illegal argument");
  }
  if (err == GF_CACHE_REGION_KEYS_NOT_STRINGS) {
    message.append(!exMsg.empty() ? exMsg
                                  : ": region entries do not support C access");
  }
  if (err == GF_CACHE_REGION_ENTRY_NOT_BYTES) {
    message.append(!exMsg.empty()
                       ? exMsg
                       : ": existing non-null values was not a byte array");
  }
  IllegalArgumentException ex(message);
  throw ex;
}

[[noreturn]] void cacheWriterException(std::string message, std::string& exMsg,
                                       GfErrType err, std::string) {
  if (err == GF_CACHE_WRITER_EXCEPTION) {
    message.append(!exMsg.empty() ? exMsg
                                  : ": exception on server during write");
  }
  if (err == GF_CACHEWRITER_ERROR) {
    message.append(!exMsg.empty() ? exMsg : ": exception in CacheWriter");
  }
  CacheWriterException ex(message);
  throw ex;
}

    [[noreturn]] void cacheLoaderException(std::string message,
                                           std::string& exMsg, GfErrType,
                                           std::string) {
  message.append(!exMsg.empty() ? exMsg : ": exception in CacheLoader");
  CacheLoaderException ex(message);
  throw ex;
}

[[noreturn]] void cacheListenerException(std::string message,
                                         std::string& exMsg, GfErrType,
                                         std::string) {
  message.append(!exMsg.empty() ? exMsg : ": exception in CacheListener");
  CacheListenerException ex(message);
  throw ex;
}

    [[noreturn]] void regionDestroyedException(std::string message,
                                               std::string& exMsg,
                                               GfErrType err, std::string) {
  if (err == GF_CACHE_REGION_INVALID) {
    message.append(!exMsg.empty() ? exMsg : ": region not valid");
  }
  if (err == GF_CACHE_REGION_DESTROYED_EXCEPTION) {
    message.append(!exMsg.empty() ? exMsg : ": Named Region Destroyed");
  }
  RegionDestroyedException ex(message);
  throw ex;
}

[[noreturn]] void cacheProxyException(std::string message, std::string& exMsg,
                                      GfErrType, std::string) {
  message.append(!exMsg.empty() ? exMsg : ": error in Cache proxy");
  CacheProxyException ex(message);
  throw ex;
}

    [[noreturn]] void geodeIOException(std::string message, std::string& exMsg,
                                       GfErrType, std::string) {
  message.append(!exMsg.empty() ? exMsg : ": Input/Output error in operation");
  GeodeIOException ex(message);
  throw ex;
}

[[noreturn]] void noSystemException(std::string message, std::string& exMsg,
                                    GfErrType, std::string) {
  message.append(!exMsg.empty() ? exMsg : ": entity does not exist");
  NoSystemException ex(message);
  throw ex;
}

    [[noreturn]] void timeoutException(std::string message, std::string& exMsg,
                                       GfErrType err, std::string) {
  if (err == GF_CLIENT_WAIT_TIMEOUT) {
    message.append(!exMsg.empty()
                       ? exMsg
                       : ": timed out, possibly bucket is not available.");
  } else {
    message.append(!exMsg.empty() ? exMsg : ": timed out");
  }
  TimeoutException ex(message);
  throw ex;
}

[[noreturn]] void outOfMemoryException(std::string message, std::string& exMsg,
                                       GfErrType, std::string) {
  message.append(!exMsg.empty() ? exMsg : ": Out of memory");
  OutOfMemoryException ex(message);
  throw ex;
}

    [[noreturn]] void bufferSizeExceededException(std::string message,
                                                  std::string& exMsg, GfErrType,
                                                  std::string) {
  message.append(!exMsg.empty() ? exMsg : ": Buffer Size Exceeded");
  BufferSizeExceededException ex(message);
  throw ex;
}

[[noreturn]] void leaseExpiredException(std::string message, std::string& exMsg,
                                        GfErrType, std::string) {
  message.append(!exMsg.empty() ? exMsg : ": lock Lease Expired On you");
  LeaseExpiredException ex(message);
  throw ex;
}

    [[noreturn]] void regionExistsException(std::string message,
                                            std::string& exMsg, GfErrType,
                                            std::string) {
  message.append(!exMsg.empty() ? exMsg : ": Named Region Exists");
  RegionExistsException ex(message);
  throw ex;
}

[[noreturn]] void entryNotFoundException(std::string message,
                                         std::string& exMsg, GfErrType,
                                         std::string) {
  message.append(!exMsg.empty() ? exMsg : ": Entry not found");
  EntryNotFoundException ex(message);
  throw ex;
}

    [[noreturn]] void entryExistsException(std::string message,
                                           std::string& exMsg, GfErrType,
                                           std::string) {
  message.append(!exMsg.empty() ? exMsg
                                : ": Entry already exists in the region");
  EntryExistsException ex(message);
  throw ex;
}

[[noreturn]] void entryDestroyedException(std::string message,
                                          std::string& exMsg, GfErrType,
                                          std::string) {
  message.append(!exMsg.empty() ? exMsg : ": Entry has been destroyed");
  EntryDestroyedException ex(message);
  throw ex;
}

    [[noreturn]] void cacheClosedException(std::string message,
                                           std::string& exMsg, GfErrType,
                                           std::string) {
  message.append(!exMsg.empty() ? exMsg : ": Cache has been closed");
  CacheClosedException ex(message);
  throw ex;
}

[[noreturn]] void statisticsDisabledException(std::string message,
                                              std::string& exMsg, GfErrType,
                                              std::string) {
  message.append(!exMsg.empty()
                     ? exMsg
                     : ": Statistics have been disabled for the region");
  StatisticsDisabledException ex(message);
  throw ex;
}

    [[noreturn]] void concurrentModificationException(std::string message,
                                                      std::string& exMsg,
                                                      GfErrType, std::string) {
  message.append(!exMsg.empty() ? exMsg
                                : ": Concurrent modification in the cache");
  ConcurrentModificationException ex(message);
  throw ex;
}

[[noreturn]] void notAuthorizedException(std::string message,
                                         std::string& exMsg, GfErrType,
                                         std::string) {
  message.append(!exMsg.empty() ? exMsg : ": unauthorized operation");
  NotAuthorizedException ex(message);
  throw ex;
}

    [[noreturn]] void authenticationFailedException(std::string message,
                                                    std::string& exMsg,
                                                    GfErrType, std::string) {
  message.append(!exMsg.empty() ? exMsg : ": authentication failed");
  AuthenticationFailedException ex(message);
  throw ex;
}

[[noreturn]] void authenticationRequiredException(std::string message,
                                                  std::string& exMsg, GfErrType,
                                                  std::string) {
  message.append(!exMsg.empty() ? exMsg : ": no authentication provided");
  AuthenticationRequiredException ex(message);
  throw ex;
}

    [[noreturn]] void duplicateDurableClientException(std::string message,
                                                      std::string& exMsg,
                                                      GfErrType, std::string) {
  message.append(!exMsg.empty() ? exMsg : ": Duplicate Durable Client Id");
  DuplicateDurableClientException ex(message);
  throw ex;
}

[[noreturn]] void queryException(std::string message, std::string& exMsg,
                                 GfErrType, std::string) {
  message.append(!exMsg.empty() ? exMsg : ": Query failed");
  QueryException ex(message);
  throw ex;
}

    [[noreturn]] void noAvailableLocatorsException(std::string,
                                                   std::string& exMsg,
                                                   GfErrType,
                                                   std::string func) {
  NoAvailableLocatorsException cause(
      func + (!exMsg.empty() ? exMsg : ": No locators available"));
  try {
    throw cause;
  } catch (...) {
    NotConnectedException ex(
        func + (!exMsg.empty() ? exMsg : ": No locators available"));
    std::throw_with_nested(ex);
  }
}

[[noreturn]] void allConnectionsInUseException(std::string message,
                                               std::string& exMsg, GfErrType,
                                               std::string) {
  message.append(!exMsg.empty() ? exMsg : ": All connections are in use");
  AllConnectionsInUseException ex(message);
  throw ex;
}

    [[noreturn]] void functionExecutionException(std::string message,
                                                 std::string& exMsg, GfErrType,
                                                 std::string) {
  message.append(!exMsg.empty() ? exMsg : ": Function execution failed");
  FunctionExecutionException ex(message);
  throw ex;
}

[[noreturn]] void diskFailureException(std::string message, std::string& exMsg,
                                       GfErrType, std::string) {
  message.append(!exMsg.empty() ? exMsg : ": Disk full");
  DiskFailureException ex(message);
  throw ex;
}

    [[noreturn]] void rollbackException(std::string message, std::string& exMsg,
                                        GfErrType, std::string) {
  message.append(!exMsg.empty() ? exMsg : ": Transaction rolled back");
  RollbackException ex(message);
  throw ex;
}

[[noreturn]] void commitConflictException(std::string message,
                                          std::string& exMsg, GfErrType,
                                          std::string) {
  message.append(!exMsg.empty() ? exMsg : ": Commit conflict exception");
  CommitConflictException ex(message);
  throw ex;
}

    [[noreturn]] void transactionDataRebalancedException(std::string message,
                                                         std::string& exMsg,
                                                         GfErrType,
                                                         std::string) {
  message.append(!exMsg.empty() ? exMsg
                                : ": Transaction data rebalanced exception");
  TransactionDataRebalancedException ex(message);
  throw ex;
}

[[noreturn]] void transactionDataNodeHasDepartedException(std::string message,
                                                          std::string& exMsg,
                                                          GfErrType,
                                                          std::string) {
  message.append(!exMsg.empty()
                     ? exMsg
                     : ": Transaction data node has departed exception");
  TransactionDataNodeHasDepartedException ex(message);
  throw ex;
}

    [[noreturn]] void putAllPartialResultException(std::string message,
                                                   std::string& exMsg,
                                                   GfErrType, std::string) {
  message.append(!exMsg.empty() ? exMsg : ": PutAll Partial exception");
  PutAllPartialResultException ex(message);
  throw ex;
}

[[noreturn]] void unknownException(std::string message, std::string& exMsg,
                                   GfErrType err, std::string func) {
  LOGINFO("error code: %d", err);
  message = func;
  if (exMsg.empty()) {
    message.append("Unknown error code ").append(std::to_string(err));
  } else {
    message.append(exMsg);
  }
  UnknownException ex(message);
  throw ex;
}

void setErrorMap() {
  error_map.emplace(std::make_pair(GF_NOTCON, notConnectedException));
  error_map.emplace(std::make_pair(GF_MSG, messageException));
  error_map.emplace(
      std::make_pair(GF_CACHESERVER_EXCEPTION, cacheServerException));
  error_map.emplace(std::make_pair(GF_NOTOWN, notOwnerException));
  error_map.emplace(
      std::make_pair(GF_CACHE_REGION_NOT_FOUND, cacheServerException));
  error_map.emplace(
      std::make_pair(GF_CACHE_REGION_NOT_GLOBAL, illegalStateException));
  error_map.emplace(std::make_pair(GF_CACHE_ILLEGAL_ARGUMENT_EXCEPTION,
                                   illegalArgumentException));
  error_map.emplace(
      std::make_pair(GF_CACHE_ILLEGAL_STATE_EXCEPTION, illegalStateException));
  error_map.emplace(
      std::make_pair(GF_CACHE_WRITER_EXCEPTION, cacheWriterException));
  error_map.emplace(std::make_pair(GF_CACHEWRITER_ERROR, cacheWriterException));
  error_map.emplace(
      std::make_pair(GF_CACHE_LOADER_EXCEPTION, cacheLoaderException));
  error_map.emplace(
      std::make_pair(GF_CACHE_LISTENER_EXCEPTION, cacheListenerException));
  error_map.emplace(
      std::make_pair(GF_CACHE_REGION_INVALID, regionDestroyedException));
  error_map.emplace(std::make_pair(GF_CACHE_PROXY, cacheProxyException));
  error_map.emplace(std::make_pair(GF_IOERR, geodeIOException));
  error_map.emplace(std::make_pair(GF_ENOENT, noSystemException));
  error_map.emplace(std::make_pair(GF_CACHE_REGION_KEYS_NOT_STRINGS,
                                   illegalArgumentException));
  error_map.emplace(std::make_pair(GF_CACHE_REGION_ENTRY_NOT_BYTES,
                                   illegalArgumentException));
  error_map.emplace(
      std::make_pair(GF_CACHE_TIMEOUT_EXCEPTION, timeoutException));
  error_map.emplace(std::make_pair(GF_TIMEOUT, timeoutException));
  error_map.emplace(std::make_pair(GF_CLIENT_WAIT_TIMEOUT, timeoutException));
  error_map.emplace(std::make_pair(GF_ENOMEM, outOfMemoryException));
  error_map.emplace(std::make_pair(GF_ERANGE, bufferSizeExceededException));
  error_map.emplace(
      std::make_pair(GF_CACHE_LEASE_EXPIRED_EXCEPTION, leaseExpiredException));
  error_map.emplace(
      std::make_pair(GF_CACHE_REGION_EXISTS_EXCEPTION, regionExistsException));
  error_map.emplace(
      std::make_pair(GF_CACHE_ENTRY_NOT_FOUND, entryNotFoundException));
  error_map.emplace(
      std::make_pair(GF_CACHE_ENTRY_EXISTS, entryExistsException));
  error_map.emplace(std::make_pair(GF_CACHE_ENTRY_DESTROYED_EXCEPTION,
                                   entryDestroyedException));
  error_map.emplace(std::make_pair(GF_CACHE_REGION_DESTROYED_EXCEPTION,
                                   regionDestroyedException));
  error_map.emplace(
      std::make_pair(GF_CACHE_CLOSED_EXCEPTION, cacheClosedException));
  error_map.emplace(std::make_pair(GF_CACHE_STATISTICS_DISABLED_EXCEPTION,
                                   statisticsDisabledException));
  error_map.emplace(std::make_pair(GF_CACHE_CONCURRENT_MODIFICATION_EXCEPTION,
                                   concurrentModificationException));
  error_map.emplace(
      std::make_pair(GF_NOT_AUTHORIZED_EXCEPTION, notAuthorizedException));
  error_map.emplace(std::make_pair(GF_AUTHENTICATION_FAILED_EXCEPTION,
                                   authenticationFailedException));
  error_map.emplace(std::make_pair(GF_AUTHENTICATION_REQUIRED_EXCEPTION,
                                   authenticationRequiredException));
  error_map.emplace(std::make_pair(GF_DUPLICATE_DURABLE_CLIENT,
                                   duplicateDurableClientException));
  error_map.emplace(std::make_pair(GF_REMOTE_QUERY_EXCEPTION, queryException));
  error_map.emplace(
      std::make_pair(GF_CACHE_LOCATOR_EXCEPTION, noAvailableLocatorsException));
  error_map.emplace(std::make_pair(GF_ALL_CONNECTIONS_IN_USE_EXCEPTION,
                                   allConnectionsInUseException));
  error_map.emplace(
      std::make_pair(GF_FUNCTION_EXCEPTION, functionExecutionException));
  error_map.emplace(std::make_pair(GF_DISKFULL, diskFailureException));
  error_map.emplace(std::make_pair(GF_ROLLBACK_EXCEPTION, rollbackException));
  error_map.emplace(
      std::make_pair(GF_COMMIT_CONFLICT_EXCEPTION, commitConflictException));
  error_map.emplace(std::make_pair(GF_TRANSACTION_DATA_REBALANCED_EXCEPTION,
                                   transactionDataRebalancedException));
  error_map.emplace(
      std::make_pair(GF_TRANSACTION_DATA_NODE_HAS_DEPARTED_EXCEPTION,
                     transactionDataNodeHasDepartedException));
  error_map.emplace(std::make_pair(GF_PUTALL_PARTIAL_RESULT_EXCEPTION,
                                   putAllPartialResultException));
  error_map.emplace(std::make_pair(GF_NOERR, unknownException));
  error_map.emplace(std::make_pair(GF_DEADLK, unknownException));
  error_map.emplace(std::make_pair(GF_EACCES, unknownException));
  error_map.emplace(std::make_pair(GF_ECONFL, unknownException));
  error_map.emplace(std::make_pair(GF_EINVAL, unknownException));
  error_map.emplace(std::make_pair(GF_ETYPE, unknownException));
  error_map.emplace(std::make_pair(GF_NOTOBJ, unknownException));
  error_map.emplace(std::make_pair(GF_NOTSUP, unknownException));
  error_map.emplace(std::make_pair(GF_SCPGBL, unknownException));
  error_map.emplace(std::make_pair(GF_SCPEXC, unknownException));
  error_map.emplace(std::make_pair(GF_OVRFLW, unknownException));
  error_map.emplace(std::make_pair(GF_EINTR, unknownException));
  error_map.emplace(std::make_pair(GF_NOSERVER_FOUND, unknownException));
  error_map.emplace(std::make_pair(GF_SERVER_FAILED, unknownException));
  error_map.emplace(std::make_pair(GF_CLIENT_WAIT_TIMEOUT_REFRESH_PRMETADATA,
                                   unknownException));
  error_map.emplace(
      std::make_pair(GF_CANNOT_PROCESS_GII_REQUEST, unknownException));
  error_map.emplace(std::make_pair(GF_CACHE_ENTRY_UPDATED, unknownException));
  error_map.emplace(std::make_pair(GF_INVALID_DELTA, unknownException));
  error_map.emplace(std::make_pair(GF_EUNDEF, unknownException));
}

[[noreturn]] void GfErrTypeThrowException(const char* str, GfErrType err) {
  std::string func;
  std::string message;
  auto exMsg = getThreadLocalExceptionMessage();
  setThreadLocalExceptionMessage("");
  if (!exMsg.empty()) {
    func.append(str);
    func.append(": ");
  }

  setErrorMap();
  auto iter = error_map.find(err);
  if (iter != std::end(error_map)) {
    iter->second(message, exMsg, err, func);
  }
  unknownException(message, exMsg, err, func);
}
}  // namespace client
}  // namespace geode
}  // namespace apache
