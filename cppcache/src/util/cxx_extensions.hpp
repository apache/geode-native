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

#ifndef GEODE_UTIL_CXX_EXTENSIONS_H_
#define GEODE_UTIL_CXX_EXTENSIONS_H_

#include <memory>
#include <utility>

namespace cxx {
// Note that even std::make_unique is added since MSVC 12.0,
// is not officially supported until C++14, so this could be removed once
// C++14 is supported.
template <typename T, typename... Args>
::std::unique_ptr<T> make_unique(Args &&...args) {
  return ::std::unique_ptr<T>(new T(::std::forward<Args>(args)...));
}
} // namespace std

#endif // GEODE_UTIL_CXX_EXTENSIONS_H_
