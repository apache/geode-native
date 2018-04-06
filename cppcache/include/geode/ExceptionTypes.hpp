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

#ifndef GEODE_EXCEPTIONTYPES_H_
#define GEODE_EXCEPTIONTYPES_H_

#include "internal/geode_globals.hpp"
#include "geode/Exception.hpp"

namespace apache {
namespace geode {
namespace client {

/*
 *
 * This is the list of exceptions directly derived from
 * apache::geode::client::Exception.
 *
 */

/**
 *@brief A geode assertion exception.
 **/
class APACHE_GEODE_EXPORT AssertionException : public Exception {
 public:
  using Exception::Exception;
  virtual ~AssertionException() noexcept {}
  virtual std::string getName() const override {
    return "apache::geode::client::AssertionException";
  }
};

/**
 *@brief Thrown when an argument to a method is illegal.
 **/
class APACHE_GEODE_EXPORT IllegalArgumentException : public Exception {
 public:
  using Exception::Exception;
  virtual ~IllegalArgumentException() noexcept {}
  virtual std::string getName() const override {
    return "apache::geode::client::IllegalArgumentException";
  }
};

/**
 *@brief Thrown when the state of cache is manipulated to be illegal.
 **/
class APACHE_GEODE_EXPORT IllegalStateException : public Exception {
 public:
  using Exception::Exception;
  virtual ~IllegalStateException() noexcept {}
  virtual std::string getName() const override {
    return "apache::geode::client::IllegalStateException";
  }
};

/**
 *@brief Thrown when an attempt is made to create an existing cache.
 **/
class APACHE_GEODE_EXPORT CacheExistsException : public Exception {
 public:
  using Exception::Exception;
  virtual ~CacheExistsException() noexcept {}
  virtual std::string getName() const override {
    return "apache::geode::client::CacheExistsException";
  }
};

/**
 *@brief Thrown when the cache xml is incorrect.
 **/
class APACHE_GEODE_EXPORT CacheXmlException : public Exception {
 public:
  using Exception::Exception;
  virtual ~CacheXmlException() noexcept {}
  virtual std::string getName() const override {
    return "apache::geode::client::CacheXmlException";
  }
};
/**
 *@brief Thrown when a timout occurs.
 **/
class APACHE_GEODE_EXPORT TimeoutException : public Exception {
 public:
  using Exception::Exception;
  virtual ~TimeoutException() noexcept {}
  virtual std::string getName() const override {
    return "apache::geode::client::TimeoutException";
  }
};

/**
 *@brief Thrown when the cache writer aborts the operation.
 **/
class APACHE_GEODE_EXPORT CacheWriterException : public Exception {
 public:
  using Exception::Exception;
  virtual ~CacheWriterException() noexcept {}
  virtual std::string getName() const override {
    return "apache::geode::client::CacheWriterException";
  }
};

/**
 *@brief Thrown when an attempt is made to create an existing region.
 **/
class APACHE_GEODE_EXPORT RegionExistsException : public Exception {
 public:
  using Exception::Exception;
  virtual ~RegionExistsException() noexcept {}
  virtual std::string getName() const override {
    return "apache::geode::client::RegionExistsException";
  }
};

/**
 *@brief Thrown when an operation is attempted on a closed cache.
 **/
class APACHE_GEODE_EXPORT CacheClosedException : public Exception {
 public:
  using Exception::Exception;
  virtual ~CacheClosedException() noexcept {}
  virtual std::string getName() const override {
    return "apache::geode::client::CacheClosedException";
  }
};

/**
 *@brief Thrown when lease of cache proxy has expired.
 **/
class APACHE_GEODE_EXPORT LeaseExpiredException : public Exception {
 public:
  using Exception::Exception;
  virtual ~LeaseExpiredException() noexcept {}
  virtual std::string getName() const override {
    return "apache::geode::client::LeaseExpiredException";
  }
};

/**
 *@brief Thrown when the cache loader aborts the operation.
 **/
class APACHE_GEODE_EXPORT CacheLoaderException : public Exception {
 public:
  using Exception::Exception;
  virtual ~CacheLoaderException() noexcept {}
  virtual std::string getName() const override {
    return "apache::geode::client::CacheLoaderException";
  }
};

/**
 *@brief Thrown when an operation is attempted on a destroyed region.
 **/
class APACHE_GEODE_EXPORT RegionDestroyedException : public Exception {
 public:
  using Exception::Exception;
  virtual ~RegionDestroyedException() noexcept {}
  virtual std::string getName() const override {
    return "apache::geode::client::RegionDestroyedException";
  }
};

/**
 *@brief Thrown when an operation is attempted on a destroyed entry.
 **/
class APACHE_GEODE_EXPORT EntryDestroyedException : public Exception {
 public:
  using Exception::Exception;
  virtual ~EntryDestroyedException() noexcept {}
  virtual std::string getName() const override {
    return "apache::geode::client::EntryDestroyedException";
  }
};

/**
 *@brief Thrown when the connecting target is not running.
 **/
class APACHE_GEODE_EXPORT NoSystemException : public Exception {
 public:
  using Exception::Exception;
  virtual ~NoSystemException() noexcept {}
  virtual std::string getName() const override {
    return "apache::geode::client::NoSystemException";
  }
};

/**
 *@brief Thrown when an attempt is made to connect to
 *       DistributedSystem second time.
 **/
class APACHE_GEODE_EXPORT AlreadyConnectedException : public Exception {
 public:
  using Exception::Exception;
  virtual ~AlreadyConnectedException() noexcept {}
  virtual std::string getName() const override {
    return "apache::geode::client::AlreadyConnectedException";
  }
};

/**
 *@brief Thrown when a non-existing file is accessed.
 **/
class APACHE_GEODE_EXPORT FileNotFoundException : public Exception {
 public:
  using Exception::Exception;
  virtual ~FileNotFoundException() noexcept {}
  virtual std::string getName() const override {
    return "apache::geode::client::FileNotFoundException";
  }
};

/**
 *@brief Thrown when an operation is interrupted.
 **/
class APACHE_GEODE_EXPORT InterruptedException : public Exception {
 public:
  using Exception::Exception;
  virtual ~InterruptedException() noexcept {}
  virtual std::string getName() const override {
    return "apache::geode::client::InterruptedException";
  }
};

/**
 *@brief Thrown when an operation unsupported by the
 *       current configuration is attempted.
 **/
class APACHE_GEODE_EXPORT UnsupportedOperationException : public Exception {
 public:
  using Exception::Exception;
  virtual ~UnsupportedOperationException() noexcept {}
  virtual std::string getName() const override {
    return "apache::geode::client::UnsupportedOperationException";
  }
};

/**
 *@brief Thrown when statistics are invoked for a region where
 *       they are disabled.
 **/
class APACHE_GEODE_EXPORT StatisticsDisabledException : public Exception {
 public:
  using Exception::Exception;
  virtual ~StatisticsDisabledException() noexcept {}
  virtual std::string getName() const override {
    return "apache::geode::client::StatisticsDisabledException";
  }
};

/**
 *@brief Thrown when a concurrent operation fails.
 **/
class APACHE_GEODE_EXPORT ConcurrentModificationException : public Exception {
 public:
  using Exception::Exception;
  virtual ~ConcurrentModificationException() noexcept {}
  virtual std::string getName() const override {
    return "apache::geode::client::ConcurrentModificationException";
  }
};

/**
 *@brief An unknown exception occurred.
 **/
class APACHE_GEODE_EXPORT UnknownException : public Exception {
 public:
  using Exception::Exception;
  virtual ~UnknownException() noexcept {}
  virtual std::string getName() const override {
    return "apache::geode::client::UnknownException";
  }
};

/**
 *@brief Thrown when a cast operation fails.
 **/
class APACHE_GEODE_EXPORT ClassCastException : public Exception {
 public:
  using Exception::Exception;
  virtual ~ClassCastException() noexcept {}
  virtual std::string getName() const override {
    return "apache::geode::client::ClassCastException";
  }
};

/**
 *@brief Thrown when an operation is attempted on a non-existent entry.
 **/
class APACHE_GEODE_EXPORT EntryNotFoundException : public Exception {
 public:
  using Exception::Exception;
  virtual ~EntryNotFoundException() noexcept {}
  virtual std::string getName() const override {
    return "apache::geode::client::EntryNotFoundException";
  }
};

/**
 *@brief Thrown when there is an input/output error.
 **/
class APACHE_GEODE_EXPORT GeodeIOException : public Exception {
 public:
  using Exception::Exception;
  virtual ~GeodeIOException() noexcept {}
  virtual std::string getName() const override {
    return "apache::geode::client::GeodeIOException";
  }
};

/**
 *@brief Thrown when geode configuration file is incorrect.
 **/
class APACHE_GEODE_EXPORT GeodeConfigException : public Exception {
 public:
  using Exception::Exception;
  virtual ~GeodeConfigException() noexcept {}
  virtual std::string getName() const override {
    return "apache::geode::client::GeodeConfigException";
  }
};

/**
 *@brief Thrown when a null argument is provided to a method
 *       where it is expected to be non-null.
 **/
class APACHE_GEODE_EXPORT NullPointerException : public Exception {
 public:
  using Exception::Exception;
  virtual ~NullPointerException() noexcept {}
  virtual std::string getName() const override {
    return "apache::geode::client::NullPointerException";
  }
};

/**
 *@brief Thrown when attempt is made to create an existing entry.
 **/
class APACHE_GEODE_EXPORT EntryExistsException : public Exception {
 public:
  using Exception::Exception;
  virtual ~EntryExistsException() noexcept {}
  virtual std::string getName() const override {
    return "apache::geode::client::EntryExistsException";
  }
};

/**
 *@brief Thrown when an operation is attempted before connecting
 *       to the distributed system.
 **/
class APACHE_GEODE_EXPORT NotConnectedException : public Exception {
 public:
  using Exception::Exception;
  virtual ~NotConnectedException() noexcept {}
  virtual std::string getName() const override {
    return "apache::geode::client::NotConnectedException";
  }
};

/**
 *@brief Thrown when there is an error in the cache proxy.
 **/
class APACHE_GEODE_EXPORT CacheProxyException : public Exception {
 public:
  using Exception::Exception;
  virtual ~CacheProxyException() noexcept {}
  virtual std::string getName() const override {
    return "apache::geode::client::CacheProxyException";
  }
};

/**
 *@brief Thrown when the system cannot allocate any more memory.
 **/
class APACHE_GEODE_EXPORT OutOfMemoryException : public Exception {
 public:
  using Exception::Exception;
  virtual ~OutOfMemoryException() noexcept {}
  virtual std::string getName() const override {
    return "apache::geode::client::OutOfMemoryException";
  }
};

/**
 *@brief Thrown when an attempt is made to release a lock not
 *       owned by the thread.
 **/
class APACHE_GEODE_EXPORT NotOwnerException : public Exception {
 public:
  using Exception::Exception;
  virtual ~NotOwnerException() noexcept {}
  virtual std::string getName() const override {
    return "apache::geode::client::NotOwnerException";
  }
};

/**
 *@brief Thrown when a region is created in an incorrect scope.
 **/
class APACHE_GEODE_EXPORT WrongRegionScopeException : public Exception {
 public:
  using Exception::Exception;
  virtual ~WrongRegionScopeException() noexcept {}
  virtual std::string getName() const override {
    return "apache::geode::client::WrongRegionScopeException";
  }
};

/**
 *@brief Thrown when the internal buffer size is exceeded.
 **/
class APACHE_GEODE_EXPORT BufferSizeExceededException : public Exception {
 public:
  using Exception::Exception;
  virtual ~BufferSizeExceededException() noexcept {}
  virtual std::string getName() const override {
    return "apache::geode::client::BufferSizeExceededException";
  }
};

/**
 *@brief Thrown when a region creation operation fails.
 **/
class APACHE_GEODE_EXPORT RegionCreationFailedException : public Exception {
 public:
  using Exception::Exception;
  virtual ~RegionCreationFailedException() noexcept {}
  virtual std::string getName() const override {
    return "apache::geode::client::RegionCreationFailedException";
  }
};

/**
 *@brief Thrown when there is a fatal internal exception in geode.
 */
class APACHE_GEODE_EXPORT FatalInternalException : public Exception {
 public:
  using Exception::Exception;
  virtual ~FatalInternalException() noexcept {}
  virtual std::string getName() const override {
    return "apache::geode::client::FatalInternalException";
  }
};

/**
 *@brief Thrown by the persistence manager when a write
 *       fails due to disk failure.
 **/
class APACHE_GEODE_EXPORT DiskFailureException : public Exception {
 public:
  using Exception::Exception;
  virtual ~DiskFailureException() noexcept {}
  virtual std::string getName() const override {
    return "apache::geode::client::DiskFailureException";
  }
};

/**
 *@brief Thrown by the persistence manager when the data
 *@brief to be read from disk is corrupt.
 **/
class APACHE_GEODE_EXPORT DiskCorruptException : public Exception {
 public:
  using Exception::Exception;
  virtual ~DiskCorruptException() noexcept {}
  virtual std::string getName() const override {
    return "apache::geode::client::DiskCorruptException";
  }
};

/**
 *@brief Thrown when persistence manager fails to initialize.
 **/
class APACHE_GEODE_EXPORT InitFailedException : public Exception {
 public:
  using Exception::Exception;
  virtual ~InitFailedException() noexcept {}
  virtual std::string getName() const override {
    return "apache::geode::client::InitFailedException";
  }
};

/**
 *@brief Thrown when persistence manager fails to close properly.
 **/
class APACHE_GEODE_EXPORT ShutdownFailedException : public Exception {
 public:
  using Exception::Exception;
  virtual ~ShutdownFailedException() noexcept {}
  virtual std::string getName() const override {
    return "apache::geode::client::ShutdownFailedException";
  }
};

/**
 *@brief Thrown when an exception occurs on the cache server.
 **/
class APACHE_GEODE_EXPORT CacheServerException : public Exception {
 public:
  using Exception::Exception;
  virtual ~CacheServerException() noexcept {}
  virtual std::string getName() const override {
    return "apache::geode::client::CacheServerException";
  }
};

/**
 *@brief Thrown when bound of array/vector etc. is exceeded.
 **/
class APACHE_GEODE_EXPORT OutOfRangeException : public Exception {
 public:
  using Exception::Exception;
  virtual ~OutOfRangeException() noexcept {}
  virtual std::string getName() const override {
    return "apache::geode::client::OutOfRangeException";
  }
};

/**
 *@brief Thrown when query exception occurs at the server.
 **/
class APACHE_GEODE_EXPORT QueryException : public Exception {
 public:
  using Exception::Exception;
  virtual ~QueryException() noexcept {}
  virtual std::string getName() const override {
    return "apache::geode::client::QueryException";
  }
};

/**
 *@brief Thrown when an unknown message is received from the server.
 **/
class APACHE_GEODE_EXPORT MessageException : public Exception {
 public:
  using Exception::Exception;
  virtual ~MessageException() noexcept {}
  virtual std::string getName() const override {
    return "apache::geode::client::MessageException";
  }
};

/**
 *@brief Thrown when a non authorized operation is done.
 **/
class APACHE_GEODE_EXPORT NotAuthorizedException : public Exception {
 public:
  using Exception::Exception;
  virtual ~NotAuthorizedException() noexcept {}
  virtual std::string getName() const override {
    return "apache::geode::client::NotAuthorizedException";
  }
};

/**
 *@brief Thrown when authentication fails.
 **/
class APACHE_GEODE_EXPORT AuthenticationFailedException : public Exception {
 public:
  using Exception::Exception;
  virtual ~AuthenticationFailedException() noexcept {}
  virtual std::string getName() const override {
    return "apache::geode::client::AuthenticationFailedException";
  }
};

/**
 *@brief Thrown when no credentials are provided by client when server expects.
 **/
class APACHE_GEODE_EXPORT AuthenticationRequiredException : public Exception {
 public:
  using Exception::Exception;
  virtual ~AuthenticationRequiredException() noexcept {}
  virtual std::string getName() const override {
    return "apache::geode::client::AuthenticationRequiredException";
  }
};

/**
 *@brief Thrown when two durable connect with same Id.
 **/
class APACHE_GEODE_EXPORT DuplicateDurableClientException : public Exception {
 public:
  using Exception::Exception;
  virtual ~DuplicateDurableClientException() noexcept {}
  virtual std::string getName() const override {
    return "apache::geode::client::DuplicateDurableClientException";
  }
};

/**
 *@brief Thrown when the cache listener throws an exception.
 **/
class APACHE_GEODE_EXPORT CacheListenerException : public Exception {
 public:
  using Exception::Exception;
  virtual ~CacheListenerException() noexcept {}
  virtual std::string getName() const override {
    return "apache::geode::client::CacheListenerException";
  }
};
/**
 *@brief Thrown during continuous query execution time.
 **/
class APACHE_GEODE_EXPORT CqException : public Exception {
 public:
  using Exception::Exception;
  virtual ~CqException() noexcept {}
  virtual std::string getName() const override {
    return "apache::geode::client::CqException";
  }
};
/**
 *@brief Thrown if the Cq on which the operaion performed is closed
 **/
class APACHE_GEODE_EXPORT CqClosedException : public Exception {
 public:
  using Exception::Exception;
  virtual ~CqClosedException() noexcept {}
  virtual std::string getName() const override {
    return "apache::geode::client::CqClosedException";
  }
};
/**
 *@brief Thrown if the Cq Query failed
 **/
class APACHE_GEODE_EXPORT CqQueryException : public Exception {
 public:
  using Exception::Exception;
  virtual ~CqQueryException() noexcept {}
  virtual std::string getName() const override {
    return "apache::geode::client::CqQueryException";
  }
};
/**
 *@brief Thrown if a Cq by this name already exists on this client
 **/
class APACHE_GEODE_EXPORT CqExistsException : public Exception {
 public:
  using Exception::Exception;
  virtual ~CqExistsException() noexcept {}
  virtual std::string getName() const override {
    return "apache::geode::client::CqExistsException";
  }
};
/**
 *@brief  Thrown if the query doesnot meet the CQ constraints.
 * E.g.:Query string should refer only one region, join not supported.
 *      The query must be a SELECT statement.
 *      DISTINCT queries are not supported.
 *      Projections are not supported.
 *      Only one iterator in the FROM clause is supported, and it must be a
 *region path.
 *      Bind parameters in the query are not supported for the initial release.
 **/
class APACHE_GEODE_EXPORT CqInvalidException : public Exception {
 public:
  using Exception::Exception;
  virtual ~CqInvalidException() noexcept {}
  virtual std::string getName() const override {
    return "apache::geode::client::CqInvalidException";
  }
};
/**
 *@brief Thrown if function execution failed
 **/
class APACHE_GEODE_EXPORT FunctionExecutionException : public Exception {
 public:
  using Exception::Exception;
  virtual ~FunctionExecutionException() noexcept {}
  virtual std::string getName() const override {
    return "apache::geode::client::FunctionExecutionException";
  }
};
/**
 *@brief Thrown if the No locators are active to reply for new connection.
 **/
class APACHE_GEODE_EXPORT NoAvailableLocatorsException : public Exception {
 public:
  using Exception::Exception;
  virtual ~NoAvailableLocatorsException() noexcept {}
  virtual std::string getName() const override {
    return "apache::geode::client::NoAvailableLocatorsException";
  }
};
/**
 *@brief Thrown if all connections in the pool are in use.
 **/
class APACHE_GEODE_EXPORT AllConnectionsInUseException : public Exception {
 public:
  using Exception::Exception;
  virtual ~AllConnectionsInUseException() noexcept {}
  virtual std::string getName() const override {
    return "apache::geode::client::AllConnectionsInUseException";
  }
};
/**
 *@brief Thrown if Delta could not be applied.
 **/
class APACHE_GEODE_EXPORT InvalidDeltaException : public Exception {
 public:
  using Exception::Exception;
  virtual ~InvalidDeltaException() noexcept {}
  virtual std::string getName() const override {
    return "apache::geode::client::InvalidDeltaException";
  }
};
/**
 *@brief Thrown if a Key is not present in the region.
 **/
class APACHE_GEODE_EXPORT KeyNotFoundException : public Exception {
 public:
  using Exception::Exception;
  virtual ~KeyNotFoundException() noexcept {}
  virtual std::string getName() const override {
    return "apache::geode::client::KeyNotFoundException";
  }
};
/**
 * @brief This is for all Exceptions that may be thrown
 * by a Geode transaction.
 **/
class APACHE_GEODE_EXPORT TransactionException : public Exception {
 public:
  using Exception::Exception;
  virtual ~TransactionException() noexcept {}
  virtual std::string getName() const override {
    return "apache::geode::client::TransactionException";
  }
};
/**
 * @brief The RollbackException exception indicates that either the transaction
 * has been rolled back or an operation cannot complete because the
 * transaction is marked for rollback only.
 **/
class APACHE_GEODE_EXPORT RollbackException : public Exception {
 public:
  using Exception::Exception;
  virtual ~RollbackException() noexcept {}
  virtual std::string getName() const override {
    return "apache::geode::client::RollbackException";
  }
};
/**
 * @brief Thrown when a commit fails due to a write conflict.
 * @see CacheTransactionManager#commit
 **/
class APACHE_GEODE_EXPORT CommitConflictException : public Exception {
 public:
  using Exception::Exception;
  virtual ~CommitConflictException() noexcept {}
  virtual std::string getName() const override {
    return "apache::geode::client::CommitConflictException";
  }
};
/**
 * @brief Thrown when the transactional data host has shutdown or no longer has
 *the data
 * being modified by the transaction.
 * This can be thrown while doing transactional operations or during commit.
 **/
class APACHE_GEODE_EXPORT TransactionDataNodeHasDepartedException
    : public Exception {
 public:
  using Exception::Exception;
  virtual ~TransactionDataNodeHasDepartedException() noexcept {}
  virtual std::string getName() const override {
    return "apache::geode::client::TransactionDataNodeHasDepartedException";
  }
};
/**
 * @brief Thrown when a {@link RebalanceOperation} occurs concurrently with a
 *transaction.
 * This can be thrown while doing transactional operations or during commit.
 **/
class APACHE_GEODE_EXPORT TransactionDataRebalancedException
    : public Exception {
 public:
  using Exception::Exception;
  virtual ~TransactionDataRebalancedException() noexcept {}
  virtual std::string getName() const override {
    return "apache::geode::client::TransactionDataRebalancedException";
  }
};

/**
 * @brief Thrown if putAll operation with single hop succeeded partially.
 **/
class APACHE_GEODE_EXPORT PutAllPartialResultException : public Exception {
 public:
  using Exception::Exception;
  virtual ~PutAllPartialResultException() noexcept {}
  virtual std::string getName() const override {
    return "apache::geode::client::PutAllPartialResultException";
  }
};

}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_EXCEPTIONTYPES_H_
