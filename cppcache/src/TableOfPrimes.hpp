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

#ifndef GEODE_TABLEOFPRIMES_H_
#define GEODE_TABLEOFPRIMES_H_

#include <algorithm>

#include <geode/ExceptionTypes.hpp>
#include <geode/internal/geode_globals.hpp>

namespace apache {
namespace geode {
namespace client {

extern const uint32_t g_primeTable[];
extern const uint32_t g_primeLen;

extern const uint8_t g_primeConcurTable[];
extern const uint8_t g_primeConcurLen;

/** @brief find a prime number greater than a given integer.
 *  A sampling of primes are used from 0 to 1 million. Not every prime is
 *  necessary, as the map scales, little steps are usually uninteresting.
 */
class APACHE_GEODE_EXPORT TableOfPrimes {
 public:
  static uint32_t getPrimeLength();

  static uint32_t getPrime(uint32_t index);

  static uint32_t nextLargerPrime(uint32_t val, uint32_t& resIndex);

  static uint8_t getMaxPrimeForConcurrency();

  static uint8_t nextLargerPrimeForConcurrency(uint8_t val);
};

}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_TABLEOFPRIMES_H_
