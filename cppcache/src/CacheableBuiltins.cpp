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

#include <geode/CacheableBuiltins.hpp>

namespace apache {
namespace geode {
namespace client {
namespace internal {

template class CacheableKeyPrimitive<bool, DSCode::CacheableBoolean>;
template class CacheableKeyPrimitive<int8_t, DSCode::CacheableByte>;
template class CacheableKeyPrimitive<double, DSCode::CacheableDouble>;
template class CacheableKeyPrimitive<float, DSCode::CacheableFloat>;
template class CacheableKeyPrimitive<int16_t, DSCode::CacheableInt16>;
template class CacheableKeyPrimitive<int32_t, DSCode::CacheableInt32>;
template class CacheableKeyPrimitive<int64_t, DSCode::CacheableInt64>;
template class CacheableKeyPrimitive<char16_t, DSCode::CacheableCharacter>;

template class CacheableArrayPrimitive<int8_t, DSCode::CacheableBytes>;
template class CacheableArrayPrimitive<bool, DSCode::BooleanArray>;
template class CacheableArrayPrimitive<char16_t, DSCode::CharArray>;
template class CacheableArrayPrimitive<double, DSCode::CacheableDoubleArray>;
template class CacheableArrayPrimitive<float, DSCode::CacheableFloatArray>;
template class CacheableArrayPrimitive<int16_t, DSCode::CacheableInt16Array>;
template class CacheableArrayPrimitive<int32_t, DSCode::CacheableInt32Array>;
template class CacheableArrayPrimitive<int64_t, DSCode::CacheableInt64Array>;
template class CacheableArrayPrimitive<std::shared_ptr<CacheableString>,
                                       DSCode::CacheableStringArray>;
}  // namespace internal
}  // namespace client
}  // namespace geode
}  // namespace apache
