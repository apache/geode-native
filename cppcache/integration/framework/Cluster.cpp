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

#include "Cluster.h"

#include <signal.h>

#include <future>

#include <boost/filesystem.hpp>

Locator::Locator(Cluster &cluster, std::vector<Locator> &locators,
                 std::string name, uint16_t jmxManagerPort, bool useIPv6)
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

Locator::~Locator() {
  try {
    if (started_) {
      stop();
    }
  } catch (...) {
  }
}

Locator::Locator(Locator &&move)
    : cluster_(move.cluster_),
      name_(move.name_),
      locators_(move.locators_),
      locatorAddress_(move.locatorAddress_),
      jmxManagerPort_(move.jmxManagerPort_),
      started_(move.started_) {
  move.started_ = false;
}

const LocatorAddress &Locator::getAddress() const { return locatorAddress_; }

void Locator::start() {
  if (started_) return;

  auto safeName = name_;
  std::replace(safeName.begin(), safeName.end(), '/', '_');

  if (boost::filesystem::is_regular_file(name_ + "/vf.gf.locator.pid")) {
    cluster_.getGfsh().stop().locator().withDir(name_).execute();
  }

  auto locator = cluster_.getGfsh()
                     .start()
                     .locator()
                     .withLogLevel("INFO")
                     .withDir(name_)
                     .withName(safeName)
                     .withBindAddress(locatorAddress_.address)
                     .withPort(locatorAddress_.port)
                     .withMaxHeap("256m")
                     .withJmxManagerPort(jmxManagerPort_)
                     .withHttpServicePort(0)
                     .withClasspath(cluster_.getClasspath())
                     .withSecurityManager(cluster_.getSecurityManager())
                     .withPreferIPv6(cluster_.getUseIPv6())
                     .withJmxManagerStart(true);

  if (cluster_.useSsl()) {
    locator.withConnect(false)
        .withSslEnabledComponents("all")
        .withSslRquireAuthentication(cluster_.requireSslAuthentication())
        .withSslKeystore(cluster_.keystore())
        .withSslTruststore(cluster_.truststore())
        .withSslKeystorePassword(cluster_.keystorePassword())
        .withSslTruststorePassword(cluster_.truststorePassword());
  }

  locator.execute(cluster_.getUser(), cluster_.getPassword(),
                  cluster_.keystore(), cluster_.truststore(),
                  cluster_.keystorePassword(), cluster_.truststorePassword());

  auto connect =
      cluster_.getGfsh().connect().withJmxManager(cluster_.getJmxManager());

  if (!cluster_.getUser().empty()) {
    connect.withUser(cluster_.getUser()).withPassword(cluster_.getPassword());
  }

  if (cluster_.useSsl()) {
    connect.withUseSsl(true)
        .withKeystore(cluster_.keystore())
        .withTruststore(cluster_.truststore())
        .withKeystorePassword(cluster_.keystorePassword())
        .withTruststorePassword(cluster_.truststorePassword());
  }

  connect.execute();

  started_ = true;
}

void Locator::stop() {
  cluster_.getGfsh().stop().locator().withDir(name_).execute();

  started_ = false;
}

Server::Server(Cluster &cluster, std::vector<Locator> &locators,
               std::string name, std::string xmlFile, bool useIPv6)
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

std::string Server::getCacheXMLFile() { return xmlFile_; }

Server::~Server() {
  try {
    if (started_) {
      stop();
    }
  } catch (...) {
  }
}

Server::Server(Server &&move)
    : cluster_(move.cluster_),
      locators_(move.locators_),
      serverAddress_(move.serverAddress_),
      started_(move.started_),
      name_(move.name_),
      xmlFile_(move.xmlFile_) {
  move.started_ = false;
}

