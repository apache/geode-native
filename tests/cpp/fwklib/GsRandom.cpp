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

#include "GsRandom.hpp"

#include <cstring>

namespace apache {
namespace geode {
namespace client {
namespace testframework {

GsRandom &GsRandom::getInstance() {
  static GsRandom instance;
  return instance;
}

/**
 * @param max the maximum length of the random string to generate.
 * @param min the minimum length of the random string to generate, default is 0.
 * @return a bounded random string with a length between min and
 * max length inclusive.
 */
char *GsRandom::randomString(int32_t max, int32_t min) {
  int32_t len = (max == min) ? max : nextInt(min, max);
  char *buf = reinterpret_cast<char *>(malloc(len + 1));
  for (int32_t i = 0; i < len; i++) buf[i] = nextByte();
  buf[len] = 0;
  return buf;
}

/**
 * Like randomString(), but returns only readable characters.
 *
 * @param max the maximum length of the random string to generate.
 * @param min the minimum length of the random string to generate, default is 0.
 * @return a bounded random string with a length between min and
 * max length inclusive.
 */
char *GsRandom::randomReadableString(int32_t max, int32_t min) {
  int32_t len = (max == min) ? max : nextInt(min, max);
  char *buf = reinterpret_cast<char *>(malloc(len + 1));
  for (int32_t i = 0; i < len; i++) buf[i] = nextByte(32, 126);
  buf[len] = 0;
  return buf;
}

const char choseFrom[] =
    "0123456789 abcdefghijklmnopqrstuvwxyz_ABCDEFGHIJKLMNOPQRSTUVWXYZ";
const int32_t choseSize = static_cast<int32_t>(strlen(choseFrom)) - 1;

/**
 * Like randomString(), but returns only only alphanumeric, underscore, or space
 * characters.
 *
 * @param max the maximum length of the random string to generate.
 * @param min the minimum length of the random string to generate, default is 0.
 * @return a bounded random string with a length between min and
 * max length inclusive.
 */
char *GsRandom::randomAlphanumericString(int32_t max, int32_t min) {
  int32_t len = (max == min) ? max : nextInt(min, max);
  char *buf = reinterpret_cast<char *>(malloc(len + 1));
  for (int32_t i = 0; i < len; i++) buf[i] = choseFrom[nextByte(0, choseSize)];
  buf[len] = 0;
  return buf;
}
}  // namespace testframework
}  // namespace client
}  // namespace geode
}  // namespace apache
