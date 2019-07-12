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

#include <array>

#include <boost/filesystem.hpp>

#include <geode/CacheableString.hpp>

#include "geode/util/LogLevel.hpp"
#include "util/Log.hpp"
#include "util/string.hpp"

using apache::geode::client::Log;
using apache::geode::client::LogLevel;
using apache::geode::client::to_utf16;
using apache::geode::client::to_utf8;
using apache::geode::client::internal::geode_hash;

const int STRING_ARRAY_LENGTH = 3;

int g_iteration = 0;

std::array<const char*, STRING_ARRAY_LENGTH> logStrings{
    "Short test string", "Slightly longer test string",
    "Very long string: "
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
    "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"};

void GeodeLogIntsToConsoleBM(benchmark::State& state) {
  Log::setLogLevel(LogLevel::All);
  std::string intString(std::to_string(g_iteration++));
  for (auto _ : state) {
    Log::debug(intString.c_str());
  }
}

void GeodeLogComboToConsoleBM(benchmark::State& state) {
  g_iteration++;
  Log::setLogLevel(LogLevel::All);
  std::string comboString = std::string(logStrings[g_iteration % 3]) + " " +
                            std::to_string(g_iteration);
  for (auto _ : state) {
    Log::debug(comboString.c_str());
  }
}

void GeodeLogStrings(benchmark::State& state) {
  int index = g_iteration++ % STRING_ARRAY_LENGTH;

  for (auto _ : state) {
    Log::debug(logStrings[index]);
  }
}

typedef std::function<void(benchmark::State&)> benchmarkWorker;

void GeodeLogStringsToConsoleBM(benchmark::State& state) {
  int index = g_iteration++ % STRING_ARRAY_LENGTH;
  Log::setLogLevel(LogLevel::All);

  GeodeLogStrings(state);
}

void GeodeLogStringsToFileBM(benchmark::State& state) {
  boost::filesystem::path sourcePath(__FILE__);
  auto filename = std::string("geode_native_") + sourcePath.stem().string() +
                  std::to_string(__LINE__) + ".log";
  boost::filesystem::path logPath(filename);

  Log::init(LogLevel::All, filename.c_str());

  GeodeLogStrings(state);

  if (boost::filesystem::exists(logPath)) {
    boost::filesystem::remove(logPath);
  }
}

BENCHMARK(GeodeLogStringsToConsoleBM)->Range(8, 8 << 10);
// BENCHMARK(GeodeLogIntsToConsoleBM)->Range(8, 8 << 10);
// BENCHMARK(GeodeLogComboToConsoleBM)->Range(8, 8 << 10);
BENCHMARK(GeodeLogStringsToFileBM)->Range(8, 8 << 10);
