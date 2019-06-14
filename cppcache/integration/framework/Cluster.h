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
          uint16_t jmxManagerPort, bool useIPv6)
      : cluster_(cluster),
        name_(std::move(name)),
        locators_(locators),
        jmxManagerPort_(jmxManagerPort) {
    auto hostname = "localhost";
    if (useIPv6) {
      hostname = "ip6-localhost";
    }

    auto port = Framework::getAvailablePort();

    locatorAddress_ = LocatorAddress{hostname, port};
  }

  ~Locator() noexcept {
    try {
      if (started_) {
        stop();
      }
    } catch (...) {
    }
  }

  Locator(const Locator &copy) = delete;
  Locator &operator=(const Locator &copy) = delete;
  Locator(Locator &&move)
      : cluster_(move.cluster_),
        name_(move.name_),
        locators_(move.locators_),
        locatorAddress_(move.locatorAddress_),
        jmxManagerPort_(move.jmxManagerPort_),
        started_(move.started_) {
    move.started_ = false;
  };

  const LocatorAddress &getAddress() const { return locatorAddress_; }

  void start();

  void stop();

 private:
  Cluster &cluster_;

  std::string name_;

  std::vector<Locator> &locators_;

  LocatorAddress locatorAddress_;

  uint16_t jmxManagerPort_;

  bool started_ = false;
};

struct ServerAddress {
  std::string address;
  uint16_t port;
};

class Server {
 public:
  Server(Cluster &cluster, std::vector<Locator> &locators, std::string name,
         std::string xmlFile, bool useIPv6)
      : cluster_(cluster),
        locators_(locators),
        name_(std::move(name)),
        xmlFile_(xmlFile) {

    auto hostname = "localhost";
    if (useIPv6) {
      hostname = "ip6-localhost";
    }

    auto port = static_cast<uint16_t>(0);
    serverAddress_ = ServerAddress{hostname, port};
  }

  std::string getCacheXMLFile() { return xmlFile_; }

  ~Server() noexcept {
    try {
      if (started_) {
        stop();
      }
    } catch (...) {
    }
  }

  Server(const Server &copy) = delete;
  Server &operator=(const Server &other) = delete;
  Server(Server &&move)
      : cluster_(move.cluster_),
        locators_(move.locators_),
        serverAddress_(move.serverAddress_),
        started_(move.started_),
        name_(move.name_),
        xmlFile_(move.xmlFile_) {
    move.started_ = false;
  };
  //  Server &operator=(Server &&other) = default;

  void start();

  void stop();

 private:
  Cluster &cluster_;
  std::vector<Locator> &locators_;

  ServerAddress serverAddress_;

  bool started_ = false;

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

class Cluster {
 public:
  Cluster(LocatorCount initialLocators, ServerCount initialServers,
          UseIpv6 useIPv6)
      : Cluster(Name(std::string(::testing::UnitTest::GetInstance()
                                     ->current_test_info()
                                     ->test_case_name()) +
                     "/" +
                     ::testing::UnitTest::GetInstance()
                         ->current_test_info()
                         ->name()),
                initialLocators, initialServers, useIPv6) {}

  Cluster(LocatorCount initialLocators, ServerCount initialServers)
      : Cluster(Name(std::string(::testing::UnitTest::GetInstance()
                                     ->current_test_info()
                                     ->test_case_name()) +
                     "/" +
                     ::testing::UnitTest::GetInstance()
                         ->current_test_info()
                         ->name()),
                initialLocators, initialServers) {}


  Cluster(LocatorCount initialLocators, ServerCount initialServers,
          CacheXMLFiles cacheXMLFiles)
      : name_(std::string(::testing::UnitTest::GetInstance()
                              ->current_test_info()
                              ->test_case_name()) +
              "/" +
              ::testing::UnitTest::GetInstance()->current_test_info()->name()),
        initialLocators_(initialLocators.get()),
        initialServers_(initialServers.get()),
        jmxManagerPort_(Framework::getAvailablePort()) {
    removeServerDirectory();
    cacheXMLFiles_ = cacheXMLFiles.get();
  }

  Cluster(Name name, LocatorCount initialLocators, ServerCount initialServers,
          UseIpv6 useIPv6)
      : Cluster(Name(name.get()), Classpath(""), SecurityManager(""), User(""),
                Password(""), initialLocators, initialServers,
                CacheXMLFiles({}), useIPv6) {}

  Cluster(Name name, LocatorCount initialLocators, ServerCount initialServers)
      : Cluster(Name(name.get()), Classpath(""), SecurityManager(""), User(""),
                Password(""), initialLocators, initialServers,
                CacheXMLFiles({}), UseIpv6(false)) {}

