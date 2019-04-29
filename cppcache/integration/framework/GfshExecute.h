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

#ifndef INTEGRATION_TEST_FRAMEWORK_GFSHEXECUTE_H
#define INTEGRATION_TEST_FRAMEWORK_GFSHEXECUTE_H

#include <algorithm>
#include <iostream>
#include <regex>
#include <string>

#include <geode/Exception.hpp>

#pragma error_messages(off, oklambdaretmulti, wvarhidemem, \
                       w_constexprnonlitret, explctspectypename)
#include <boost/process.hpp>
#pragma error_messages(on, oklambdaretmulti, wvarhidemem, \
                       w_constexprnonlitret, explctspectypename)

#include "Gfsh.h"
#include "TestConfig.h"

template <class _T>
bool starts_with(const _T &input, const _T &match) {
  return input.size() >= match.size() &&
         std::equal(std::begin(match), std::end(match), std::begin(input));
}

class GfshExecuteException : public apache::geode::client::Exception {
 public:
  GfshExecuteException(std::string message, int returnCode)
      : apache::geode::client::Exception(message), returnCode_(returnCode) {}
  ~GfshExecuteException() noexcept override {}
  std::string getName() const override { return "GfshExecuteException"; }
  int getGfshReturnCode() { return returnCode_; }

 private:
  int returnCode_;
};

class GfshExecute : public Gfsh {
 public:
  GfshExecute() = default;
  virtual ~GfshExecute() override = default;

  class Connect : public Command<void> {
   public:
    explicit Connect(Gfsh &gfsh) : Command{gfsh, "connect"} {}

    Connect &withJmxManager(const std::string &jmxManager) {
      command_ += " --jmx-manager=" + jmxManager;
      return *this;
    };
  };

 protected:
  void execute(const std::string &command) override;

  boost::process::child executeChild(std::vector<std::string> &commands,
                                     boost::process::environment &env,
                                     boost::process::ipstream &outStream,
                                     boost::process::ipstream &errStream);

  void extractConnectionCommand(const std::string &command) {
    if (starts_with(command, std::string("connect"))) {
      connection_ = command;
    } else if (starts_with(command, std::string("start locator"))) {
      auto jmxManagerHost = std::string("localhost");
      auto jmxManagerPort = std::string("1099");

      std::regex jmxManagerHostRegex("bind-address=([^\\s]+)");
      std::smatch jmxManagerHostMatch;
      if (std::regex_search(command, jmxManagerHostMatch,
                            jmxManagerHostRegex)) {
        jmxManagerHost = jmxManagerHostMatch[1];
      }

      std::regex jmxManagerPortRegex("jmx-manager-port=(\\d+)");
      std::smatch jmxManagerPortMatch;
      if (std::regex_search(command, jmxManagerPortMatch,
                            jmxManagerPortRegex)) {
        jmxManagerPort = jmxManagerPortMatch[1];
      }

      connection_ = "connect --jmx-manager=" + jmxManagerHost + "[" +
                    jmxManagerPort + "]";
    }
  }

 private:
  std::string connection_;
};

#endif  // INTEGRATION_TEST_FRAMEWORK_GFSHEXECUTE_H
