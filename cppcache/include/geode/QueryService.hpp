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

#ifndef GEODE_QUERYSERVICE_H_
#define GEODE_QUERYSERVICE_H_

#include <memory>
#include <string>

#include "CqAttributes.hpp"
#include "CqQuery.hpp"
#include "CqServiceStatistics.hpp"
#include "ExceptionTypes.hpp"
#include "Query.hpp"
#include "internal/geode_globals.hpp"

/**
 * @file
 */

namespace apache {
namespace geode {
namespace client {

/**
 * @class QueryService QueryService.hpp
 * QueryService is the class obtained from a Cache.
 * A Query is created from a QueryService and executed on the server
 * returning a SelectResults which can be either a ResultSet or a StructSet.
 */
class APACHE_GEODE_EXPORT QueryService {
 public:
  typedef std::vector<std::shared_ptr<CqQuery>> query_container_type;

  /**
   * Get a new Query with the specified query string.
   *
   * @param querystr The query string with which to create a new Query.
   * @returns A smart pointer to the Query.
   */
  virtual std::shared_ptr<Query> newQuery(std::string querystr) = 0;

  /**
   * Constructs a new named continuous query, represented by an instance of
   * CqQuery. The CqQuery is not executed, however, until the execute method
   * is invoked on the CqQuery. The name of the query will be used
   * to identify this query in statistics archival.
   *
   * @param name the String name for this query
   * @param querystr the OQL query
   * @param cqAttr the CqAttributes
   * @param isDurable true if the CQ is durable
   * @return the newly created CqQuery object
   * @throws CqExistsException if a CQ by this name already exists on this
   * client
   * @throws IllegalArgumentException if queryString is null, or cqAttr is
   * nullptr
   * @throws IllegalStateException if this method is called from a cache
   *         server
   * @throws QueryInvalidException if there is a syntax error in the query
   * @throws CqException if failed to create cq, failure during creating
   *         managing cq metadata info.
   * @throws CqInvalidException if the query doesnot meet the CQ constraints.
   *   E.g.: Query string should refer only one region, join not supported.
   *         The query must be a SELECT statement.
   *         DISTINCT queries are not supported.
   *         Projections are not supported.
   *         Only one iterator in the FROM clause is supported, and it must be a
   * region path.
   *         Bind parameters in the query are not supported for the initial
   * release.
   */
  virtual std::shared_ptr<CqQuery> newCq(
      std::string name, std::string querystr,
      const std::shared_ptr<CqAttributes>& cqAttr, bool isDurable = false) = 0;

  /**
   * @nativeclient
   * Constructs a new named continuous query, represented by an instance of
   * CqQuery. The CqQuery is not executed, however, until the execute method
   * is invoked on the CqQuery. The name of the query will be used
   * to identify this query in statistics archival.
   *
   * @param querystr the OQL query
   * @param cqAttr the CqAttributes
   * @param isDurable true if the CQ is durable
   * @return the newly created CqQuery object
   * @throws CqExistsException if a CQ by this name already exists on this
   * client
   * @throws IllegalArgumentException if queryString is null, or cqAttr is
   * nullptr
   * @throws IllegalStateException if this method is called from a cache
   *         server
   * @throws QueryInvalidException if there is a syntax error in the query
   * @throws CqException if failed to create cq, failure during creating
   *         managing cq metadata info.
   * @throws CqInvalidException if the query doesnot meet the CQ constraints.
   *   E.g.: Query string should refer only one region, join not supported.
   *         The query must be a SELECT statement.
   *         DISTINCT queries are not supported.
   *         Projections are not supported.
   *         Only one iterator in the FROM clause is supported, and it must be a
   * region path.
   *         Bind parameters in the query are not supported for the initial
   * release.
   *
   * @endnativeclient
   */
  virtual std::shared_ptr<CqQuery> newCq(
      std::string querystr, const std::shared_ptr<CqAttributes>& cqAttr,
      bool isDurable = false) = 0;

  /**
   * Close all CQs, and release resources
   * associated with executing CQs.
   */
  virtual void closeCqs() = 0;

  /**
   * @nativeclient
   * Retrieve  all registered CQs
   * @endnativeclient
   */
  virtual query_container_type getCqs() const = 0;

  /**
   * Retrieve a CqQuery by name.
   * @return the CqQuery or nullptr if not found
   */
  virtual std::shared_ptr<CqQuery> getCq(const std::string& name) const = 0;

  /**
   * @nativeclient
   * Executes all the cqs on this client.
   * @endnativeclient
   */
  virtual void executeCqs() = 0;

  /**
   * Stops all the cqs on this client.
   */
  virtual void stopCqs() = 0;

  /**
   * @nativeclient
   * Get statistics information for all CQs
   * @return the CqServiceStatistics
   * @endnativeclient
   */
  virtual std::shared_ptr<CqServiceStatistics> getCqServiceStatistics()
      const = 0;

  /**
   * Gets all the durable CQs registered by this client.
   *
   * @return List of names of registered durable CQs, empty list if no durable
   * cqs.
   */
  virtual std::shared_ptr<CacheableArrayList> getAllDurableCqsFromServer()
      const = 0;
};
}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_QUERYSERVICE_H_