  Cluster(Name name, Classpath classpath, SecurityManager securityManager,
          User user, Password password, LocatorCount initialLocators,
          ServerCount initialServers, CacheXMLFiles cacheXMLFiles, UseIpv6 useIPv6)
      : name_(name.get()),
        classpath_(classpath.get()),
        securityManager_(securityManager.get()),
        user_(user.get()),
        password_(password.get()),
        initialLocators_(initialLocators.get()),
        initialServers_(initialServers.get()) {
    jmxManagerPort_ = Framework::getAvailablePort();
    cacheXMLFiles_ = cacheXMLFiles.get();
    useIPv6_ = useIPv6.get();

    removeServerDirectory();
    start();
  };

  Cluster(Name name, Classpath classpath, SecurityManager securityManager,
          User user, Password password, LocatorCount initialLocators,
          ServerCount initialServers)
      : name_(name.get()),
        classpath_(classpath.get()),
        securityManager_(securityManager.get()),
        user_(user.get()),
        password_(password.get()),
        initialLocators_(initialLocators.get()),
        initialServers_(initialServers.get()) {
    jmxManagerPort_ = Framework::getAvailablePort();

    removeServerDirectory();
    start();
  }

  ~Cluster() noexcept {
    try {
      if (started_) {
        stop();
      }
    } catch (...) {
    }
  }

  Cluster(const Cluster &copy) = delete;
  Cluster &operator=(const Cluster &other) = delete;
  Cluster(Cluster &&copy) = default;
  Cluster &operator=(Cluster &&other) = default;

  std::string getJmxManager();

  uint16_t getLocatorPort();

  void start(std::function<void()> fn = []() {});

  void stop();

  void removeServerDirectory() {
    boost::filesystem::path serverDir = boost::filesystem::relative(name_);
    boost::filesystem::remove_all(serverDir);
  }

  apache::geode::client::Cache createCache() { return createCache({}); }

  apache::geode::client::Cache createCache(
      const std::unordered_map<std::string, std::string> &properties) {
    return createCache(properties, false);
  }

  apache::geode::client::Cache createCache(
      const std::unordered_map<std::string, std::string> &properties,
      bool subscriptionEnabled) {
    using apache::geode::client::CacheFactory;

    CacheFactory cacheFactory;

    for (auto &&property : properties) {
      cacheFactory.set(property.first, property.second);
    }

    auto cache = cacheFactory.set("log-level", "none")
                     .set("statistic-sampling-enabled", "false")
                     .create();

    auto poolFactory =
        cache.getPoolManager().createFactory().setSubscriptionEnabled(
            subscriptionEnabled);
    applyLocators(poolFactory);
    poolFactory.create("default");

    return cache;
  }

  void applyLocators(apache::geode::client::PoolFactory &poolFactory) {
    for (const auto &locator : locators_) {
      poolFactory.addLocator(locator.getAddress().address,
                             locator.getAddress().port);
    }
  }

  void useSsl(const std::string keystore, const std::string truststore, const std::string keystorePassword, const std::string truststorePassword);

  bool useSsl();
  std::string keystore();
  std::string truststore();
  std::string keystorePassword();
  std::string truststorePassword();

  Gfsh &getGfsh() noexcept { return gfsh_; }

  std::vector<Server> &getServers() { return servers_; }

  std::vector<Locator> &getLocators() { return locators_; }

  std::string &getClasspath() { return classpath_; }

  std::string &getSecurityManager() { return securityManager_; }

  std::string &getUser() { return user_; }

  std::string &getPassword() { return password_; }

  std::vector<std::string> &getCacheXMLFiles() { return cacheXMLFiles_; }

  bool getUseIPv6() { return useIPv6_; }

 private:
  std::string name_;
  std::string classpath_;
  std::string securityManager_;
  std::string user_;
  std::string password_;
  std::vector<std::string> cacheXMLFiles_;

  size_t initialLocators_;
  std::vector<Locator> locators_;

  size_t initialServers_;
  std::vector<Server> servers_;

  bool started_ = false;
  uint16_t jmxManagerPort_;

  bool useSsl_ = false;
  std::string keystore_;
  std::string keystorePassword_;
  std::string truststore_;
  std::string truststorePassword_;

  bool useIPv6_ = false;

  GfshExecute gfsh_;

  void startLocators();
  void startServers();
};

#endif  // INTEGRATION_TEST_FRAMEWORK_CLUSTER_H
