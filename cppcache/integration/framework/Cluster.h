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

#ifndef INTEGRATION_TEST_FRAMEWORK_CLUSTER_H
#define INTEGRATION_TEST_FRAMEWORK_CLUSTER_H

#include <cstdint>
#include <string>

#include <geode/Cache.hpp>
#include <geode/PoolManager.hpp>

#include "Framework.h"
#include "GfshExecute.h"
#include "NamedType.h"
#include "gtest/gtest.h"

class Cluster;

struct LocatorAddress {
  std::string address;
  uint16_t port;
};

class Locator {
 public:
  Locator(Cluster &cluster, std::vector<Locator> &locators, std::string name,
          uint16_t jmxManagerPort);

  Locator(Cluster &cluster, std::vector<Locator> &locators, std::string name,
          uint16_t jmxManagerPort, LocatorAddress locatorAddress,
          std::vector<LocatorAddress> remoteLocators,
          int16_t distributedSystemId);

  ~Locator();

  Locator(const Locator &copy) = delete;
  Locator &operator=(const Locator &copy) = delete;
  Locator(Locator &&move);

  const LocatorAddress &getAddress() const;

  void start();

  void stop();

 private:
  Cluster &cluster_;

  std::string name_;

  std::vector<Locator> &locators_;

  uint16_t jmxManagerPort_;

  LocatorAddress locatorAddress_;

  std::vector<LocatorAddress> remoteLocators_;

  bool started_ = false;

  int16_t distributedSystemId_ = 0;
};

struct ServerAddress {
  std::string address;
  uint16_t port;
};

class Server {
 public:
  Server(Cluster &cluster, std::vector<Locator> &locators, std::string name,
         std::string xmlFile, ServerAddress serverAddress);

  std::string getCacheXMLFile();

  ~Server();

  Server(const Server &copy) = delete;
  Server &operator=(const Server &other) = delete;
  Server(Server &&move);

  void start();

  void stop();

  void wait();

  const ServerAddress &getAddress() const;

  boost::process::pid_t getPid() const {
    return pid_;
  }

 protected:
  std::string getPidFilePath() const;
  boost::process::pid_t getPidFromFile() const;

 private:
  Cluster &cluster_;
  std::vector<Locator> &locators_;

  ServerAddress serverAddress_;

  bool started_{false};
  boost::process::pid_t pid_;

  std::string name_;
  std::string xmlFile_;
};

using LocatorCount = NamedType<size_t, struct LocatorCountParameter>;
using ServerCount = NamedType<size_t, struct ServerCountParameter>;
using Name = NamedType<std::string, struct NameParameter>;
using Classpath = NamedType<std::string, struct ClasspathParameter>;
using SecurityManager = NamedType<std::string, struct SecurityManagerParameter>;
using User = NamedType<std::string, struct UserParameter>;
using Password = NamedType<std::string, struct PasswordParameter>;
using CacheXMLFiles =
    NamedType<std::vector<std::string>, struct CacheXMLFilesParameter>;
using UseIpv6 = NamedType<bool, struct UseIpv6Parameter>;
using ConserveSockets = NamedType<bool, struct ConserveSocketsParameter>;
using InitialLocators =
    NamedType<std::vector<LocatorAddress>, struct InitialLocatorsParameter>;
using RemoteLocators =
    NamedType<std::vector<LocatorAddress>, struct RemoteLocatorsParameter>;
using DistributedSystemId =
    NamedType<int16_t, struct DistributedSystemIdParameter>;
using InitialServers =
    NamedType<std::vector<ServerAddress>, struct InitialServerParameter>;

class Cluster {
 public:
  enum class SubscriptionState { Enabled, Disabled };

  Cluster(InitialLocators initialLocators, InitialServers initialServers,
          UseIpv6 useIpv6 = UseIpv6{false});

  Cluster(InitialLocators initialLocators, InitialServers initialServers,
          RemoteLocators remoteLocators,
          DistributedSystemId distributedSystemId);