void Server::start() {
  auto safeName = name_;
  std::replace(safeName.begin(), safeName.end(), '/', '_');

  auto server =
      cluster_.getGfsh()
          .start()
          .server()
          .withDir(name_)
          .withName(safeName)
          .withBindAddress(serverAddress_.address)
          .withPort(serverAddress_.port)
          .withMaxHeap("1g")
          .withLocators(locators_.front().getAddress().address + "[" +
                        std::to_string(locators_.front().getAddress().port) +
                        "]")
          .withClasspath(cluster_.getClasspath())
          .withSecurityManager(cluster_.getSecurityManager())
          .withCacheXMLFile(getCacheXMLFile())
          .withPreferIPv6(cluster_.getUseIPv6());

  if (!cluster_.getUser().empty()) {
    server.withUser(cluster_.getUser()).withPassword(cluster_.getPassword());
  }

  if (cluster_.useSsl()) {
    server.withSslEnabledComponents("all")
        .withSslRquireAuthentication(cluster_.requireSslAuthentication())
        .withSslKeystore(cluster_.keystore())
        .withSslTruststore(cluster_.truststore())
        .withSslKeystorePassword(cluster_.keystorePassword())
        .withSslTruststorePassword(cluster_.truststorePassword());
  }

  server.execute();

  started_ = true;
}

void Server::stop() {
  cluster_.getGfsh().stop().server().withDir(name_).execute();

  started_ = false;
}

Cluster::Cluster(LocatorCount initialLocators, ServerCount initialServers,
                 UseIpv6 useIPv6)
    : Cluster(
          Name(std::string(::testing::UnitTest::GetInstance()
                               ->current_test_info()
                               ->test_case_name()) +
               "/" +
               ::testing::UnitTest::GetInstance()->current_test_info()->name()),
          initialLocators, initialServers, useIPv6) {}

Cluster::Cluster(LocatorCount initialLocators, ServerCount initialServers)
    : Cluster(
          Name(std::string(::testing::UnitTest::GetInstance()
                               ->current_test_info()
                               ->test_case_name()) +
               "/" +
               ::testing::UnitTest::GetInstance()->current_test_info()->name()),
          initialLocators, initialServers) {}

Cluster::Cluster(LocatorCount initialLocators, ServerCount initialServers,
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

Cluster::Cluster(Name name, LocatorCount initialLocators,
                 ServerCount initialServers, UseIpv6 useIPv6)
    : Cluster(Name(name.get()), Classpath(""), SecurityManager(""), User(""),
              Password(""), initialLocators, initialServers, CacheXMLFiles({}),
              useIPv6) {}

Cluster::Cluster(Name name, LocatorCount initialLocators,
                 ServerCount initialServers)
    : Cluster(Name(name.get()), Classpath(""), SecurityManager(""), User(""),
              Password(""), initialLocators, initialServers, CacheXMLFiles({}),
              UseIpv6(false)) {}

Cluster::Cluster(Name name, Classpath classpath,
                 SecurityManager securityManager, User user, Password password,
                 LocatorCount initialLocators, ServerCount initialServers,
                 CacheXMLFiles cacheXMLFiles, UseIpv6 useIPv6)
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
}

Cluster::Cluster(Name name, Classpath classpath,
                 SecurityManager securityManager, User user, Password password,
                 LocatorCount initialLocators, ServerCount initialServers)
    : name_(name.get()),
      classpath_(classpath.get()),
      securityManager_(securityManager.get()),
      user_(user.get()),
      password_(password.get()),
      initialLocators_(initialLocators.get()),
      initialServers_(initialServers.get()) {
  jmxManagerPort_ = Framework::getAvailablePort();

  removeServerDirectory();
}

Cluster::~Cluster() {
  try {
    if (started_) {
      stop();
    }
  } catch (...) {
  }
}

void Cluster::removeServerDirectory() {
  boost::filesystem::path serverDir = boost::filesystem::relative(name_);
  boost::filesystem::remove_all(serverDir);
}

apache::geode::client::Cache Cluster::createCache() { return createCache({}); }

apache::geode::client::Cache Cluster::createCache(
    const std::unordered_map<std::string, std::string> &properties) {
  return createCache(properties, false);
}

apache::geode::client::Cache Cluster::createCache(
    const std::unordered_map<std::string, std::string> &properties,
    bool subscriptionEnabled) {
  using apache::geode::client::CacheFactory;

  CacheFactory cacheFactory;

  for (auto &&property : properties) {
    cacheFactory.set(property.first, property.second);
  }

  auto cache = cacheFactory.set("log-level", "debug")
                   .set("statistic-sampling-enabled", "false")
                   .set("log-file", "geode_native.log")
                   .create();

  auto poolFactory =
      cache.getPoolManager().createFactory().setSubscriptionEnabled(
          subscriptionEnabled);
  applyLocators(poolFactory);
  poolFactory.create("default");

  return cache;
}

