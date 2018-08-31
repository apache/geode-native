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

#ifndef GEODE_POOLFACTORY_H_
#define GEODE_POOLFACTORY_H_

#include <chrono>

#include "internal/geode_globals.hpp"
#include "internal/chrono/duration.hpp"
#include "Pool.hpp"

/**
 * @file
 */

namespace apache {
namespace geode {
namespace client {

class CacheImpl;
class PoolAttributes;
class Pool;

/**
 * This interface provides for the configuration and creation of instances of
 * {@link Pool}.
 * <p>Every pool needs to have at least one {@link #addLocator locator} or
 * {@link #addServer server} added
 * to it. Locators should be added unless direct connections to
 * bridge servers are desired.
 * <p>The setter methods are used to specify
 * non-default values for the other pool properties.
 * <p>Once it is configured {@link #create}
 * will produce an instance.
 * <p>The factory can be restored to its default
 * configuration by calling {@link #reset}.
 * <p>Instances of this interface can be created by calling
 * {@link Cache#getPoolFactory}.
 * <p>
 * If a subscription is going to be made using a pool then subscriptions
 * {@link #setSubscriptionEnabled must be enabled} on the pool.
 * Subscriptions are made using these APIs:
 * <ul>
 * <li>{@link QueryService#newCq}
 * <li>{@link Region#registerKeys}
 * <li>{@link Region#registerAllKeys}
 * <li>{@link Region#registerRegex}
 * </ul>
 *
 */
class APACHE_GEODE_EXPORT PoolFactory {
 public:
  /**
   * The default amount of time which we will wait for a free connection if max
   * connections is set and all of the connections are in use.
   * <p>Current value: <code>10s</code>.
   */
  static const std::chrono::milliseconds DEFAULT_FREE_CONNECTION_TIMEOUT;

  /**
   * The default interval in which the pool will check to see if
   * a connection to a given server should be moved to a different
   * server to improve the load balance.
   * <p>Current value: <code>5min</code>
   */
  static const std::chrono::milliseconds DEFAULT_LOAD_CONDITIONING_INTERVAL;

  /**
   * The default size in bytes of the socket buffer on each connection
   * established.
   * <p>Current value: <code>32768</code>.
   */
  static const int DEFAULT_SOCKET_BUFFER_SIZE = 32768;

  /**
   * The default amount of time to wait for a response from a server.
   * <p>Current value: <code>10s</code>.
   */
  static const std::chrono::milliseconds DEFAULT_READ_TIMEOUT;

  /**
   * The default number of connections to be created initially.
   * <p>Current value: <code>1</code>.
   */
  static const int DEFAULT_MIN_CONNECTIONS = 1;

  /**
   * The default maximum number of connections to be created.
   * <p>Current value: <code>-1</code>.
   */
  static const int DEFAULT_MAX_CONNECTIONS = -1;

  /**
   * The default amount of time in to wait for a connection to become idle.
   * <p>Current value: <code>5s</code>.
   */
  static const std::chrono::milliseconds DEFAULT_IDLE_TIMEOUT;

  /**
   * The default number of times to retry an operation after a timeout or
   * exception.
   * <p>Current value: <code>-1</code>.
   */
  static const int DEFAULT_RETRY_ATTEMPTS = -1;

  /**
   * The default frequenc, to ping servers.
   * <p>Current value: <code>10s</code>.
   */
  static const std::chrono::milliseconds DEFAULT_PING_INTERVAL;

  /**
   * The default frequency to update the locator list.
   * <p>Current value: <code>5s</code>.
   */
  static const std::chrono::milliseconds DEFAULT_UPDATE_LOCATOR_LIST_INTERVAL;

  /**
   * The default frequency that client statistics are sent to the server.
   * <p>Current value: <code>std::chrono::milliseconds::zero()</code>
   * (disabled).
   */
  static const std::chrono::milliseconds DEFAULT_STATISTIC_INTERVAL;

  /**
   * The default value for whether to establish a server to client subscription.
   * <p>Current value: <code>false</code>.
   */
  static const bool DEFAULT_SUBSCRIPTION_ENABLED = false;

  /**
   * The default redundancy for servers holding subscriptions established by
   * this
   * client.
   * <p>Current value: <code>0</code>.
   */
  static const int DEFAULT_SUBSCRIPTION_REDUNDANCY = 0;

  /**
   * The default amount of time that messages sent from a  server to a client
   * will be tracked. The tracking is done to minimize duplicate events.
   * <p>Current value: <code>900s</code>.
   */
  static const std::chrono::milliseconds
      DEFAULT_SUBSCRIPTION_MESSAGE_TRACKING_TIMEOUT;

