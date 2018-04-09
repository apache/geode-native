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
  // broken on windows env["JAVA_ARGS"] = "-Xmx1g -client";

  // pipes broken on windows.
  // ipstream outStream;
  // ipstream errStream;
  child gfsh(GFSH_EXECUTABLE, args = commands, env, std_out > null,
             std_err > null, std_in < null);

  // std::string line;

  // while (outStream && std::getline(outStream, line) && !line.empty())
  //  BOOST_LOG_TRIVIAL(debug) << "Gfsh::execute: " << line;

  // while (errStream && std::getline(errStream, line) && !line.empty())
  //  BOOST_LOG_TRIVIAL(error) << "Gfsh::execute: " << line;

  gfsh.wait();

  extractConnectionCommand(command);
}
