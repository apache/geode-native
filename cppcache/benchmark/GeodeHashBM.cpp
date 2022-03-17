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

#include "internal/string.hpp"

using apache::geode::client::to_utf16;
using apache::geode::client::to_utf8;
using apache::geode::client::internal::geode_hash;

template <class ToString, class FromString>
ToString convert(const FromString& from);

template <>
std::string convert(const std::u32string& from) {
  return to_utf8(from);
}

template <>
std::u16string convert(const std::u32string& from) {
  return to_utf16(from);
}

template <class String, char32_t UnicodeChar>
void GeodeHashBM(benchmark::State& state) {
  const std::u32string u32String(state.range(0), UnicodeChar);
  const String string = convert<String>(u32String);

  for (auto _ : state) {
    int hashcode;
    benchmark::DoNotOptimize(hashcode = geode_hash<String>{}(string));
  }
}

constexpr char32_t LATIN_CAPITAL_LETTER_C = U'\U00000043';
constexpr char32_t INVERTED_EXCLAMATION_MARK = U'\U000000A1';
constexpr char32_t SAMARITAN_PUNCTUATION_ZIQAA = U'\U00000838';
constexpr char32_t LINEAR_B_SYLLABLE_B008_A = U'\U00010000';

BENCHMARK_TEMPLATE(GeodeHashBM, std::string, LATIN_CAPITAL_LETTER_C)
    ->Range(8, 8 << 10);
BENCHMARK_TEMPLATE(GeodeHashBM, std::u16string, LATIN_CAPITAL_LETTER_C)
    ->Range(8, 8 << 10);
BENCHMARK_TEMPLATE(GeodeHashBM, std::string, INVERTED_EXCLAMATION_MARK)
    ->Range(8, 8 << 10);
BENCHMARK_TEMPLATE(GeodeHashBM, std::u16string, INVERTED_EXCLAMATION_MARK)
    ->Range(8, 8 << 10);
BENCHMARK_TEMPLATE(GeodeHashBM, std::string, SAMARITAN_PUNCTUATION_ZIQAA)
    ->Range(8, 8 << 10);
BENCHMARK_TEMPLATE(GeodeHashBM, std::u16string, SAMARITAN_PUNCTUATION_ZIQAA)
    ->Range(8, 8 << 10);
BENCHMARK_TEMPLATE(GeodeHashBM, std::string, LINEAR_B_SYLLABLE_B008_A)
    ->Range(8, 8 << 10);
BENCHMARK_TEMPLATE(GeodeHashBM, std::u16string, LINEAR_B_SYLLABLE_B008_A)
    ->Range(8, 8 << 10);
