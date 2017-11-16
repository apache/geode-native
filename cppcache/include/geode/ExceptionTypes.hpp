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

/**
 * @file
 */

#include "geode_globals.hpp"
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
class AssertionException : public Exception {
  using Exception::Exception;
};

/**
 *@brief Thrown when an argument to a method is illegal.
 **/
class IllegalArgumentException : public Exception {
  using Exception::Exception;
};

/**
 *@brief Thrown when the state of cache is manipulated to be illegal.
 **/
class IllegalStateException : public Exception {
  using Exception::Exception;
};

/**
 *@brief Thrown when an attempt is made to create an existing cache.
 **/
class CacheExistsException : public Exception {
  using Exception::Exception;
};

/**
 *@brief Thrown when the cache xml is incorrect.
 **/
class CacheXmlException : public Exception {
  using Exception::Exception;
};
/**
 *@brief Thrown when a timout occurs.
 **/
class TimeoutException : public Exception {
  using Exception::Exception;
};

/**
 *@brief Thrown when the cache writer aborts the operation.
 **/
class CacheWriterException : public Exception {
  using Exception::Exception;
};

/**
 *@brief Thrown when an attempt is made to create an existing region.
 **/
class RegionExistsException : public Exception {
  using Exception::Exception;
};

/**
 *@brief Thrown when an operation is attempted on a closed cache.
 **/
class CacheClosedException : public Exception {
  using Exception::Exception;
};

/**
 *@brief Thrown when lease of cache proxy has expired.
 **/
class LeaseExpiredException : public Exception {
  using Exception::Exception;
};

/**
 *@brief Thrown when the cache loader aborts the operation.
 **/
class CacheLoaderException : public Exception {
  using Exception::Exception;
};

/**
 *@brief Thrown when an operation is attempted on a destroyed region.
 **/
class RegionDestroyedException : public Exception {
  using Exception::Exception;
};

/**
 *@brief Thrown when an operation is attempted on a destroyed entry.
 **/
class EntryDestroyedException : public Exception {
  using Exception::Exception;
};

/**
 *@brief Thrown when the connecting target is not running.
 **/
class NoSystemException : public Exception {
  using Exception::Exception;
};

/**
 *@brief Thrown when an attempt is made to connect to
 *       DistributedSystem second time.
 **/
class AlreadyConnectedException : public Exception {
  using Exception::Exception;
};

/**
 *@brief Thrown when a non-existing file is accessed.
 **/
class FileNotFoundException : public Exception {
  using Exception::Exception;
};

/**
 *@brief Thrown when an operation is interrupted.
 **/
class InterruptedException : public Exception {
  using Exception::Exception;
};

/**
 *@brief Thrown when an operation unsupported by the
 *       current configuration is attempted.
 **/
class UnsupportedOperationException : public Exception {
  using Exception::Exception;
};

/**
 *@brief Thrown when statistics are invoked for a region where
 *       they are disabled.
 **/
class StatisticsDisabledException : public Exception {
  using Exception::Exception;
};

/**
 *@brief Thrown when a concurrent operation fails.
 **/
class ConcurrentModificationException : public Exception {
  using Exception::Exception;
};

/**
 *@brief An unknown exception occurred.
 **/
class UnknownException : public Exception {
  using Exception::Exception;
};

/**
 *@brief Thrown when a cast operation fails.
 **/
class ClassCastException : public Exception {
  using Exception::Exception;
};

/**
 *@brief Thrown when an operation is attempted on a non-existent entry.
 **/
class EntryNotFoundException : public Exception {
  using Exception::Exception;
};

/**
 *@brief Thrown when there is an input/output error.
 **/
class GeodeIOException : public Exception {
  using Exception::Exception;
};

/**
 *@brief Thrown when geode configuration file is incorrect.
 **/
class GeodeConfigException : public Exception {
  using Exception::Exception;
};

/**
 *@brief Thrown when a null argument is provided to a method
 *       where it is expected to be non-null.
 **/
class NullPointerException : public Exception {
  using Exception::Exception;
};

/**
 *@brief Thrown when attempt is made to create an existing entry.
 **/
class EntryExistsException : public Exception {
  using Exception::Exception;
};

/**
 *@brief Thrown when an operation is attempted before connecting
 *       to the distributed system.
 **/
class NotConnectedException : public Exception {
  using Exception::Exception;
};

/**
 *@brief Thrown when there is an error in the cache proxy.
 **/
class CacheProxyException : public Exception {
  using Exception::Exception;
};

/**
 *@brief Thrown when the system cannot allocate any more memory.
 **/
class OutOfMemoryException : public Exception {
  using Exception::Exception;
};

/**
 *@brief Thrown when an attempt is made to release a lock not
 *       owned by the thread.
 **/
class NotOwnerException : public Exception {
  using Exception::Exception;
};

/**
 *@brief Thrown when a region is created in an incorrect scope.
 **/
class WrongRegionScopeException : public Exception {
  using Exception::Exception;
};

/**
 *@brief Thrown when the internal buffer size is exceeded.
 **/
class BufferSizeExceededException : public Exception {
  using Exception::Exception;
};

/**
 *@brief Thrown when a region creation operation fails.
 **/