void Cluster::applyLocators(apache::geode::client::PoolFactory &poolFactory) {
  for (const auto &locator : locators_) {
    poolFactory.addLocator(locator.getAddress().address,
                           locator.getAddress().port);
  }
}

Gfsh &Cluster::getGfsh() { return gfsh_; }

std::vector<Server> &Cluster::getServers() { return servers_; }

std::vector<Locator> &Cluster::getLocators() { return locators_; }

std::string &Cluster::getClasspath() { return classpath_; }

std::string &Cluster::getSecurityManager() { return securityManager_; }

std::string &Cluster::getUser() { return user_; }

std::string &Cluster::getPassword() { return password_; }

std::vector<std::string> &Cluster::getCacheXMLFiles() { return cacheXMLFiles_; }

bool Cluster::getUseIPv6() { return useIPv6_; }

void Cluster::start() { start(std::function<void()>()); }

void Cluster::start(std::function<void()> extraGfshCommands) {
  locators_.reserve(initialLocators_);
  for (size_t i = 0; i < initialLocators_; i++) {
    locators_.push_back({*this, locators_,
                         name_ + "/locator/" + std::to_string(i),
                         jmxManagerPort_, getUseIPv6()});
  }

  servers_.reserve(initialServers_);
  std::string xmlFile;
  for (size_t i = 0; i < initialServers_; i++) {
    xmlFile = (cacheXMLFiles_.size() == 0)
                  ? ""
                  : cacheXMLFiles_.size() == 1 ? cacheXMLFiles_[1]
                                               : cacheXMLFiles_[i];

    servers_.push_back({*this, locators_,
                        name_ + "/server/" + std::to_string(i), xmlFile,
                        getUseIPv6()});
  }

  startLocators();

  if (extraGfshCommands) {
    extraGfshCommands();
  }

  startServers();

  started_ = true;
}

std::string Cluster::getJmxManager() {
  return locators_.begin()->getAddress().address + "[" +
         std::to_string(jmxManagerPort_) + "]";
}

uint16_t Cluster::getLocatorPort() {
  return locators_.begin()->getAddress().port;
}

void Cluster::startServers() {
  std::vector<std::future<void>> futures;

  for (auto &server : this->servers_) {
    futures.push_back(std::async(std::launch::async, [&] { server.start(); }));
  }

  for (auto &future : futures) {
    future.get();
  }
}

void Cluster::startLocators() {
  std::vector<std::future<void>> futures;

  for (auto &locator : locators_) {
    futures.push_back(std::async(std::launch::async, [&] { locator.start(); }));
  }

  // TODO hack until there is a way to either tell servers to retry or wait
  // for single future.
  for (auto &future : futures) {
    future.get();
  }
}

void Cluster::stop() {
  std::vector<std::future<void>> futures;
  for (auto &server : servers_) {
    futures.push_back(std::async(std::launch::async, [&] { server.stop(); }));
  }

  for (auto &locator : locators_) {
    futures.push_back(std::async(std::launch::async, [&] { locator.stop(); }));
  }

  for (auto &future : futures) {
    future.wait();
  }

  started_ = false;
}

void Cluster::useSsl(const bool requireSslAuthentication,
                     const std::string keystore, const std::string truststore,
                     const std::string keystorePassword,
                     const std::string truststorePassword) {
  useSsl_ = true;
  requireSslAuthentication_ = requireSslAuthentication;
  keystore_ = keystore;
  truststore_ = truststore;
  keystorePassword_ = keystorePassword;
  truststorePassword_ = truststorePassword;
}

bool Cluster::useSsl() { return useSsl_; }

bool Cluster::requireSslAuthentication() { return requireSslAuthentication_; }

std::string Cluster::keystore() { return keystore_; }

std::string Cluster::truststore() { return truststore_; }

std::string Cluster::keystorePassword() { return keystorePassword_; }

std::string Cluster::truststorePassword() { return truststorePassword_; }
