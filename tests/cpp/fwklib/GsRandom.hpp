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

#ifndef GEODE_FWKLIB_GSRANDOM_H_
#define GEODE_FWKLIB_GSRANDOM_H_

#include <random>
#include <string>

#include <geode/internal/geode_base.hpp>

namespace apache {
namespace geode {
namespace client {
namespace testframework {

class GsRandom {
 private:
  std::default_random_engine engine;

  std::uniform_int_distribution<int16_t> distBoolean;
  std::uniform_int_distribution<int16_t> distUint8;
  std::uniform_int_distribution<uint16_t> distUint16;
  std::uniform_int_distribution<int32_t> distInt32;
  std::uniform_int_distribution<uint32_t> distUint32;
  std::uniform_real_distribution<double> distDouble;

  GsRandom()
      : distBoolean(std::numeric_limits<bool>::min(),
                    std::numeric_limits<bool>::max()),
        distUint8(std::numeric_limits<uint8_t>::min(),
                  std::numeric_limits<uint8_t>::max()),
        distUint16(),
        distInt32(),
        distUint32(),
        distDouble() {
    std::random_device seed;
    engine = std::default_random_engine(seed());
  }
  ~GsRandom() = default;
  GsRandom(const GsRandom&) = delete;
  GsRandom& operator=(const GsRandom&) = delete;

 public:
  /**
   * Creates a new random number generator. Its seed is initialized to
   * a value based on the random device.
   *
   */
  static GsRandom& getInstance();

  /**
   * @return the next pseudorandom, uniformly distributed <code>boolean</code>
   *         value from this random number generator's sequence.
   */
  inline bool nextBoolean() { return 1 == distBoolean(engine); }

  /**
   * @return the next pseudorandom, uniformly distributed <code>uint16_t</code>
   *         value from this random number generator's sequence.
   */
  inline uint16_t nextInt16() { return distUint16(engine); }

  /**
   * @return the next pseudorandom, uniformly distributed <code>byte</code>
   *         value from this random number generator's sequence.
   */
  inline uint8_t nextByte() { return static_cast<uint8_t>(distUint8(engine)); }

  /**
   * @param   min the minimum range (inclusive) for the pseudorandom.
   * @param   max the maximum range (inclusive) for the pseudorandom.
   * @return  the next pseudorandom, uniformly distributed <code>char</code>
   *          value from this random number generator's sequence.
   *       If max < min, returns 0 .
   */
  inline uint8_t nextByte(uint8_t min, uint8_t max) {
    return static_cast<uint8_t>(
        distUint8(engine, decltype(distUint8)::param_type(min, max)));
  }

  /**
   * @param   max the maximum range (inclusive) for the pseudorandom.
   * @return  the next pseudorandom, uniformly distributed <code>double</code>
   *          value from this random number generator's sequence.
   */
  inline double nextDouble(double max) { return nextDouble(0.0, max); }

  /**
   * @param   min the minimum range (inclusive) for the pseudorandom.
   * @param   max the maximum range (inclusive) for the pseudorandom.
   * @return  the next pseudorandom, uniformly distributed <code>double</code>
   *      value from this random number generator's sequence within a range
   *      from min to max.
   */
  inline double nextDouble(double min, double max) {
    return distDouble(engine, decltype(distDouble)::param_type(min, max));
  }

  /**
   * @param   max the maximum range (inclusive) for the pseudorandom.
   * @return  the next pseudorandom, uniformly distributed <code>int32_t</code>
   *          value from this random number generator's sequence.
   */
  inline int32_t nextInt(int32_t max) { return nextInt(0, max); }

  /**
   * @param   min the minimum range (inclusive) for the pseudorandom.
   * @param   max the maximum range (inclusive) for the pseudorandom.
   * @return  the next pseudorandom, uniformly distributed <code>int32_t</code>
   *          value from this random number generator's sequence.
   *       If max < min, returns 0 .
   */
  inline int32_t nextInt(int32_t min, int32_t max) {
    return distInt32(engine, decltype(distInt32)::param_type(min, max));
  }

  /** @brief return random number where: min <= retValue < max */
  static uint32_t random(uint32_t min, uint32_t max) {
    return getInstance().nextInt(min, max - 1);
  }

  /** @brief return random number where: 0 <= retValue < max */
  static uint32_t random(uint32_t max) { return random(0, max - 1); }

  /** @brief return random double where: min <= retValue <= max */
  static double random(double min, double max) {
    return getInstance().nextDouble(min, max);
  }

  /** @brief return bounded random string,
   * Like randomString(), but returns only only alphanumeric,
   *   underscore, or space characters.
   *
   * @param size the length of the random string to generate.
   * @retval a bounded random string
   */
  static std::string getAlphanumericString(uint32_t size) {
    std::string str(size + 1, '\0');
    static const char chooseFrom[] =
        "0123456789 abcdefghijklmnopqrstuvwxyz_ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    static const int32_t chooseSize = sizeof(chooseFrom) - 1;

    for (uint32_t idx = 0; idx < size; idx++) {
      str[idx] = chooseFrom[random(chooseSize)];
    }

    return str;
  }

  /** @brief return bounded random string,
   * Like randomString(), but returns only only alphanumeric,
   *   underscore, or space characters.
   *
   * @param size the length of the random string to generate.
   * @retval a bounded random string
   */
  static void getAlphanumericString(uint32_t size, char* buffer) {
    static const char chooseFrom[] =
        "0123456789 abcdefghijklmnopqrstuvwxyz_ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    static const int32_t chooseSize = sizeof(chooseFrom) - 1;

    for (uint32_t idx = 0; idx < size; idx++) {
      buffer[idx] = chooseFrom[random(chooseSize)];
    }
  }

  /**
   * @param max the maximum length of the random string to generate.
   * @return a bounded random string with a length between 0 and
   * max length inclusive.
   */
  char* randomString(int32_t max, int32_t min = 0);

  /**
   * Like randomString(), but returns only readable characters.
   *
   * @param max the maximum length of the random string to generate.
   * @return a bounded random string with a length between 0 and
   * max length inclusive.
   */
  char* randomReadableString(int32_t max, int32_t min = 0);

  /**
   * Like randomString(), but returns only alphanumeric, underscore, or space
   * characters.
   *
   * @param max the maximum length of the random string to generate.
   * @return a bounded random string with a length between 0 and
   * max length inclusive.
   */
  char* randomAlphanumericString(int32_t max, int32_t min);
};

}  // namespace testframework
}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_FWKLIB_GSRANDOM_H_
