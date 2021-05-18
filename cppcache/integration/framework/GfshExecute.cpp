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

#include "GfshExecute.h"

#include <mutex>

// Disable warning for "extra qualifications" here.  One of the boost log
// headers triggers this warning.  Note: use of disable pragma here is
// intentional - attempts to use push/pop as you ordinarily should just
// yielded a gripe from the MS tools that "warning number '4596' is not a
// valid compiler warning". re-enabling the warning after the include
// fails in the same way, so just leave it disabled for the rest of the
// file.  This is safe, since the warning can only trigger inside a class
// declaration, of which there are none in this file.
#ifdef WIN32
#pragma warning(disable : 4596)
#endif
#include <boost/log/trivial.hpp>

#if defined(_WINDOWS)
std::mutex g_child_mutex;
#endif

using boost::process::args;
using boost::process::child;
using boost::process::environment;
using boost::process::ipstream;
using boost::process::std_err;
using boost::process::std_in;
using boost::process::std_out;

GfshExecuteException::GfshExecuteException(std::string message, int returnCode)
    : apache::geode::client::Exception(message), returnCode_(returnCode) {}

GfshExecuteException::~GfshExecuteException() {}

std::string GfshExecuteException::getName() const {
  return "GfshExecuteException";
}

int GfshExecuteException::getGfshReturnCode() { return returnCode_; }

void GfshExecute::execute(const std::string &command, const std::string &user,
                          const std::string &password,
                          const std::string &keyStorePath,
                          const std::string &trustStorePath,
                          const std::string &keyStorePassword,
                          const std::string &trustStorePassword) {
  BOOST_LOG_TRIVIAL(info) << "Gfsh::execute: " << command;

  std::vector<std::string> commands;
  if (!connection_.empty()) {
    commands.push_back("-e");
    commands.push_back(connection_);
  }
  commands.push_back("-e");
  commands.push_back(command);

  environment env{boost::this_process::environment()};

  if (!maxHeap_.empty()) {
    env["JAVA_ARGS"] = "-Xmx" + maxHeap_ + " " + env["JAVA_ARGS"].to_string();
  }

  ipstream outStream;
  ipstream errStream;

  auto gfsh = executeChild(commands, env, outStream, errStream);

  std::string line;

  while (outStream && std::getline(outStream, line)) {
    BOOST_LOG_TRIVIAL(trace) << "Gfsh::execute: " << line;
  }

  while (errStream && std::getline(errStream, line)) {
    BOOST_LOG_TRIVIAL(debug) << "Gfsh::execute: " << line;
  }

  gfsh.wait();

  auto exit_code = gfsh.exit_code();
  BOOST_LOG_TRIVIAL(debug) << "Gfsh::execute: exit:" << exit_code;

  if (exit_code) {
    throw GfshExecuteException("gfsh error", exit_code);
  }
  extractConnectionCommand(command, user, password, keyStorePath,
                           trustStorePath, keyStorePassword,
                           trustStorePassword);
}

child GfshExecute::executeChild(std::vector<std::string> &commands,
                                environment &env, ipstream &outStream,
                                ipstream &errStream) {
#if defined(_WINDOWS)
  // https://github.com/klemens-morgenstern/boost-process/issues/159
  std::lock_guard<std::mutex> guard(g_child_mutex);
#endif
  return child(getFrameworkString(FrameworkVariable::GfShExecutable),
               args = commands, env, std_out > outStream, std_err > errStream,
               std_in < boost::process::null);
}

void GfshExecute::extractConnectionCommand(
    const std::string &command, const std::string &user,
    const std::string &password, const std::string &keyStorePath,
    const std::string &trustStorePath, const std::string &keyStorePassword,
    const std::string &trustStorePassword) {
  if (starts_with(command, std::string("connect"))) {
    connection_ = command;
  } else if (starts_with(command, std::string("start locator"))) {
    auto jmxManagerHost = std::string("localhost");
    auto jmxManagerPort = std::string("1099");

    std::regex jmxManagerHostRegex("bind-address=([^\\s]+)");
    std::smatch jmxManagerHostMatch;
    if (std::regex_search(command, jmxManagerHostMatch, jmxManagerHostRegex)) {
      jmxManagerHost = jmxManagerHostMatch[1];
    }

    std::regex jmxManagerPortRegex("jmx-manager-port=(\\d+)");
    std::smatch jmxManagerPortMatch;
    if (std::regex_search(command, jmxManagerPortMatch, jmxManagerPortRegex)) {
      jmxManagerPort = jmxManagerPortMatch[1];
    }

    connection_ =
        "connect --jmx-manager=" + jmxManagerHost + "[" + jmxManagerPort + "]";

    if (!(user.empty() || password.empty())) {
      connection_ += " --user=" + user + " --password=" + password;
    }

    if (!(keyStorePath.empty() || trustStorePath.empty() ||
          keyStorePassword.empty() || trustStorePassword.empty())) {
      connection_ += " --use-ssl=true --key-store=" + keyStorePath +
                     " --trust-store=" + trustStorePath +
                     " --key-store-password=" + keyStorePassword +
                     " --trust-store-password=" + trustStorePassword;
    }
  }
}

GfshExecute &GfshExecute::withMaxHeap(std::string maxHeap) {
  maxHeap_ = std::move(maxHeap);
  return *this;
}
