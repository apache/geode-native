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

static int g_iteration = 0;

static std::array<const char*, STRING_ARRAY_LENGTH> logStrings{
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

void GeodeLogStrings(benchmark::State& state) {
  int index = g_iteration++ % STRING_ARRAY_LENGTH;

  for (auto _ : state) {
    LOG_DEBUG(logStrings[index]);
  }
}

void GeodeLogInts(benchmark::State& state) {
  std::string intString(std::to_string(g_iteration++));
  for (auto _ : state) {
    LOG_DEBUG(intString.c_str());
  }
}

void GeodeLogCombo(benchmark::State& state) {
  g_iteration++;
  std::string comboString = std::string(logStrings[g_iteration % 3]) + " " +
                            std::to_string(g_iteration);
  for (auto _ : state) {
    LOG_DEBUG(comboString.c_str());
  }
}

template <void T(benchmark::State&)>
void GeodeLogToConsole(benchmark::State& state) {
  Log::init(LogLevel::All);

  T(state);

  Log::close();
}

template <void T(benchmark::State&)>
void GeodeLogToFile(benchmark::State& state) {
  boost::filesystem::path sourcePath(__FILE__);
  auto filename = std::string("geode_native_") + sourcePath.stem().string() +
                  std::to_string(__LINE__) + ".log";
  boost::filesystem::path logPath(filename);

  Log::init(LogLevel::All, filename.c_str());

  T(state);

  Log::close();

  if (boost::filesystem::exists(logPath)) {
    boost::filesystem::remove(logPath);
  }
}

static const auto kLogStringsToConsole = GeodeLogToConsole<GeodeLogStrings>;
static const auto kLogIntsToConsole = GeodeLogToConsole<GeodeLogInts>;
static const auto kLogComboToConsole = GeodeLogToConsole<GeodeLogCombo>;

static const auto kLogStringsToFile = GeodeLogToFile<GeodeLogStrings>;
static const auto kLogIntsToFile = GeodeLogToFile<GeodeLogInts>;
static const auto kLogComboToFile = GeodeLogToFile<GeodeLogCombo>;

BENCHMARK(kLogStringsToConsole)->Range(8, 8 << 10);
BENCHMARK(kLogIntsToConsole)->Range(8, 8 << 10);
BENCHMARK(kLogComboToConsole)->Range(8, 8 << 10);
BENCHMARK(kLogStringsToFile)->Range(8, 8 << 10);
BENCHMARK(kLogIntsToFile)->Range(8, 8 << 10);
BENCHMARK(kLogComboToFile)->Range(8, 8 << 10);
