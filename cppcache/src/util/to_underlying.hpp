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

#ifndef UTIL_UNDERLYING_TYPE_HPP
#define UTIL_UNDERLYING_TYPE_HPP

#include <type_traits>

/* Note: this implementation can be thrown away once we upgrade to C++14. */

template< class T >
using underlying_type_t = typename std::underlying_type<T>::type;

/* Note: this implementation can be thrown away once we upgrade to C++23. */

template< class Enum >
constexpr underlying_type_t<Enum> to_underlying( Enum e ) noexcept {
    return static_cast<underlying_type_t<Enum>>(e);
}

#endif
