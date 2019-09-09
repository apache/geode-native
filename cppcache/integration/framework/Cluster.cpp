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

  cluster_.getGfsh()
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
      .execute(cluster_.getUser(), cluster_.getPassword());

  started_ = true;
}

void Locator::stop() {
  cluster_.getGfsh().stop().locator().withDir(name_).execute();

//  std::cout << "locator: " << locatorAddress_.port << ": stopped" << std::endl << std::flush;
  started_ = false;
}

void Server::start() {
  auto safeName = name_;
  std::replace(safeName.begin(), safeName.end(), '/', '_');

  cluster_.getGfsh()
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
      .withPreferIPv6(cluster_.getUseIPv6())
      .execute();

//  std::cout << "server: " << serverAddress_.port << ": started" << std::endl << std::flush;

  started_ = true;
}

void Server::stop() {
  cluster_.getGfsh().stop().server().withDir(name_).execute();

//  std::cout << "server: " << serverAddress_.port << ": stopped" << std::endl << std::flush;
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

  //    std::cout << "cluster: " << jmxManagerPort_ << ": started" << std::endl;
  started_ = true;
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

  //    std::cout << "cluster: " << jmxManagerPort_ << ": stopped" << std::endl;
  started_ = false;
}
