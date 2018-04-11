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

#include <boost/log/trivial.hpp>

#if defined(_WINDOWS)
std::mutex g_child_mutex;
#endif

void GfshExecute::execute(const std::string &command) {
  BOOST_LOG_TRIVIAL(info) << "Gfsh::execute: " << command;

  using namespace boost::process;

  std::vector<std::string> commands;
  if (!connection_.empty()) {
    commands.push_back("-e");
    commands.push_back(connection_);
  }
  commands.push_back("-e");
  commands.push_back(command);

  auto env = boost::this_process::environment();
  environment _env = env;
  // broken on windows env["JAVA_ARGS"] = "-Xmx1g -client";

  ipstream outStream;
  ipstream errStream;

  auto gfsh = executeChild(commands, _env, outStream, errStream);

  std::string line;

  while (outStream && std::getline(outStream, line)) {
    BOOST_LOG_TRIVIAL(debug) << "Gfsh::execute: " << line;
  }

  while (errStream && std::getline(errStream, line)) {
    BOOST_LOG_TRIVIAL(error) << "Gfsh::execute: " << line;
  }

  gfsh.wait();

  auto exit_code = gfsh.exit_code();
  BOOST_LOG_TRIVIAL(debug) << "Gfsh::execute: exit:" << exit_code;

  extractConnectionCommand(command);
}

boost::process::child GfshExecute::executeChild(
    std::vector<std::string> &commands, boost::process::environment &env,
    boost::process::ipstream &outStream, boost::process::ipstream &errStream) {
  using namespace boost::process;

#if defined(_WINDOWS)
  // https://github.com/klemens-morgenstern/boost-process/issues/159
  std::lock_guard<std::mutex> guard(g_child_mutex);
#endif
  return child(GFSH_EXECUTABLE, args = commands, env, std_out > outStream,
               std_err > errStream);
}
