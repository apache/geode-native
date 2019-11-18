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
  int returnCode_;

 public:
  GfshExecuteException(std::string message, int returnCode);
  ~GfshExecuteException() noexcept override;
  std::string getName() const override;
  int getGfshReturnCode();
};

class GfshExecute : public Gfsh {
  std::string connection_;

  void execute(const std::string &command, const std::string &user,
               const std::string &password) override;

  boost::process::child executeChild(std::vector<std::string> &commands,
                                     boost::process::environment &env,
                                     boost::process::ipstream &outStream,
                                     boost::process::ipstream &errStream);

  void extractConnectionCommand(const std::string &command,
                                const std::string &user,
                                const std::string &password);

 public:
  GfshExecute() = default;
  virtual ~GfshExecute() override = default;
};

#endif  // INTEGRATION_TEST_FRAMEWORK_GFSHEXECUTE_H
