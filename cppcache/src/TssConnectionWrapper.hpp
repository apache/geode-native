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

#ifndef GEODE_TSSCONNECTIONWRAPPER_H_
#define GEODE_TSSCONNECTIONWRAPPER_H_

#include <map>
#include <memory>
#include <string>

namespace apache {
namespace geode {
namespace client {

class Pool;
class TcrEndpoint;
class TcrConnection;

class PoolWrapper {
 private:
  std::map<std::string, TcrConnection*> endpointsToConnectionMap_;

 public:
  PoolWrapper() = default;
  ~PoolWrapper() noexcept = default;
  PoolWrapper(const PoolWrapper&) = delete;
  PoolWrapper& operator=(const PoolWrapper&) = delete;

  TcrConnection* getSHConnection(const TcrEndpoint& endpoint);

  void setSHConnection(const TcrEndpoint& ep, TcrConnection* connection);

  void releaseSHConnections(Pool& pool);

  TcrConnection* getAnyConnection();
};

class TssConnectionWrapper {
 private:
  thread_local static TssConnectionWrapper instance_;

  TcrConnection* connection_;
  std::shared_ptr<Pool> pool_;
  std::map<std::string, PoolWrapper*> poolNameToPoolWrapperMap_;

  TssConnectionWrapper();
  ~TssConnectionWrapper();

 public:
  inline static TssConnectionWrapper& get() { return instance_; }

  TssConnectionWrapper& operator=(const TssConnectionWrapper&) = delete;
  TssConnectionWrapper(const TssConnectionWrapper&) = delete;

  inline void setConnection(TcrConnection* connection,
                            const std::shared_ptr<Pool>& pool) {
    connection_ = connection;
    pool_ = pool;
  }

  inline TcrConnection* getConnection() const { return connection_; }

  inline TcrConnection** getConnDoublePtr() { return &connection_; }

  TcrConnection* getSHConnection(const TcrEndpoint& endpoint,
                                 const std::string& poolName);

  void setSHConnection(const TcrEndpoint& endpoint, TcrConnection* connection);

  void releaseSHConnections(Pool& pool);

  TcrConnection* getAnyConnection(const std::string& poolName) const;
};

}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_TSSCONNECTIONWRAPPER_H_