class RegionCreationFailedException : public Exception {
  using Exception::Exception;
};

/**
 *@brief Thrown when there is a fatal internal exception in geode.
 */
class FatalInternalException : public Exception {
  using Exception::Exception;
};

/**
 *@brief Thrown by the persistence manager when a write
 *       fails due to disk failure.
 **/
class DiskFailureException : public Exception {
  using Exception::Exception;
};

/**
 *@brief Thrown by the persistence manager when the data
 *@brief to be read from disk is corrupt.
 **/
class DiskCorruptException : public Exception {
  using Exception::Exception;
};

/**
 *@brief Thrown when persistence manager fails to initialize.
 **/
class InitFailedException : public Exception {
  using Exception::Exception;
};

/**
 *@brief Thrown when persistence manager fails to close properly.
 **/
class ShutdownFailedException : public Exception {
  using Exception::Exception;
};

/**
 *@brief Thrown when an exception occurs on the cache server.
 **/
class CacheServerException : public Exception {
  using Exception::Exception;
};

/**
 *@brief Thrown when bound of array/vector etc. is exceeded.
 **/
class OutOfRangeException : public Exception {
  using Exception::Exception;
};

/**
 *@brief Thrown when query exception occurs at the server.
 **/
class QueryException : public Exception {
  using Exception::Exception;
};

/**
 *@brief Thrown when an unknown message is received from the server.
 **/
class MessageException : public Exception {
  using Exception::Exception;
};

/**
 *@brief Thrown when a non authorized operation is done.
 **/
class NotAuthorizedException : public Exception {
  using Exception::Exception;
};

/**
 *@brief Thrown when authentication fails.
 **/
class AuthenticationFailedException : public Exception {
  using Exception::Exception;
};

/**
 *@brief Thrown when no credentials are provided by client when server expects.
 **/
class AuthenticationRequiredException : public Exception {
  using Exception::Exception;
};

/**
 *@brief Thrown when two durable connect with same Id.
 **/
class DuplicateDurableClientException : public Exception {
  using Exception::Exception;
};

/**
 *@brief Thrown when the cache listener throws an exception.
 **/
class CacheListenerException : public Exception {
  using Exception::Exception;
};
/**
 *@brief Thrown during continuous query execution time.
 **/
class CqException : public Exception {
  using Exception::Exception;
};
/**
 *@brief Thrown if the Cq on which the operaion performed is closed
 **/
class CqClosedException : public Exception {
  using Exception::Exception;
};
/**
 *@brief Thrown if the Cq Query failed
 **/
class CqQueryException : public Exception {
  using Exception::Exception;
};
/**
 *@brief Thrown if a Cq by this name already exists on this client
 **/
class CqExistsException : public Exception {
  using Exception::Exception;
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
class CqInvalidException : public Exception {
  using Exception::Exception;
};
/**
 *@brief Thrown if function execution failed
 **/
class FunctionExecutionException : public Exception {
  using Exception::Exception;
};
/**
 *@brief Thrown if the No locators are active to reply for new connection.
 **/
class NoAvailableLocatorsException : public Exception {
  using Exception::Exception;
};
/**
 *@brief Thrown if all connections in the pool are in use.
 **/
class AllConnectionsInUseException : public Exception {
  using Exception::Exception;
};
/**
 *@brief Thrown if Delta could not be applied.
 **/
class InvalidDeltaException : public Exception {
  using Exception::Exception;
};
/**
 *@brief Thrown if a Key is not present in the region.
 **/
class KeyNotFoundException : public Exception {
  using Exception::Exception;
};
/**
 * @brief This is for all Exceptions that may be thrown
 * by a Geode transaction.
 **/
class TransactionException : public Exception {
  using Exception::Exception;
};
/**
 * @brief The RollbackException exception indicates that either the transaction
 * has been rolled back or an operation cannot complete because the
 * transaction is marked for rollback only.
 **/
class RollbackException : public Exception {
  using Exception::Exception;
};
/**
 * @brief Thrown when a commit fails due to a write conflict.
 * @see CacheTransactionManager#commit
 **/
class CommitConflictException : public Exception {
  using Exception::Exception;
};
/**
 * @brief Thrown when the transactional data host has shutdown or no longer has
 *the data
 * being modified by the transaction.
 * This can be thrown while doing transactional operations or during commit.
 **/
class TransactionDataNodeHasDepartedException : public Exception {
  using Exception::Exception;
};
/**
 * @brief Thrown when a {@link RebalanceOperation} occurs concurrently with a
 *transaction.
 * This can be thrown while doing transactional operations or during commit.
 **/
class TransactionDataRebalancedException : public Exception {
  using Exception::Exception;
};

/**
 * @brief Thrown if putAll operation with single hop succeeded partially.
 **/
class PutAllPartialResultException : public Exception {
  using Exception::Exception;
};

/**
 *@brief Thrown when a version on which delta is based is different than the
 *current version
 **/

extern void CPPCACHE_EXPORT GfErrTypeThrowException(const char* str,
                                                    GfErrType err);

#define GfErrTypeToException(str, err)   \
  {                                      \
    if (err != GF_NOERR) {               \
      GfErrTypeThrowException(str, err); \
    }                                    \
  }
}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_EXCEPTIONTYPES_H_
