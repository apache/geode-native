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

namespace {

using apache::geode::client::AllConnectionsInUseException;
using apache::geode::client::AuthenticationFailedException;
using apache::geode::client::AuthenticationRequiredException;
using apache::geode::client::BufferSizeExceededException;
using apache::geode::client::CacheClosedException;
using apache::geode::client::CacheListenerException;
using apache::geode::client::CacheLoaderException;
using apache::geode::client::CacheProxyException;
using apache::geode::client::CacheServerException;
using apache::geode::client::CacheWriterException;
using apache::geode::client::CommitConflictException;
using apache::geode::client::ConcurrentModificationException;
using apache::geode::client::DiskFailureException;
using apache::geode::client::DuplicateDurableClientException;
using apache::geode::client::EntryDestroyedException;
using apache::geode::client::EntryExistsException;
using apache::geode::client::EntryNotFoundException;
using apache::geode::client::FunctionExecutionException;
using apache::geode::client::GeodeIOException;
using apache::geode::client::IllegalArgumentException;
using apache::geode::client::IllegalStateException;
using apache::geode::client::LeaseExpiredException;
using apache::geode::client::LowMemoryException;
using apache::geode::client::MessageException;
using apache::geode::client::NoAvailableLocatorsException;
using apache::geode::client::NoSystemException;
using apache::geode::client::NotAuthorizedException;
using apache::geode::client::NotConnectedException;
using apache::geode::client::NotOwnerException;
using apache::geode::client::OutOfMemoryException;
using apache::geode::client::PutAllPartialResultException;
using apache::geode::client::QueryException;
using apache::geode::client::QueryExecutionLowMemoryException;
using apache::geode::client::RegionDestroyedException;
using apache::geode::client::RegionExistsException;
using apache::geode::client::RollbackException;
using apache::geode::client::StatisticsDisabledException;
using apache::geode::client::TimeoutException;
using apache::geode::client::TransactionDataNodeHasDepartedException;
using apache::geode::client::TransactionDataRebalancedException;
using apache::geode::client::UnknownException;

[[noreturn]] void notConnectedException(std::string message, std::string& exMsg,
                                        GfErrType, std::string) {
  message.append(!exMsg.empty() ? exMsg : ": not connected to Geode");
  throw NotConnectedException{message};
}

[[noreturn]] void messageException(std::string message, std::string& exMsg,
                                   GfErrType, std::string) {
  message.append(!exMsg.empty() ? exMsg
                                : ": message from server could not be handled");
  throw MessageException{message};
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

  throw CacheServerException{message};
}

[[noreturn]] void notOwnerException(std::string message, std::string& exMsg,
                                    GfErrType, std::string) {
  message.append(!exMsg.empty() ? exMsg : ": not own the lock");
  throw NotOwnerException{message};
}

[[noreturn]] void illegalStateException(std::string message, std::string& exMsg,
                                        GfErrType err, std::string) {
  if (err == GF_CACHE_REGION_NOT_GLOBAL) {
    message.append(!exMsg.empty() ? exMsg : ": region not global");
  }
  if (err == GF_CACHE_ILLEGAL_STATE_EXCEPTION) {
    message.append(!exMsg.empty() ? exMsg : ": illegal State");
  }
  throw IllegalStateException{message};
}

[[noreturn]] void illegalArgumentException(std::string message,
                                           std::string& exMsg, GfErrType err,
                                           std::string) {
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
  throw IllegalArgumentException{message};
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
  throw CacheWriterException{message};
}

[[noreturn]] void cacheLoaderException(std::string message, std::string& exMsg,
                                       GfErrType, std::string) {
  message.append(!exMsg.empty() ? exMsg : ": exception in CacheLoader");
  throw CacheLoaderException{message};
}

[[noreturn]] void cacheListenerException(std::string message,
                                         std::string& exMsg, GfErrType,
                                         std::string) {
  message.append(!exMsg.empty() ? exMsg : ": exception in CacheListener");
  throw CacheListenerException{message};
}

[[noreturn]] void regionDestroyedException(std::string message,
                                           std::string& exMsg, GfErrType err,
                                           std::string) {
  if (err == GF_CACHE_REGION_INVALID) {
    message.append(!exMsg.empty() ? exMsg : ": region not valid");
  }
  if (err == GF_CACHE_REGION_DESTROYED_EXCEPTION) {
    message.append(!exMsg.empty() ? exMsg : ": Named Region Destroyed");
  }
  throw RegionDestroyedException{message};
}

[[noreturn]] void cacheProxyException(std::string message, std::string& exMsg,
                                      GfErrType, std::string) {
  message.append(!exMsg.empty() ? exMsg : ": error in Cache proxy");
  throw CacheProxyException{message};
}

[[noreturn]] void geodeIOException(std::string message, std::string& exMsg,
                                   GfErrType, std::string) {
  message.append(!exMsg.empty() ? exMsg : ": Input/Output error in operation");
  throw GeodeIOException{message};
}

[[noreturn]] void noSystemException(std::string message, std::string& exMsg,
                                    GfErrType, std::string) {
  message.append(!exMsg.empty() ? exMsg : ": entity does not exist");
  throw NoSystemException{message};
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
  throw TimeoutException{message};
}

[[noreturn]] void outOfMemoryException(std::string message, std::string& exMsg,
                                       GfErrType, std::string) {
  message.append(!exMsg.empty() ? exMsg : ": Out of memory");
  throw OutOfMemoryException{message};
}

[[noreturn]] void bufferSizeExceededException(std::string message,
                                              std::string& exMsg, GfErrType,
                                              std::string) {
  message.append(!exMsg.empty() ? exMsg : ": Buffer Size Exceeded");
  throw BufferSizeExceededException{message};
}

[[noreturn]] void leaseExpiredException(std::string message, std::string& exMsg,
                                        GfErrType, std::string) {
  message.append(!exMsg.empty() ? exMsg : ": lock Lease Expired On you");
  throw LeaseExpiredException{message};
}

[[noreturn]] void regionExistsException(std::string message, std::string& exMsg,
                                        GfErrType, std::string) {
  message.append(!exMsg.empty() ? exMsg : ": Named Region Exists");
  throw RegionExistsException{message};
}

[[noreturn]] void entryNotFoundException(std::string message,
                                         std::string& exMsg, GfErrType,
                                         std::string) {
  message.append(!exMsg.empty() ? exMsg : ": Entry not found");
  throw EntryNotFoundException{message};
}

[[noreturn]] void entryExistsException(std::string message, std::string& exMsg,
                                       GfErrType, std::string) {
  message.append(!exMsg.empty() ? exMsg
                                : ": Entry already exists in the region");
  throw EntryExistsException{message};
}

[[noreturn]] void entryDestroyedException(std::string message,
                                          std::string& exMsg, GfErrType,
                                          std::string) {
  message.append(!exMsg.empty() ? exMsg : ": Entry has been destroyed");
  throw EntryDestroyedException{message};
}

[[noreturn]] void cacheClosedException(std::string message, std::string& exMsg,
                                       GfErrType, std::string) {
  message.append(!exMsg.empty() ? exMsg : ": Cache has been closed");
  throw CacheClosedException{message};
}

[[noreturn]] void statisticsDisabledException(std::string message,
                                              std::string& exMsg, GfErrType,
                                              std::string) {
  message.append(!exMsg.empty()
                     ? exMsg
                     : ": Statistics have been disabled for the region");
  throw StatisticsDisabledException{message};
}

[[noreturn]] void concurrentModificationException(std::string message,
                                                  std::string& exMsg, GfErrType,
                                                  std::string) {
  message.append(!exMsg.empty() ? exMsg
                                : ": Concurrent modification in the cache");
  throw ConcurrentModificationException{message};
}

[[noreturn]] void notAuthorizedException(std::string message,
                                         std::string& exMsg, GfErrType,
                                         std::string) {
  message.append(!exMsg.empty() ? exMsg : ": unauthorized operation");
  throw NotAuthorizedException{message};
}

[[noreturn]] void authenticationFailedException(std::string message,
                                                std::string& exMsg, GfErrType,
                                                std::string) {
  message.append(!exMsg.empty() ? exMsg : ": authentication failed");
  throw AuthenticationFailedException{message};
}

[[noreturn]] void authenticationRequiredException(std::string message,
                                                  std::string& exMsg, GfErrType,
                                                  std::string) {
  message.append(!exMsg.empty() ? exMsg : ": no authentication provided");
  throw AuthenticationRequiredException{message};
}

[[noreturn]] void duplicateDurableClientException(std::string message,
                                                  std::string& exMsg, GfErrType,
                                                  std::string) {
  message.append(!exMsg.empty() ? exMsg : ": Duplicate Durable Client Id");
  throw DuplicateDurableClientException{message};
}

[[noreturn]] void queryException(std::string message, std::string& exMsg,
                                 GfErrType, std::string) {
  message.append(!exMsg.empty() ? exMsg : ": Query failed");
  throw QueryException{message};
}

[[noreturn]] void noAvailableLocatorsException(std::string, std::string& exMsg,
                                               GfErrType, std::string func) {
  try {
    throw NoAvailableLocatorsException{
        func + (!exMsg.empty() ? exMsg : ": No locators available")};
  } catch (...) {
    std::throw_with_nested(NotConnectedException{
        func + (!exMsg.empty() ? exMsg : ": No locators available")});
  }
}

[[noreturn]] void allConnectionsInUseException(std::string message,
                                               std::string& exMsg, GfErrType,
                                               std::string) {
  message.append(!exMsg.empty() ? exMsg : ": All connections are in use");
  throw AllConnectionsInUseException{message};
}

[[noreturn]] void functionExecutionException(std::string message,
                                             std::string& exMsg, GfErrType,
                                             std::string) {
  message.append(!exMsg.empty() ? exMsg : ": Function execution failed");
  throw FunctionExecutionException{message};
}

[[noreturn]] void diskFailureException(std::string message, std::string& exMsg,
                                       GfErrType, std::string) {
  message.append(!exMsg.empty() ? exMsg : ": Disk full");
  throw DiskFailureException{message};
}

[[noreturn]] void rollbackException(std::string message, std::string& exMsg,
                                    GfErrType, std::string) {
  message.append(!exMsg.empty() ? exMsg : ": Transaction rolled back");
  throw RollbackException{message};
}

[[noreturn]] void commitConflictException(std::string message,
                                          std::string& exMsg, GfErrType,
                                          std::string) {
  message.append(!exMsg.empty() ? exMsg : ": Commit conflict exception");
  throw CommitConflictException{message};
}

[[noreturn]] void transactionDataRebalancedException(std::string message,
                                                     std::string& exMsg,
                                                     GfErrType, std::string) {
  message.append(!exMsg.empty() ? exMsg
                                : ": Transaction data rebalanced exception");
  throw TransactionDataRebalancedException{message};
}

[[noreturn]] void transactionDataNodeHasDepartedException(std::string message,
                                                          std::string& exMsg,
                                                          GfErrType,
                                                          std::string) {
  message.append(!exMsg.empty()
                     ? exMsg
                     : ": Transaction data node has departed exception");
  throw TransactionDataNodeHasDepartedException{message};
}

[[noreturn]] void putAllPartialResultException(std::string message,
                                               std::string& exMsg, GfErrType,
                                               std::string) {
  message.append(!exMsg.empty() ? exMsg : ": PutAll Partial exception");
  throw PutAllPartialResultException{message};
}

[[noreturn]] void lowMemoryException(std::string message, std::string& exMsg,
                                     GfErrType, std::string) {
  message.append(!exMsg.empty() ? exMsg : ": Low memory exception");
  throw LowMemoryException{message};
}

[[noreturn]] void queryLowMemoryException(std::string message,
                                          std::string& exMsg, GfErrType,
                                          std::string) {
  message.append(!exMsg.empty() ? exMsg
                                : ": Query execution low memory exception");
  throw QueryExecutionLowMemoryException{message};
}

[[noreturn]] void unknownException(std::string message, std::string& exMsg,
                                   GfErrType err, std::string func) {
  LOG_INFO("error code: %d", err);
  message = func;
  if (exMsg.empty()) {
    message.append("Unknown error code ").append(std::to_string(err));
  } else {
    message.append(exMsg);
  }
  throw UnknownException{message};
}

}  // namespace

