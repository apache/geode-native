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

#ifndef GEODE_ERRTYPE_H_
#define GEODE_ERRTYPE_H_

/**
 * @enum GfErrType
 *Error codes returned by Geode C++ interface functions
 */
typedef enum {
  GF_NOERR = 0,           /**< success - no error               */
  GF_DEADLK = 1,          /**< deadlock detected                */
  GF_EACCES = 2,          /**< permission problem               */
  GF_ECONFL = 3,          /**< class creation conflict          */
  GF_EINVAL = 4,          /**< invalid argument                 */
  GF_ENOENT = 5,          /**< entity does not exist            */
  GF_ENOMEM = 6,          /**< insufficient memory              */
  GF_ERANGE = 7,          /**< index out of range               */
  GF_ETYPE = 8,           /**< type mismatch                    */
  GF_NOTOBJ = 9,          /**< invalid object reference         */
  GF_NOTCON = 10,         /**< not connected to Geode         */
  GF_NOTOWN = 11,         /**< lock not owned by process/thread */
  GF_NOTSUP = 12,         /**< operation not supported          */
  GF_SCPGBL = 13,         /**< attempt to exit global scope     */
  GF_SCPEXC = 14,         /**< maximum scopes exceeded          */
  GF_TIMEOUT = 15,        /**< operation timed out              */
  GF_OVRFLW = 16,         /**< arithmetic overflow              */
  GF_IOERR = 17,          /**< paging file I/O error            */
  GF_EINTR = 18,          /**< interrupted Geode call         */
  GF_MSG = 19,            /**< message could not be handled     */
  GF_DISKFULL = 20,       /**< disk full                        */
  GF_NOSERVER_FOUND = 21, /** NoServer found */
  GF_SERVER_FAILED = 22,

  GF_CLIENT_WAIT_TIMEOUT = 23,
  GF_CLIENT_WAIT_TIMEOUT_REFRESH_PRMETADATA = 24,

  GF_CACHE_REGION_NOT_FOUND = 101, /**< No region with the specified name. */
  GF_CACHE_REGION_INVALID = 102,   /**< the region is not valid */
  GF_CACHE_REGION_KEYS_NOT_STRINGS = 103, /**< Entry keys are not strings */
  GF_CACHE_REGION_ENTRY_NOT_BYTES =
      104,                          /**< Entry's value is not a byte array */
  GF_CACHE_REGION_NOT_GLOBAL = 105, /**< Distributed locks not supported */
  GF_CACHE_PROXY = 106, /**< Errors detected in CacheProxy processing */
  GF_CACHE_ILLEGAL_ARGUMENT_EXCEPTION =
      107, /**< IllegalArgumentException in Cache Proxy */
  GF_CACHE_ILLEGAL_STATE_EXCEPTION =
      108,                          /**< IllegalStateException in CacheProxy */
  GF_CACHE_TIMEOUT_EXCEPTION = 109, /**< TimeoutException in CacheProxy */
  GF_CACHE_WRITER_EXCEPTION = 110,  /**< CacheWriterException in CacheProxy */
  GF_CACHE_REGION_EXISTS_EXCEPTION =
      111,                         /**< RegionExistsException in CacheProxy */
  GF_CACHE_CLOSED_EXCEPTION = 112, /**< CacheClosedException in CacheProxy */
  GF_CACHE_LEASE_EXPIRED_EXCEPTION =
      113,                         /**< LeaseExpiredException in CacheProxy */
  GF_CACHE_LOADER_EXCEPTION = 114, /**< CacheLoaderException in CacheProxy */
  GF_CACHE_REGION_DESTROYED_EXCEPTION =
      115, /**< RegionDestroyedException in CacheProxy */
  GF_CACHE_ENTRY_DESTROYED_EXCEPTION =
      116, /**< EntryDestroyedException in CacheProxy */
  GF_CACHE_STATISTICS_DISABLED_EXCEPTION =
      117, /**< StatisticsDisabledException in CacheProxy */
  GF_CACHE_CONCURRENT_MODIFICATION_EXCEPTION =
      118, /**< ConcurrentModificationException in CacheProxy */
  GF_CACHE_ENTRY_NOT_FOUND = 119, /**< EntryNotFoundException in CacheProxy */
  GF_CACHE_ENTRY_EXISTS = 120,    /**< EntryExistsException in CacheProxy */
  GF_CACHEWRITER_ERROR =
      121, /**< An Exception occured while invoking a cachewritter callback*/
  GF_CANNOT_PROCESS_GII_REQUEST =
      123, /**< A failure other than timeout occured durring a batch request */
  GF_CACHESERVER_EXCEPTION = 124, /**< Java cache server exception sent to the
                                     thin client by java cache server */
  // GF_CACHE_REDUNDANCY_FAILURE = 125, /**< redundancy level not satisfied */
  GF_AUTHENTICATION_FAILED_EXCEPTION = 126, /**<Authentication Fails */
  GF_NOT_AUTHORIZED_EXCEPTION = 127, /**<Non Authorized Operation Tried */
  GF_AUTHENTICATION_REQUIRED_EXCEPTION = 128, /**No Authentication Provided */
  GF_DUPLICATE_DURABLE_CLIENT =
      129, /**< Java cache server refused duplicate durable client  */
  GF_REMOTE_QUERY_EXCEPTION = 130,   /** Query exception on java cache server */
  GF_CACHE_LISTENER_EXCEPTION = 131, /** Exception in CacheListener */
  GF_ALL_CONNECTIONS_IN_USE_EXCEPTION = 132, /** ALl connections in use*/
                                             /**
                                              * local entry was updated while a remote modification operation was
                                              * in progress
                                              */
  GF_CACHE_ENTRY_UPDATED = 133,
  GF_CACHE_LOCATOR_EXCEPTION = 134, /** Exception in Locator */
  GF_INVALID_DELTA = 135,
  GF_FUNCTION_EXCEPTION = 136,
  GF_ROLLBACK_EXCEPTION = 137,
  GF_COMMIT_CONFLICT_EXCEPTION = 138,
  GF_TRANSACTION_DATA_NODE_HAS_DEPARTED_EXCEPTION = 139,
  GF_TRANSACTION_DATA_REBALANCED_EXCEPTION = 140,
  GF_PUTALL_PARTIAL_RESULT_EXCEPTION = 141,
  GF_EUNDEF = 999 /**< unknown exception */
} GfErrType;

#endif  // GEODE_ERRTYPE_H_
