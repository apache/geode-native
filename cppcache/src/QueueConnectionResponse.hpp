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

#ifndef GEODE_QUEUECONNECTIONRESPONSE_H_
#define GEODE_QUEUECONNECTIONRESPONSE_H_

#include <list>

#include <geode/DataInput.hpp>

#include "ServerLocation.hpp"
#include "ServerLocationResponse.hpp"

namespace apache {
namespace geode {
namespace client {

using internal::DSFid;

class QueueConnectionResponse : public ServerLocationResponse {
 public:
  QueueConnectionResponse()
      : ServerLocationResponse(), m_durableQueueFound(false) {}

  ~QueueConnectionResponse() override = default;

  void fromData(DataInput& input) override;

  DSFid getDSFID() const override;

  virtual std::list<ServerLocation> getServers() { return m_list; }

  virtual bool isDurableQueueFound() { return m_durableQueueFound; }

  static std::shared_ptr<Serializable> create() {
    return std::make_shared<QueueConnectionResponse>();
  }

 private:
  void readList(DataInput& input);
  std::list<ServerLocation> m_list;
  bool m_durableQueueFound;
};

}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_QUEUECONNECTIONRESPONSE_H_
