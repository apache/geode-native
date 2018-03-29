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

void Locator::start() {
  if (started_) return;

  cluster_.getGfsh()
      .start()
      .locator()
      .withDir(name_)
      .withBindAddress(locatorAddress_.address)
      .withPort(locatorAddress_.port)
      .withJmxManagerPort(jmxManagerPort_)
      .withHttpServicePort(0)
      .execute();

  //    std::cout << "locator: " << locatorAddress_.port << ": started"
  //              << std::endl;
  started_ = true;
}

void Locator::stop() {
  cluster_.getGfsh().stop().locator().withDir(name_).execute();

  //    std::cout << "locator: " << locatorAddress_.port << ": stopped"
  //              << std::endl;
  started_ = false;
}

void Server::start() {
  cluster_.getGfsh()
      .start()
      .server()
      .withDir(name_)
      .withBindAddress(serverAddress_.address)
      .withPort(serverAddress_.port)
      .withLocators(locators_.front().getAdddress().address + "[" +
                    std::to_string(locators_.front().getAdddress().port) + "]")
      .execute();

  //    std::cout << "server: " << serverAddress_.port << ": started" <<
  //    std::endl;
  started_ = true;
}

void Server::stop() {
  cluster_.getGfsh().stop().server().withDir(name_).execute();

  //    std::cout << "server: " << serverAddress_.port << ": stopped" <<
  //    std::endl;
  started_ = false;
}

void Cluster::start() {
  locators_.reserve(initialLocators_);
  for (size_t i = 0; i < initialLocators_; i++) {
    locators_.push_back({*this, locators_,
                         name_ + "/locator/" + std::to_string(i),
                         jmxManagerPort_});
  }

  servers_.reserve(initialServers_);
  for (size_t i = 0; i < initialServers_; i++) {
    servers_.push_back(
        {*this, locators_, name_ + "/server/" + std::to_string(i)});
  }

  std::vector<std::future<void>> futures;

  for (auto &locator : locators_) {
    futures.push_back(std::async(std::launch::async, [&] { locator.start(); }));
  }

  // TODO hack until there is a way to either tell servers to retry or wait
  // for single future.
  for (auto &future : futures) {
    future.wait();
  }

  for (auto &server : servers_) {
    futures.push_back(std::async(std::launch::async, [&] { server.start(); }));
  }

  for (auto &future : futures) {
    future.wait();
  }

  //    std::cout << "cluster: " << jmxManagerPort_ << ": started" << std::endl;
  started_ = true;
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