using error_function_t =
    std::function<void(std::string, std::string&, GfErrType, std::string)>;

std::map<GfErrType, error_function_t>& get_error_map() {
  static std::map<GfErrType, error_function_t> error_map{
      {GF_NOTCON, notConnectedException},
      {GF_MSG, messageException},
      {GF_CACHESERVER_EXCEPTION, cacheServerException},
      {GF_NOTOWN, notOwnerException},
      {GF_CACHE_REGION_NOT_FOUND, cacheServerException},
      {GF_CACHE_REGION_NOT_GLOBAL, illegalStateException},
      {GF_CACHE_ILLEGAL_ARGUMENT_EXCEPTION, illegalArgumentException},
      {GF_CACHE_ILLEGAL_STATE_EXCEPTION, illegalStateException},
      {GF_CACHE_WRITER_EXCEPTION, cacheWriterException},
      {GF_CACHEWRITER_ERROR, cacheWriterException},
      {GF_CACHE_LOADER_EXCEPTION, cacheLoaderException},
      {GF_CACHE_LISTENER_EXCEPTION, cacheListenerException},
      {GF_CACHE_REGION_INVALID, regionDestroyedException},
      {GF_CACHE_PROXY, cacheProxyException},
      {GF_IOERR, geodeIOException},
      {GF_ENOENT, noSystemException},
      {GF_CACHE_REGION_KEYS_NOT_STRINGS, illegalArgumentException},
      {GF_CACHE_REGION_ENTRY_NOT_BYTES, illegalArgumentException},
      {GF_CACHE_TIMEOUT_EXCEPTION, timeoutException},
      {GF_TIMEOUT, timeoutException},
      {GF_CLIENT_WAIT_TIMEOUT, timeoutException},
      {GF_ENOMEM, outOfMemoryException},
      {GF_ERANGE, bufferSizeExceededException},
      {GF_CACHE_LEASE_EXPIRED_EXCEPTION, leaseExpiredException},
      {GF_CACHE_REGION_EXISTS_EXCEPTION, regionExistsException},
      {GF_CACHE_ENTRY_NOT_FOUND, entryNotFoundException},
      {GF_CACHE_ENTRY_EXISTS, entryExistsException},
      {GF_CACHE_ENTRY_DESTROYED_EXCEPTION, entryDestroyedException},
      {GF_CACHE_REGION_DESTROYED_EXCEPTION, regionDestroyedException},
      {GF_CACHE_CLOSED_EXCEPTION, cacheClosedException},
      {GF_CACHE_STATISTICS_DISABLED_EXCEPTION, statisticsDisabledException},
      {GF_CACHE_CONCURRENT_MODIFICATION_EXCEPTION,
       concurrentModificationException},
      {GF_NOT_AUTHORIZED_EXCEPTION, notAuthorizedException},
      {GF_AUTHENTICATION_FAILED_EXCEPTION, authenticationFailedException},
      {GF_AUTHENTICATION_REQUIRED_EXCEPTION, authenticationRequiredException},
      {GF_DUPLICATE_DURABLE_CLIENT, duplicateDurableClientException},
      {GF_REMOTE_QUERY_EXCEPTION, queryException},
      {GF_CACHE_LOCATOR_EXCEPTION, noAvailableLocatorsException},
      {GF_ALL_CONNECTIONS_IN_USE_EXCEPTION, allConnectionsInUseException},
      {GF_FUNCTION_EXCEPTION, functionExecutionException},
      {GF_DISKFULL, diskFailureException},
      {GF_ROLLBACK_EXCEPTION, rollbackException},
      {GF_COMMIT_CONFLICT_EXCEPTION, commitConflictException},
      {GF_TRANSACTION_DATA_REBALANCED_EXCEPTION,
       transactionDataRebalancedException},
      {GF_TRANSACTION_DATA_NODE_HAS_DEPARTED_EXCEPTION,
       transactionDataNodeHasDepartedException},
      {GF_PUTALL_PARTIAL_RESULT_EXCEPTION, putAllPartialResultException},
      {GF_LOW_MEMORY_EXCEPTION, lowMemoryException},
      {GF_QUERY_EXECUTION_LOW_MEMORY_EXCEPTION, queryLowMemoryException},
      {GF_NOERR, unknownException},
      {GF_DEADLK, unknownException},
      {GF_EACCES, unknownException},
      {GF_ECONFL, unknownException},
      {GF_EINVAL, unknownException},
      {GF_ETYPE, unknownException},
      {GF_NOTOBJ, unknownException},
      {GF_NOTSUP, unknownException},
      {GF_SCPGBL, unknownException},
      {GF_SCPEXC, unknownException},
      {GF_OVRFLW, unknownException},
      {GF_EINTR, unknownException},
      {GF_NOSERVER_FOUND, unknownException},
      {GF_SERVER_FAILED, unknownException},
      {GF_CLIENT_WAIT_TIMEOUT_REFRESH_PRMETADATA, unknownException},
      {GF_CANNOT_PROCESS_GII_REQUEST, unknownException},
      {GF_CACHE_ENTRY_UPDATED, unknownException},
      {GF_INVALID_DELTA, unknownException},
      {GF_EUNDEF, unknownException}};

  return error_map;
}

namespace apache {
namespace geode {
namespace client {

void setThreadLocalExceptionMessage(std::string exMsg);
const std::string& getThreadLocalExceptionMessage();

[[noreturn]] void GfErrTypeThrowException(const char* str, GfErrType err) {
  std::string func;
  std::string message;
  auto exMsg = getThreadLocalExceptionMessage();
  setThreadLocalExceptionMessage("");
  if (!exMsg.empty()) {
    func.append(str);
    func.append(": ");
  }

  auto& error_map = get_error_map();
  auto iter = error_map.find(err);
  if (iter != std::end(error_map)) {
    iter->second(message, exMsg, err, func);
  }
  unknownException(message, exMsg, err, func);
}
}  // namespace client
}  // namespace geode
}  // namespace apache