  /**
   * The default amount of time to wait before sending an acknowledgement to the
   * server about events received from the subscriptions.
   * <p>Current value: <code>100ms</code>.
   */
  static const std::chrono::milliseconds DEFAULT_SUBSCRIPTION_ACK_INTERVAL;

  /**
   * The default server group.
   * <p>Current value: <code>""</code>.
   */
  static const std::string DEFAULT_SERVER_GROUP;
  /**
   * Whether thread local connection is enabled.
   * <p>Current value: <code>"false"</code>.
   */
  static constexpr bool DEFAULT_THREAD_LOCAL_CONN = false;

  /**
   * Whether client is in multi user secure mode
   * <p>Current value: <code>"false"</code>.
   */
  static constexpr bool DEFAULT_MULTIUSER_SECURE_MODE = false;

  /**
   * The default value for whether to have single hop optimisations enabled.
   * <p>Current value: <code>true</code>.
   */
  static constexpr bool DEFAULT_PR_SINGLE_HOP_ENABLED = true;

  /**
   * Sets the free connection timeout for this pool.
   * If the pool has a max connections setting, operations will block
   * if all of the connections are in use. The free connection timeout
   * specifies how long those operations will block waiting for
   * a free connection before receiving
   * an {@link AllConnectionsInUseException}. If max connections
   * is not set this setting has no effect.
   *
   * @see #setMaxConnections(int)
   *
   * @param connectionTimeout is the connection timeout
   *
   * @return a reference to <code>this</code>
   * @throws IllegalArgumentException if <code>connectionTimeout</code>
   * is less than or equal to <code>std::chrono::milliseconds::zero()</code>.
   */
  PoolFactory& setFreeConnectionTimeout(
      std::chrono::milliseconds connectionTimeout);

  /**
   * Sets the load conditioning interval for this pool.
   * This interval controls how frequently the pool will check to see if
   * a connection to a given server should be moved to a different
   * server to improve the load balance.
   * <p>A value of <code>std::chrono::milliseconds::zero()</code> disables load
   * conditioning.
   *
   * @param loadConditioningInterval is the connection lifetime
   *
   * @return a reference to <code>this</code>
   * @throws IllegalArgumentException if <code>connectionLifetime</code>
   * is less than <code>std::chrono::milliseconds::zero()</code>.
   */
  PoolFactory& setLoadConditioningInterval(
      std::chrono::milliseconds loadConditioningInterval);

  /**
   * Sets the socket buffer size for each connection made in this pool.
   * Large messages can be received and sent faster when this buffer is larger.
   * Larger buffers also optimize the rate at which servers can send events
   * for client subscriptions.
   *
   * @param bufferSize is the size of the socket buffers used for reading and
   * writing on each connection in this pool.
   *
   * @return a reference to <code>this</code>
   * @throws IllegalArgumentException if <code>bufferSize</code>
   * is less than or equal to <code>0</code>.
   */
  PoolFactory& setSocketBufferSize(int bufferSize);

  /**
   * Sets the thread local connections policy for this pool.
   * If <code>true</code> then any time a thread goes to use a connection
   * from this pool it will check a thread local cache and see if it already
   * has a connection in it. If so it will use it. If not it will get one from
   * this pool and cache it in the thread local. This gets rid of thread
   * contention
   * for the connections but increases the number of connections the servers
   * see.
   * <p>If <code>false</code> then connections are returned to the pool as soon
   * as the operation being done with the connection completes. This allows
   * connections to be shared amonst multiple threads keeping the number of
   * connections down.
   *
   * @param threadLocalConnections if <code>true</code> then enable thread local
   * connections.
   * @return a reference to <code>this</code>
   */
  PoolFactory& setThreadLocalConnections(bool threadLocalConnections);

  /**
   * Sets the duration to wait for a response from a server before timing out
   * the operation and trying another server (if any are available).
   *
   * @param timeout duration to wait for a response from a
   * server
   * @return a reference to <code>this</code>
   * @throws IllegalArgumentException if <code>timeout</code>
   * is less than or equal to <code>std::chrono::milliseconds::zero()</code>.
   */
  PoolFactory& setReadTimeout(std::chrono::milliseconds timeout);

  /**
   * Sets the minimum number of connections to keep available at all times.
   * When the pool is created, it will create this many connections.
   * If <code>0</code> then connections will not be made until an actual
   * operation
   * is done that requires client-to-server communication.
   *
   * @param minConnections is the initial number of connections
   * this pool will create.
   * @return a reference to <code>this</code>
   * @throws IllegalArgumentException if <code>minConnections</code>
   * is less than <code>0</code>.
   */
  PoolFactory& setMinConnections(int minConnections);

