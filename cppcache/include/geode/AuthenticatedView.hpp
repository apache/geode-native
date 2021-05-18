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

#ifndef GEODE_AUTHENTICATEDVIEW_H_
#define GEODE_AUTHENTICATEDVIEW_H_

#include <geode/PoolFactory.hpp>
#include <geode/QueryService.hpp>
#include <geode/Region.hpp>
#include <geode/RegionService.hpp>
#include <geode/internal/geode_globals.hpp>

/**
 * @file
 */

namespace apache {
namespace geode {
namespace client {

class GuardUserAttributes;
class UserAttributes;
class FunctionServiceImpl;

/**
 * Creates an authenticated cache view to allow credential based access to
 * region services.
 */

class APACHE_GEODE_EXPORT AuthenticatedView : public RegionService {
  /**
   * @brief public methods
   */
 public:
  /**
   * Indicates if this cache has been closed.
   * After a new cache object is created, this method returns false;
   * After the close is called on this cache object, this method
   * returns true.
   *
   * @return true, if this cache is closed; false, otherwise
   */
  bool isClosed() const override;

  /**
   * Terminates this object cache and releases all the local resources.
   * After this cache is closed, any further
   * method call on this cache or any region object will throw
   * <code>CacheClosedException</code>, unless otherwise noted.
   * @throws CacheClosedException,  if the cache is already closed.
   */
  void close() override;

  /** Look up a region with the full path from root.
   * @param path the region's path, such as <code>RootA/Sub1/Sub1A</code>.
   * @returns region, or nullptr if no such region exists.
   */
  std::shared_ptr<Region> getRegion(const std::string& path) const override;

  /**
   * Gets the QueryService from which a new Query can be obtained.
   *
   * @returns A smart pointer to the QueryService.
   */
  std::shared_ptr<QueryService> getQueryService() override;

  /**
   * Returns a set of root regions in the cache. This set is a snapshot and
   * is not backed by the Cache. The vector passed in is cleared and the
   * regions are added to it.
   */
  std::vector<std::shared_ptr<Region>> rootRegions() const override;

  ~AuthenticatedView() override;

  AuthenticatedView(std::shared_ptr<Properties> credentials,
                    std::shared_ptr<Pool> pool, CacheImpl* cacheImpl);
  AuthenticatedView(AuthenticatedView&& other) = default;
  AuthenticatedView(const AuthenticatedView& other) = delete;

  /**
   * Returns a factory that can create a {@link PdxInstance}.
   * @param className the fully qualified class name that the PdxInstance will
   * become when it is fully deserialized.
   * @param expectDomainClass Whether or not created PdxType represents a
   * Java domain class.
   * @return the factory
   */
  PdxInstanceFactory createPdxInstanceFactory(
      const std::string& className, bool expectDomainClass) const override;

  /**
   * Returns a factory that can create a {@link PdxInstance}.
   * @param className the fully qualified class name that the PdxInstance will
   * become when it is fully deserialized.
   * @return the factory
   */
  PdxInstanceFactory createPdxInstanceFactory(
      const std::string& className) const override;

  AuthenticatedView& operator=(AuthenticatedView&& other) = default;
  AuthenticatedView& operator=(const AuthenticatedView& other) = delete;

 private:
  std::shared_ptr<UserAttributes> m_userAttributes;
  bool m_isAuthenticatedViewClosed;
  std::shared_ptr<QueryService> m_remoteQueryService;
  CacheImpl* m_cacheImpl;

  friend class Pool;
  friend class ProxyRegion;
  friend class ProxyRemoteQueryService;
  friend class RemoteQuery;
  friend class ExecutionImpl;
  friend class FunctionServiceImpl;
  friend class FunctionService;
  friend class GuardUserAttributes;
  friend class CacheRegionHelper;
};
}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_AUTHENTICATEDVIEW_H_
