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

#ifndef GEODE_REMOTEQUERY_H_
#define GEODE_REMOTEQUERY_H_

#include <memory>
#include <string>

#include <geode/AuthenticatedView.hpp>
#include <geode/ExceptionTypes.hpp>
#include <geode/Query.hpp>
#include <geode/ResultSet.hpp>
#include <geode/SelectResults.hpp>
#include <geode/StructSet.hpp>
#include <geode/internal/geode_globals.hpp>

#include "CacheImpl.hpp"

namespace apache {
namespace geode {
namespace client {

class ThinClientBaseDM;

class RemoteQuery : public Query {
  std::string m_queryString;
  std::shared_ptr<RemoteQueryService> m_queryService;
  ThinClientBaseDM* m_tccdm;
  AuthenticatedView* m_authenticatedView;

 public:
  RemoteQuery(std::string querystr,
              const std::shared_ptr<RemoteQueryService>& queryService,
              ThinClientBaseDM* tccdmptr,
              AuthenticatedView* authenticatedView = nullptr);

  ~RemoteQuery() noexcept override = default;

  std::shared_ptr<SelectResults> execute(
      std::chrono::milliseconds timeout =
          DEFAULT_QUERY_RESPONSE_TIMEOUT) override;

  std::shared_ptr<SelectResults> execute(
      std::shared_ptr<CacheableVector> paramList = nullptr,
      std::chrono::milliseconds timeout =
          DEFAULT_QUERY_RESPONSE_TIMEOUT) override;

  /**
   * executes a query using a given distribution manager
   * used by Region.query() and Region.getAll()
   */
  std::shared_ptr<SelectResults> execute(
      std::chrono::milliseconds timeout, const char* func,
      ThinClientBaseDM* tcdm, std::shared_ptr<CacheableVector> paramList);

  // nothrow version of execute()
  GfErrType executeNoThrow(std::chrono::milliseconds timeout,
                           TcrMessageReply& reply, const char* func,
                           ThinClientBaseDM* tcdm,
                           std::shared_ptr<CacheableVector> paramList);

  const std::string& getQueryString() const override;

  void compile() override;

  bool isCompiled() override;
};

}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_REMOTEQUERY_H_
