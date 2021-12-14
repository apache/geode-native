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

template class APACHE_GEODE_EXPLICIT_TEMPLATE_EXPORT
    CacheableKeyPrimitive<bool, DSCode::CacheableBoolean>;
template class APACHE_GEODE_EXPLICIT_TEMPLATE_EXPORT
    CacheableKeyPrimitive<int8_t, DSCode::CacheableByte>;
template class APACHE_GEODE_EXPLICIT_TEMPLATE_EXPORT
    CacheableKeyPrimitive<double, DSCode::CacheableDouble>;
template class APACHE_GEODE_EXPLICIT_TEMPLATE_EXPORT
    CacheableKeyPrimitive<float, DSCode::CacheableFloat>;
template class APACHE_GEODE_EXPLICIT_TEMPLATE_EXPORT
    CacheableKeyPrimitive<int16_t, DSCode::CacheableInt16>;
template class APACHE_GEODE_EXPLICIT_TEMPLATE_EXPORT
    CacheableKeyPrimitive<int32_t, DSCode::CacheableInt32>;
template class APACHE_GEODE_EXPLICIT_TEMPLATE_EXPORT
    CacheableKeyPrimitive<int64_t, DSCode::CacheableInt64>;
template class APACHE_GEODE_EXPLICIT_TEMPLATE_EXPORT
    CacheableKeyPrimitive<char16_t, DSCode::CacheableCharacter>;

template class APACHE_GEODE_EXPLICIT_TEMPLATE_EXPORT
    CacheableArrayPrimitive<int8_t, DSCode::CacheableBytes>;
template class APACHE_GEODE_EXPLICIT_TEMPLATE_EXPORT
    CacheableArrayPrimitive<bool, DSCode::BooleanArray>;
template class APACHE_GEODE_EXPLICIT_TEMPLATE_EXPORT
    CacheableArrayPrimitive<char16_t, DSCode::CharArray>;
template class APACHE_GEODE_EXPLICIT_TEMPLATE_EXPORT
    CacheableArrayPrimitive<double, DSCode::CacheableDoubleArray>;
template class APACHE_GEODE_EXPLICIT_TEMPLATE_EXPORT
    CacheableArrayPrimitive<float, DSCode::CacheableFloatArray>;
template class APACHE_GEODE_EXPLICIT_TEMPLATE_EXPORT
    CacheableArrayPrimitive<int16_t, DSCode::CacheableInt16Array>;
template class APACHE_GEODE_EXPLICIT_TEMPLATE_EXPORT
    CacheableArrayPrimitive<int32_t, DSCode::CacheableInt32Array>;
template class APACHE_GEODE_EXPLICIT_TEMPLATE_EXPORT
    CacheableArrayPrimitive<int64_t, DSCode::CacheableInt64Array>;
template class APACHE_GEODE_EXPLICIT_TEMPLATE_EXPORT CacheableArrayPrimitive<
    std::shared_ptr<CacheableString>, DSCode::CacheableStringArray>;

template class APACHE_GEODE_EXPLICIT_TEMPLATE_EXPORT
    CacheableContainerPrimitive<std::vector<std::shared_ptr<Cacheable>>,
                                DSCode::CacheableVector>;

template class APACHE_GEODE_EXPLICIT_TEMPLATE_EXPORT
    CacheableContainerPrimitive<HashMapOfCacheable, DSCode::CacheableHashMap>;

template class APACHE_GEODE_EXPLICIT_TEMPLATE_EXPORT
    CacheableContainerPrimitive<HashSetOfCacheableKey,
                                DSCode::CacheableHashSet>;

template class APACHE_GEODE_EXPLICIT_TEMPLATE_EXPORT
    CacheableContainerPrimitive<std::vector<std::shared_ptr<Cacheable>>,
                                DSCode::CacheableArrayList>;

template class APACHE_GEODE_EXPLICIT_TEMPLATE_EXPORT
    CacheableContainerPrimitive<std::vector<std::shared_ptr<Cacheable>>,
                                DSCode::CacheableLinkedList>;

template class APACHE_GEODE_EXPLICIT_TEMPLATE_EXPORT
    CacheableContainerPrimitive<HashMapOfCacheable, DSCode::CacheableStack>;

template class APACHE_GEODE_EXPLICIT_TEMPLATE_EXPORT
    CacheableContainerPrimitive<HashMapOfCacheable, DSCode::CacheableHashTable>;

template class APACHE_GEODE_EXPLICIT_TEMPLATE_EXPORT
    CacheableContainerPrimitive<HashMapOfCacheable,
                                DSCode::CacheableIdentityHashMap>;

template class APACHE_GEODE_EXPLICIT_TEMPLATE_EXPORT
    CacheableContainerPrimitive<HashSetOfCacheableKey,
                                DSCode::CacheableLinkedHashSet>;

}  // namespace internal
}  // namespace client
}  // namespace geode
}  // namespace apache
