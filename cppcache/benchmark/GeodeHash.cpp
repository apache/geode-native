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

#include <geode/CacheableString.hpp>

using namespace apache::geode::client;

class GeodeHashBM : public benchmark::Fixture {};

BENCHMARK_DEFINE_F(GeodeHashBM, std_string)(benchmark::State& state) {
  std::string x(state.range(0), 'x');
  for (auto _ : state) {
    int hashcode;
    benchmark::DoNotOptimize(hashcode = geode_hash<std::string>{}(x));
  }
}
BENCHMARK_REGISTER_F(GeodeHashBM, std_string)->Range(8, 8 << 10);

BENCHMARK_DEFINE_F(GeodeHashBM, std_u16string)(benchmark::State& state) {
  std::u16string x(state.range(0), u'x');
  for (auto _ : state) {
    int hashcode;
    benchmark::DoNotOptimize(hashcode = geode_hash<std::u16string>{}(x));
  }
}
BENCHMARK_REGISTER_F(GeodeHashBM, std_u16string)->Range(8, 8 << 10);
