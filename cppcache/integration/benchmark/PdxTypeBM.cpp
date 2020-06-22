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

#include <benchmark/benchmark.h>
#include <framework/Cluster.h>

static void PdxTypeBM_createInstance(benchmark::State& state) {
  const std::string gemfireJsonClassName = "__GEMFIRE_JSON";

  Cluster cluster{Name("PdxTypeBM"), LocatorCount{1}, ServerCount{1}};
  cluster.start();
  cluster.getGfsh().create();

  auto cache = cluster.createCache();
  auto repetitions = 1000;

  for (auto _ : state) {
    for (auto i = 0; i < repetitions; ++i) {
      auto instance_factory =
          cache.createPdxInstanceFactory(gemfireJsonClassName);
      instance_factory.writeString("bar", std::to_string(i));
      auto instance = instance_factory.create();
    }
  }
}

BENCHMARK(PdxTypeBM_createInstance)->UseRealTime();
