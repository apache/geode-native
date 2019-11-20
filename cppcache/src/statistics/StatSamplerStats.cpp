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

#include "StatSamplerStats.hpp"

#include "StatisticsManager.hpp"

namespace apache {
namespace geode {
namespace statistics {

StatSamplerStats::StatSamplerStats(StatisticsFactory* statFactory) {
  std::vector<std::shared_ptr<StatisticDescriptor>> statDescriptorArr(2);
  statDescriptorArr[0] = statFactory->createIntCounter(
      "sampleCount", "Total number of samples taken by this sampler.",
      "samples", false);

  statDescriptorArr[1] = statFactory->createLongCounter(
      "sampleTime", "Total amount of time spent taking samples.",
      "milliseconds", false);

  samplerType =
      statFactory->createType("StatSampler", "Stats on the statistic sampler.",
                              std::move(statDescriptorArr));
  sampleCountId = samplerType->nameToId("sampleCount");
  sampleTimeId = samplerType->nameToId("sampleTime");
  this->samplerStats = statFactory->createStatistics(samplerType, "statSampler",
                                                     statFactory->getId());
}

void StatSamplerStats::setInitialValues() {
  if (samplerStats) {
    samplerStats->setInt(sampleCountId, 0);
    samplerStats->setLong(sampleTimeId, 0);
  }
}

void StatSamplerStats::tookSample(int64_t nanosSpentWorking) {
  if (samplerStats) {
    samplerStats->incInt(sampleCountId, 1);
    samplerStats->incLong(sampleTimeId, nanosSpentWorking / 1000000);
  }
}

void StatSamplerStats::close() {
  if (samplerStats) {
    samplerStats->close();
  }
}

StatSamplerStats::~StatSamplerStats() {
  samplerType = nullptr;
  samplerStats = nullptr;
}

}  // namespace statistics
}  // namespace geode
}  // namespace apache
