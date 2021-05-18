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

#include <framework/Cluster.h>
#include <framework/Framework.h>
#include <framework/Gfsh.h>

#include <gtest/gtest.h>

class ThinClient : ::testing::Test {
  class OperMonitor : public CacheListener {
    int m_events;
    int m_value;

    void check(const EntryEvent &event) {
      char buf[256] = {'\0'};
      m_events++;
      auto keyPtr = std::dynamic_pointer_cast<CacheableString>(event.getKey());
      auto valuePtr =
          std::dynamic_pointer_cast<CacheableInt32>(event.getNewValue());

      if (valuePtr != nullptr) {
        m_value = valuePtr->value();
      }
      sprintf(buf, "Key = %s, Value = %d", keyPtr->toString().c_str(),
              valuePtr->value());
      LOG(buf);
    }

   public:
    OperMonitor() : m_events(0), m_value(0) {}
    ~OperMonitor() {}

    virtual void afterCreate(const EntryEvent &event) { check(event); }

    virtual void afterUpdate(const EntryEvent &event) { check(event); }

    void validate(bool conflation) {
      LOG("validate called");
      char buf[256] = {'\0'};

      if (conflation) {
        sprintf(buf, "Conflation On: Expected events = 2, Actual = %d",
                m_events);
        ASSERT(m_events == 2, buf);
      } else {
        sprintf(buf, "Conflation Off: Expected events = 5, Actual = %d",
                m_events);
        ASSERT(m_events == 5, buf);
      }
      sprintf(buf, "Expected Value = 5, Actual = %d", m_value);
      ASSERT(m_value == 5, buf);
    }
  };

  void configure_pool(apache::geode::client::Cache &cache) {
    auto poolFactory = cache.getPoolManager()
                           .createFactory()
                           .setSubscriptionRedundancy(0)
                           .setSubscriptionEnabled(true)
                           .setSubscriptionAckInterval(std::chrono::seconds(1));

    cluster_.applyLocators(poolFactory);

    poolFactory.create("__TESTPOOL1_");
  }

  void configure_regions(apache::geode::client::Cache &cache) {
    auto conflated_regionFactory =
        conflated_cache_.createRegionFactory(RegionShortcut::CACHING_PROXY)
            .setPoolName("__TESTPOOL1_");

    auto conflated = regionFactory.create("ConflatedRegion");
    auto non_conflated = regionFactory.create("NonConflatedRegion");

    conflated->getAttributesMutator()->setCacheListener(
        std::make_shared<OperMonitor>());
    non - conflated->getAttributesMutator()->setCacheListener(
              std::make_shared<OperMonitor>());
  }

  void configure_cache(apache::geode::client::Cache &cache) {
    configure_pool(cache);
    configure_regions(cache);
  }

 protected:
  Cluster cluster_{LocatorCount{1}, ServerCount{1}};
  apache::geode::client::Cache conflated_cache_{
      apache::geode::client::CacheFactory()
          .set("log-level", "none")
          .set("statistic-sampling-enabled", "false")
          .set("durable-client-id", "DurableId1")
          .set("durable-timeout", std::chrono::seconds(300))
          .set("conflate-events", "true")
          .create()},
      non_conflated_cache_{
          apache::geode::client::CacheFactory()
              .set("log-level", "none")
              .set("statistic-sampling-enabled", "false")
              .set("durable-client-id", "DurableId2")
              .set("durable-timeout", std::chrono::seconds(300))
              .set("conflate-events", "false")
              .create()},
      feeder_cache_{apache::geode::client::CacheFactory()
                        .set("log-level", "none")
                        .set("statistic-sampling-enabled", "false")
                        .set("durable-client-id", "DurableId1")
                        .set("durable-timeout", std::chrono::seconds(300))
                        .set("conflate-events", "true")
                        .create()};

  ThinClient() {
    cluster_.start();

    cluster_.getGfsh()
        .create()
        .region()
        .withName("ConflatedRegion")
        .withType("REPLICATE")
        .execute();

    cluster_.getGfsh()
        .create()
        .region()
        .withName("NonConflatedRegion")
        .withType("REPLICATE")
        .execute();

    configure_cache(conflated_cache_);
    configure_cache(non_conflated_cache_);
    configure_cache(feeder_cache_);
  }
};

TEST_F(ThinClient, Conflation) {
  auto conflated = feeder_cache_.getRegion("ConflatedRegion");

  for(auto i = 1; i < 6; ++i) {
    conflated.put(std::string("Key-").append(std::to_string(i)), i);
  }

  auto non_conflated = feeder_cache_.getRegion("NonConflatedRegion");

  for(auto i = 1; i < 6; ++i) {
    non_conflated.put(std::string("Key-").append(std::to_string(i)), i);
  }
}