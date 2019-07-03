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

#ifndef GEODE_INTERNALCACHETRANSACTIONMANAGER2PCIMPL_H_
#define GEODE_INTERNALCACHETRANSACTIONMANAGER2PCIMPL_H_

#include "CacheTransactionManagerImpl.hpp"

namespace apache {
namespace geode {
namespace client {

class InternalCacheTransactionManager2PCImpl
    : public CacheTransactionManagerImpl {
 public:
  explicit InternalCacheTransactionManager2PCImpl(CacheImpl* cache);
  virtual ~InternalCacheTransactionManager2PCImpl() override;

  virtual void prepare() override;
  virtual void commit() override;
  virtual void rollback() override;

 private:
  void afterCompletion(int32_t status);

  InternalCacheTransactionManager2PCImpl& operator=(
      const InternalCacheTransactionManager2PCImpl& other);
  InternalCacheTransactionManager2PCImpl(
      const InternalCacheTransactionManager2PCImpl& other);
};

}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_INTERNALCACHETRANSACTIONMANAGER2PCIMPL_H_