  Cluster(Name name, Classpath classpath, SecurityManager securityManager,
          User user, Password password, InitialLocators initialLocators,
          InitialServers initialServers, CacheXMLFiles cacheXMLFiles,
          RemoteLocators remoteLocators,
          DistributedSystemId distributedSystemId);

  Cluster(LocatorCount initialLocators, ServerCount initialServers);

  Cluster(LocatorCount initialLocators, ServerCount initialServers,
          CacheXMLFiles cacheXMLFiles);

  Cluster(LocatorCount initialLocators, ServerCount initialServers,
          ConserveSockets conserveSockets, CacheXMLFiles cacheXMLFiles);

  Cluster(Name name, LocatorCount initialLocators, ServerCount initialServers);

  Cluster(Name name, Classpath classpath, SecurityManager securityManager,
          User user, Password password, LocatorCount initialLocators,
          ServerCount initialServers, CacheXMLFiles cacheXMLFiles);

  Cluster(Name name, Classpath classpath, SecurityManager securityManager,
          User user, Password password, LocatorCount initialLocators,
          ServerCount initialServers);

  ~Cluster();

  Cluster(const Cluster &copy) = delete;
  Cluster &operator=(const Cluster &other) = delete;
  Cluster(Cluster &&copy) = default;
  Cluster &operator=(Cluster &&other) = default;

  std::string getJmxManager();

  uint16_t getLocatorPort();

  void start();
  void start(std::function<void()> fn);

  void stop();

  void removeServerDirectory();

  apache::geode::client::Cache createCache();

  apache::geode::client::Cache createCache(
      const std::unordered_map<std::string, std::string> &properties);

  apache::geode::client::Cache createCache(
      const std::unordered_map<std::string, std::string> &properties,
      SubscriptionState);

  void applyLocators(apache::geode::client::PoolFactory &poolFactory);

  void applyServer(apache::geode::client::PoolFactory &poolFactory,
                   ServerAddress server);

  void useSsl(const bool requireSslAuthentication, const std::string keystore,
              const std::string truststore, const std::string keystorePassword,
              const std::string truststorePassword);

  bool useSsl();

  void usePropertiesFile(const std::string propertiesFile);
  void useSecurityPropertiesFile(const std::string securityPropertiesFile);
  void useHostNameForClients(const std::string hostNameForClients);
  bool usePropertiesFile();
  bool useSecurityPropertiesFile();
  bool useHostNameForClients();
  bool requireSslAuthentication();

  std::string keystore();
  std::string truststore();
  std::string keystorePassword();
  std::string truststorePassword();

  Gfsh &getGfsh();

  std::vector<Server> &getServers();

  std::vector<Locator> &getLocators();

  std::string &getClasspath();

  std::string &getSecurityManager();

  std::string &getUser();

  std::string &getPassword();

  std::vector<std::string> &getCacheXMLFiles();

  bool getUseIPv6();

  bool getConserveSockets();

 private:
  std::string name_;
  std::string classpath_;
  std::string securityManager_;
  std::string user_;
  std::string password_;
  std::vector<std::string> cacheXMLFiles_;

  std::vector<LocatorAddress> initialLocators_;
  std::vector<Locator> locators_;
  std::vector<LocatorAddress> remoteLocators_;

  std::vector<ServerAddress> initialServers_;
  std::vector<Server> servers_;

  bool started_ = false;
  uint16_t jmxManagerPort_;

  bool useSsl_ = false;
  bool requireSslAuthentication_ = false;
  bool usePropertiesFile_ = false;
  bool useSecurityPropertiesFile_ = false;
  bool useHostNameForClients_ = false;

  std::string keystore_;
  std::string keystorePassword_;
  std::string truststore_;
  std::string truststorePassword_;

  std::string propertiesFile_;
  std::string securityPropertiesFile_;
  std::string hostName_;

  bool useIPv6_ = false;
  bool conserveSockets_ = false;

  int16_t distributedSystemId_ = 0;

  GfshExecute gfsh_;

  void startLocators();
  void startServers();
};

#endif  // INTEGRATION_TEST_FRAMEWORK_CLUSTER_H