  /**
   * Sets the max number of client to server connections that the pool will
   * create. If all of
   * the connections are in use, an operation requiring a client to server
   * connection
   * will block until a connection is available.
   *
   * @see #setFreeConnectionTimeout(int)
   *
   * @param maxConnections is the maximum number of connections in the pool.
   * <code>-1</code> indicates that there is no maximum number of connections
   *
   * @return a reference to <code>this</code>
   * @throws IllegalArgumentException if <code>maxConnections</code>
   * is less than <code>minConnections</code>.
   */
  PoolFactory& setMaxConnections(int maxConnections);

  /**
   * Sets the amount of time a connection can be idle before expiring the
   * connection. If the pool size is greater than the minimum specified by
   * {@link PoolFactory#setMinConnections(int)}, connections which have been
   * idle for longer than the idleTimeout will be closed.
   *
   * @param idleTimeout is the duration that an idle connection
   * should live no less than before expiring, actual time may be longer
   * depending on clock resolution. A duration std::chrono::milliseconds::zero()
   * indicates that connections should never expire.
   * @return a reference to <code>this</code>
   */
  PoolFactory& setIdleTimeout(std::chrono::milliseconds);

  /**
   * Set the number of times to retry a request after timeout/exception.
   * @param retryAttempts is the number of times to retry a request
   * after timeout/exception. -1 indicates that a request should be
   * tried against every available server before failing
   * @return a reference to <code>this</code>
   * @throws IllegalArgumentException if <code>idleTimout</code>
   * is less than <code>0</code>.
   */
  PoolFactory& setRetryAttempts(int retryAttempts);

  /**
   * The frequency with which servers must be pinged to verify that they are
   * still alive.
   * Each server will be sent a ping every <code>pingInterval</code> if there
   * has not
   * been any other communication with the server.
   *
   * These pings are used by the server to monitor the health of
   * the client. Make sure that the <code>pingInterval</code> is less than the
   * maximum time between pings allowed by the bridge server.
   *
   * @param pingInterval is the amount of time  between pings.
   * @return a reference to <code>this</code>
   * @throws IllegalArgumentException if <code>pingInterval</code>
   * is less than <code>0</code>.
   *
   * @see CacheServer#setMaximumTimeBetweenPings(int)
   */
  PoolFactory& setPingInterval(std::chrono::milliseconds pingInterval);

  /**
   * The frequency with which client updates the locator list. To disable this
   * set its value to std::chrono::milliseconds::zero().
   *
   * @param updateLocatorListInterval is the amount of time
   * between checking locator list at locator.
   * @return a reference to <code>this</code>
   */
  PoolFactory& setUpdateLocatorListInterval(
      std::chrono::milliseconds updateLocatorListInterval);

  /**
   * The frequency with which the client statistics must be sent to the server.
   * Doing this allows <code>GFMon</code> to monitor clients.
   * <p>A value of <code>std::chrono::milliseconds::zero()</code> disables the
   * sending of client statistics to the server.
   *
   * @param statisticInterval is the amount of time between
   * sends of client statistics to the server.
   *
   * @return a reference to <code>this</code>
   * @throws IllegalArgumentException if <code>statisticInterval</code>
   * is less than <code>std::chrono::milliseconds::zero()</code>.
   */
  PoolFactory& setStatisticInterval(
      std::chrono::milliseconds statisticInterval);

  /**
   * Configures the group which contains all the servers that this pool connects
   * to.
   * @param group is the server group that this pool will connect to.
   * If the value is <code>null</code> or <code>""</code> then the pool connects
   * to all servers.
   * @return a reference to <code>this</code>
   */
  PoolFactory& setServerGroup(std::string group);

  /**
   * Adds a locator, given its host and port, to this factory.
   * The locator must be a server locator and will be used to discover other
   * running
   * bridge servers and locators.
   * @param host is the host name or ip address that the locator is listening
   * on.
   * @param port is the port that the locator is listening on.
   * @return a reference to <code>this</code>
   * @throws IllegalArgumentException if the <code>host</code> is an unknown
   * host
   * according to {@link java.net.InetAddress#getByName} or if the port is
   * outside
   * the valid range of [1..65535] inclusive.
   * @throws IllegalStateException if the locator has already been {@link
   * #addServer added} to this factory.
   */
  PoolFactory& addLocator(const std::string& host, int port);

  /**
   * Adds a server, given its host and port, to this factory.
   * The server must be a bridge server and this client will
   * directly connect to the server without consulting a server locator.
   * @param host is the host name or ip address that the server is listening on.
   * @param port is the port that the server is listening on.
   * @return a reference to <code>this</code>
   * @throws IllegalArgumentException if the <code>host</code> is an unknown
   * host
   * according to {@link java.net.InetAddress#getByName} or if the port is
   * outside
   * the valid range of [1..65535] inclusive.
   * @throws IllegalStateException if the server has already been {@link
   * #addLocator added} to this factory.
   */
  PoolFactory& addServer(const std::string& host, int port);

