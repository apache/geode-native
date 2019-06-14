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
        .withSslKeystore(cluster_.keystore())
        .withSslTruststore(cluster_.truststore())
        .withSslKeystorePassword(cluster_.keystorePassword())
        .withSslTruststorePassword(cluster_.truststorePassword());
  }

  locator.execute(cluster_.getUser(), cluster_.getPassword());

  auto connect = cluster_.getGfsh()
      .connect()
      .withJmxManager(cluster_.getJmxManager());

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

void Server::start() {
  auto safeName = name_;
  std::replace(safeName.begin(), safeName.end(), '/', '_');

  auto server = cluster_.getGfsh()
      .start()
      .server()
      .withDir(name_)
      .withName(safeName)
      .withBindAddress(serverAddress_.address)
      .withPort(serverAddress_.port)
      .withMaxHeap("1g")
      .withLocators(locators_.front().getAddress().address + "[" +
                    std::to_string(locators_.front().getAddress().port) + "]")
      .withClasspath(cluster_.getClasspath())
      .withSecurityManager(cluster_.getSecurityManager())
      .withUser(cluster_.getUser())
      .withPassword(cluster_.getPassword())
      .withCacheXMLFile(getCacheXMLFile())
      .withPreferIPv6(cluster_.getUseIPv6());

  if (cluster_.useSsl()) {
    server.withSslEnabledComponents("all")
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
    xmlFile = (cacheXMLFiles_.size() == 0) ? "" :
               cacheXMLFiles_.size() == 1 ? cacheXMLFiles_[1] :
               cacheXMLFiles_[i];

    servers_.push_back(
      {*this, locators_, name_ + "/server/" + std::to_string(i), xmlFile, getUseIPv6()});
  }

  startLocators();

  extraGfshCommands();

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

void Cluster::useSsl(const std::string keystore, const std::string truststore, const std::string keystorePassword, const std::string truststorePassword) {
  useSsl_ = true;
  keystore_ = keystore;
  truststore_ = truststore;
  keystorePassword_ = keystorePassword;
  truststorePassword_ = truststorePassword;
}

bool Cluster::useSsl() {
  return useSsl_;
}

std::string Cluster::keystore() {
  return keystore_;
}

std::string Cluster::truststore() {
  return truststore_;
}

std::string Cluster::keystorePassword() {
  return keystorePassword_;
}

std::string Cluster::truststorePassword() {
  return truststorePassword_;
}
