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

#include <CacheImpl.hpp>
#include <TcrConnection.hpp>
#include <TcrConnectionManager.hpp>

#include <gtest/gtest.h>

namespace {

using apache::geode::client::CacheImpl;
using apache::geode::client::TcrConnection;
using apache::geode::client::TcrConnectionManager;

class TcrConnectionTest : public TcrConnection {
 public:
  explicit TcrConnectionTest(const TcrConnectionManager& manager)
      : TcrConnection(manager) {}

  int getExpiryTimeVariancePercentage() {
    return expiryTimeVariancePercentage_;
  }
};

TEST(
    TcrConnectionTest,
    getExpiryTimeVariancePercentageReturnsUniformRandomBetweenMinusNineAndNine) {
  // Create several connections at the same time
  const int connections = 1000;
  std::unique_ptr<TcrConnectionTest> tcrConnections[connections];
  for (int i = 0; i < connections; i++) {
    tcrConnections[i] =
        std::unique_ptr<TcrConnectionTest>(new TcrConnectionTest(
            static_cast<const TcrConnectionManager>(nullptr)));
  }

  const int variancesPositions = 19;
  int variances[variancesPositions];
  for (int i = 0; i < variancesPositions; i++) {
    variances[i] = 0;
  }

  // Check that the variance of the connections lies between -9 and 9
  for (int i = 0; i < connections; i++) {
    int variance = tcrConnections[i]->getExpiryTimeVariancePercentage();
    EXPECT_LE(abs(variance), 9);
    variances[variance + 9]++;
  }

  // Check that the variance is distributed uniformly
  float tolerance = 0.5;
  int minOccurrences = connections / variancesPositions -
                       (connections / variancesPositions) * tolerance;
  int maxOccurrences = connections / variancesPositions +
                       (connections / variancesPositions) * tolerance;
  for (int i = 0; i < variancesPositions; i++) {
    EXPECT_GT(variances[i], minOccurrences);
    EXPECT_LT(variances[i], maxOccurrences);
  }
}

}  // namespace