  /**
   * If set to <code>true</code> then the created pool will have
   * server-to-client
   * subscriptions enabled.
   * If set to <code>false</code> then all <code>Subscription*</code> attributes
   * are ignored at the time of creation.
   * @return a reference to <code>this</code>
   */
  PoolFactory& setSubscriptionEnabled(bool enabled);

  /**
   * Sets the redundancy level for this pools server-to-client subscriptions.
   * If <code>0</code> then no redundant copies are kept on the servers.
   * Otherwise an effort is made to maintain the requested number of
   * copies of the server-to-client subscriptions. At most, one copy per server
   * is
   *  made up to the requested level.
   * @param redundancy is the number of redundant servers for this client's
   * subscriptions.
   * @return a reference to <code>this</code>
   * @throws IllegalArgumentException if <code>redundancyLevel</code>
   * is less than <code>-1</code>.
   */
  PoolFactory& setSubscriptionRedundancy(int redundancy);

  /**
   * Sets the messageTrackingTimeout attribute which is the time-to-live period
   * for subscription events the client has received from the server. It is used
   * to minimize duplicate events. Entries that have not been modified for this
   * amount of time  are expired from the list.
   *
   * @param messageTrackingTimeout is the duration to set the timeout to.
   * @return a reference to <code>this</code>
   *
   * @throws IllegalArgumentException if <code>messageTrackingTimeout</code>
   * is less than or equal to <code>0</code>.

   */
  PoolFactory& setSubscriptionMessageTrackingTimeout(
      std::chrono::milliseconds messageTrackingTimeout);

  /**
   * Sets the is the interval to wait before sending acknowledgements to the
   * bridge server for events received from the server subscriptions.
   *
   * @param ackInterval is the duration to wait before sending  event
   * acknowledgements.
   *
   * @throws IllegalArgumentException if <code>ackInterval</code>
   * is less than or equal to <code>0</code>.
   * @return a reference to <code>this</code>
   */
  PoolFactory& setSubscriptionAckInterval(
      std::chrono::milliseconds ackInterval);

  /**
   * Sets whether Pool is in multi user secure mode.
   * If its in multiuser mode then app needs to get RegionService instance of
   * Cache.
   * Deafult value is false.
   * @return a reference to <code>this</code>
   */
  PoolFactory& setMultiuserAuthentication(bool multiuserAuthentication);

  /**
   * Resets the configuration of this factory to its defaults.
   * @return a reference to <code>this</code>
   */
  PoolFactory& reset();

  /**
   * Creates a new Pool for connecting a client to a set of Geode Cache
   * Servers.
   * using this factory's settings for attributes.
   *
   * @param name is the name of the pool, used when connecting regions to it
   * @throws IllegalStateException if a pool with <code>name</code> already
   * exists
   * @throws IllegalStateException if a locator or server has not been added.
   * @return the newly created pool.
   */
  std::shared_ptr<Pool> create(std::string name);

  /**
   * By default setPRSingleHopEnabled is true<br>
   * The client is aware of location of partitions on servers hosting
   * {@link Region}s.
   * Using this information, the client routes the client cache operations
   * directly to the server which is hosting the required partition for the
   * cache operation.
   * If setPRSingleHopEnabled is false the client can do an extra hop on servers
   * to go to the required partition for that cache operation.
   * The setPRSingleHopEnabled avoids extra hops only for following cache
   * operations:<br>
   * 1. {@link Region#put(Object, Object)}<br>
   * 2. {@link Region#get(Object)}<br>
   * 3. {@link Region#destroy(Object)}<br>
   * If true, works best when {@link PoolFactory#setMaxConnections(int)} is set
   * to -1.
   * @param name is boolean whether PR Single Hop optimization is enabled or
   * not.
   * @return a reference to <code>this</code>
   */
  PoolFactory& setPRSingleHopEnabled(bool enabled);

  ~PoolFactory() = default;

  PoolFactory(const PoolFactory&) = default;

 private:
  PoolFactory(const Cache& cache);
  PoolFactory& addCheck(const std::string& host, int port);
  std::shared_ptr<PoolAttributes> m_attrs;
  bool m_isSubscriptionRedundancy;
  bool m_addedServerOrLocator;
  const Cache& m_cache;

  friend class Cache;
  friend class PoolManager;
  friend class PoolManagerImpl;
  friend class CacheFactory;
  friend class CacheXmlCreation;
};

}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_POOLFACTORY_H_
