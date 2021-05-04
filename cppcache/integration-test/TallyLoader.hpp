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

#ifndef GEODE_INTEGRATION_TEST_TALLYLOADER_H_
#define GEODE_INTEGRATION_TEST_TALLYLOADER_H_

namespace apache {
namespace geode {
namespace client {
namespace testing {

using apache::geode::client::Cacheable;
using apache::geode::client::CacheableInt32;
using apache::geode::client::CacheableKey;
using apache::geode::client::CacheLoader;
using apache::geode::client::Region;
using apache::geode::client::Serializable;

class TallyLoader : virtual public CacheLoader {
 private:
  int32_t m_loads;

 public:
  TallyLoader() : CacheLoader(), m_loads(0) {}
  ~TallyLoader() noexcept override = default;

  std::shared_ptr<Cacheable> load(
      Region&, const std::shared_ptr<CacheableKey>&,
      const std::shared_ptr<Serializable>&) override {
    LOG_DEBUG("TallyLoader::load invoked for %d.", m_loads);
    char buf[1024];
    sprintf(buf, "TallyLoader state: (loads = %d)", m_loads);
    LOG(buf);
    return CacheableInt32::create(m_loads++);
  }

  virtual void close(Region&) override { LOG("TallyLoader::close"); }

  int getLoads() { return m_loads; }

  void reset() { m_loads = 0; }
};

}  // namespace testing
}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_INTEGRATION_TEST_TALLYLOADER_H_
