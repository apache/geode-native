#pragma once

#ifndef GEODE_CACHEABLEKEY_INL_
#define GEODE_CACHEABLEKEY_INL_

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

#include "CacheableKey.hpp"
#include "CacheableString.hpp"
#include "Cacheable.hpp"
#include "CacheableBuiltins.hpp"
/**
 * @file
 */

namespace apache {
namespace geode {
namespace client {

template <class TKEY>
inline std::shared_ptr<CacheableKey> createKey( const std::shared_ptr< TKEY >& value )
{
  return std::shared_ptr<CacheableKey>( value );
}

template< class PRIM >
inline std::shared_ptr<CacheableKey> CacheableKey::create( const PRIM value )
{
  return createKey( value );
}

template <typename TKEY>
inline std::shared_ptr<CacheableKey> createKey( const TKEY* value )
{
  return createKeyArr( value );
}

}  // namespace client
}  // namespace geode
}  // namespace apache

#endif // GEODE_CACHEABLEKEY_INL_
