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
    case GF_TIMOUT: {
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

AssertionException::~AssertionException() {}
std::string AssertionException::getName() const {
  return "apache::geode::client::AssertionException";
}

IllegalArgumentException::~IllegalArgumentException() {}
std::string IllegalArgumentException::getName() const {
  return "apache::geode::client::IllegalArgumentException";
}

IllegalStateException::~IllegalStateException() {}
std::string IllegalStateException::getName() const {
  return "apache::geode::client::IllegalStateException";
}

CacheExistsException::~CacheExistsException() {}
std::string CacheExistsException::getName() const {
  return "apache::geode::client::CacheExistsException";
}

CacheXmlException::~CacheXmlException() {}
std::string CacheXmlException::getName() const {
  return "apache::geode::client::CacheXmlException";
}

TimeoutException::~TimeoutException() {}
std::string TimeoutException::getName() const {
  return "apache::geode::client::TimeoutException";
}

CacheWriterException::~CacheWriterException() {}
std::string CacheWriterException::getName() const {
  return "apache::geode::client::CacheWriterException";
}

RegionExistsException::~RegionExistsException() {}
std::string RegionExistsException::getName() const {
  return "apache::geode::client::RegionExistsException";
}

CacheClosedException::~CacheClosedException() {}
std::string CacheClosedException::getName() const {
  return "apache::geode::client::CacheClosedException";
}

LeaseExpiredException::~LeaseExpiredException() {}
std::string LeaseExpiredException::getName() const {
  return "apache::geode::client::LeaseExpiredException";
}

CacheLoaderException::~CacheLoaderException() {}
std::string CacheLoaderException::getName() const {
  return "apache::geode::client::CacheLoaderException";
}

RegionDestroyedException::~RegionDestroyedException() {}
std::string RegionDestroyedException::getName() const {
  return "apache::geode::client::RegionDestroyedException";
}

EntryDestroyedException::~EntryDestroyedException() {}
std::string EntryDestroyedException::getName() const {
  return "apache::geode::client::EntryDestroyedException";
}

NoSystemException::~NoSystemException() {}
std::string NoSystemException::getName() const {
  return "apache::geode::client::NoSystemException";
}

AlreadyConnectedException::~AlreadyConnectedException() {}
std::string AlreadyConnectedException::getName() const {
  return "apache::geode::client::AlreadyConnectedException";
}

FileNotFoundException::~FileNotFoundException() {}
std::string FileNotFoundException::getName() const {
  return "apache::geode::client::FileNotFoundException";
}

InterruptedException::~InterruptedException() {}
std::string InterruptedException::getName() const {
  return "apache::geode::client::InterruptedException";
}

UnsupportedOperationException::~UnsupportedOperationException() {}
std::string UnsupportedOperationException::getName() const {
  return "apache::geode::client::UnsupportedOperationException";
}

StatisticsDisabledException::~StatisticsDisabledException() {}
std::string StatisticsDisabledException::getName() const {
  return "apache::geode::client::StatisticsDisabledException";
}

ConcurrentModificationException::~ConcurrentModificationException() {}
std::string ConcurrentModificationException::getName() const {
  return "apache::geode::client::ConcurrentModificationException";
}

UnknownException::~UnknownException() {}
std::string UnknownException::getName() const {
  return "apache::geode::client::UnknownException";
}

ClassCastException::~ClassCastException() {}
std::string ClassCastException::getName() const {
  return "apache::geode::client::ClassCastException";
}

EntryNotFoundException::~EntryNotFoundException() {}
std::string EntryNotFoundException::getName() const {
  return "apache::geode::client::EntryNotFoundException";
}

GeodeIOException::~GeodeIOException() {}
std::string GeodeIOException::getName() const {
  return "apache::geode::client::GeodeIOException";
}

GeodeConfigException::~GeodeConfigException() {}
std::string GeodeConfigException::getName() const {
  return "apache::geode::client::GeodeConfigException";
}

NullPointerException::~NullPointerException() {}
std::string NullPointerException::getName() const {
  return "apache::geode::client::NullPointerException";
}

EntryExistsException::~EntryExistsException() {}
std::string EntryExistsException::getName() const {
  return "apache::geode::client::EntryExistsException";
}

NotConnectedException::~NotConnectedException() {}
std::string NotConnectedException::getName() const {
  return "apache::geode::client::NotConnectedException";
}

CacheProxyException::~CacheProxyException() {}
std::string CacheProxyException::getName() const {
  return "apache::geode::client::CacheProxyException";
}

OutOfMemoryException::~OutOfMemoryException() {}
std::string OutOfMemoryException::getName() const {
  return "apache::geode::client::OutOfMemoryException";
}

NotOwnerException::~NotOwnerException() {}
std::string NotOwnerException::getName() const {
  return "apache::geode::client::NotOwnerException";
}

WrongRegionScopeException::~WrongRegionScopeException() {}
std::string WrongRegionScopeException::getName() const {
  return "apache::geode::client::WrongRegionScopeException";
}

BufferSizeExceededException::~BufferSizeExceededException() {}
std::string BufferSizeExceededException::getName() const {
  return "apache::geode::client::BufferSizeExceededException";
}

RegionCreationFailedException::~RegionCreationFailedException() {}
std::string RegionCreationFailedException::getName() const {
  return "apache::geode::client::RegionCreationFailedException";
}

FatalInternalException::~FatalInternalException() {}
std::string FatalInternalException::getName() const {
  return "apache::geode::client::FatalInternalException";
}

DiskFailureException::~DiskFailureException() {}
std::string DiskFailureException::getName() const {
  return "apache::geode::client::DiskFailureException";
}

DiskCorruptException::~DiskCorruptException() {}
std::string DiskCorruptException::getName() const {
  return "apache::geode::client::DiskCorruptException";
}

InitFailedException::~InitFailedException() {}
std::string InitFailedException::getName() const {
  return "apache::geode::client::InitFailedException";
}

ShutdownFailedException::~ShutdownFailedException() {}
std::string ShutdownFailedException::getName() const {
  return "apache::geode::client::ShutdownFailedException";
}

CacheServerException::~CacheServerException() {}
std::string CacheServerException::getName() const {
  return "apache::geode::client::CacheServerException";
}

OutOfRangeException::~OutOfRangeException() {}
std::string OutOfRangeException::getName() const {
  return "apache::geode::client::OutOfRangeException";
}

QueryException::~QueryException() {}
std::string QueryException::getName() const {
  return "apache::geode::client::QueryException";
}

MessageException::~MessageException() {}
std::string MessageException::getName() const {
  return "apache::geode::client::MessageException";
}

NotAuthorizedException::~NotAuthorizedException() {}
std::string NotAuthorizedException::getName() const {
  return "apache::geode::client::NotAuthorizedException";
}

AuthenticationFailedException::~AuthenticationFailedException() {}
std::string AuthenticationFailedException::getName() const {
  return "apache::geode::client::AuthenticationFailedException";
}

AuthenticationRequiredException::~AuthenticationRequiredException() {}
std::string AuthenticationRequiredException::getName() const {
  return "apache::geode::client::AuthenticationRequiredException";
}

DuplicateDurableClientException::~DuplicateDurableClientException() {}
std::string DuplicateDurableClientException::getName() const {
  return "apache::geode::client::DuplicateDurableClientException";
}

CacheListenerException::~CacheListenerException() {}
std::string CacheListenerException::getName() const {
  return "apache::geode::client::CacheListenerException";
}

CqException::~CqException() {}
std::string CqException::getName() const {
  return "apache::geode::client::CqException";
}

CqClosedException::~CqClosedException() {}
std::string CqClosedException::getName() const {
  return "apache::geode::client::CqClosedException";
}

CqQueryException::~CqQueryException() {}
std::string CqQueryException::getName() const {
  return "apache::geode::client::CqQueryException";
}

CqExistsException::~CqExistsException() {}
std::string CqExistsException::getName() const {
  return "apache::geode::client::CqExistsException";
}

CqInvalidException::~CqInvalidException() {}
std::string CqInvalidException::getName() const {
  return "apache::geode::client::CqInvalidException";
}

FunctionExecutionException::~FunctionExecutionException() {}
std::string FunctionExecutionException::getName() const {
  return "apache::geode::client::FunctionExecutionException";
}

NoAvailableLocatorsException::~NoAvailableLocatorsException() {}
std::string NoAvailableLocatorsException::getName() const {
  return "apache::geode::client::NoAvailableLocatorsException";
}

AllConnectionsInUseException::~AllConnectionsInUseException() {}
std::string AllConnectionsInUseException::getName() const {
  return "apache::geode::client::AllConnectionsInUseException";
}

InvalidDeltaException::~InvalidDeltaException() {}
std::string InvalidDeltaException::getName() const {
  return "apache::geode::client::InvalidDeltaException";
}

KeyNotFoundException::~KeyNotFoundException() {}
std::string KeyNotFoundException::getName() const {
  return "apache::geode::client::KeyNotFoundException";
}

TransactionException::~TransactionException() {}
std::string TransactionException::getName() const {
  return "apache::geode::client::TransactionException";
}

RollbackException::~RollbackException() {}
std::string RollbackException::getName() const {
  return "apache::geode::client::RollbackException";
}

CommitConflictException::~CommitConflictException() {}
std::string CommitConflictException::getName() const {
  return "apache::geode::client::CommitConflictException";
}

TransactionDataNodeHasDepartedException::
    ~TransactionDataNodeHasDepartedException() {}
std::string TransactionDataNodeHasDepartedException::getName() const {
  return "apache::geode::client::TransactionDataNodeHasDepartedException";
}

TransactionDataRebalancedException::~TransactionDataRebalancedException() {}
std::string TransactionDataRebalancedException::getName() const {
  return "apache::geode::client::TransactionDataRebalancedException";
}

PutAllPartialResultException::~PutAllPartialResultException() {}
std::string PutAllPartialResultException::getName() const {
  return "apache::geode::client::PutAllPartialResultException";
}

SslException::~SslException() {}
std::string SslException::getName() const {
  return "apache::geode::client::SslException";
}
}  // namespace client
}  // namespace geode
}  // namespace apache
