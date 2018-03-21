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

#ifndef INTEGRATION_TEST_FRAMEWORK_GFSHPROCESS_H
#define INTEGRATION_TEST_FRAMEWORK_GFSHPROCESS_H

#include <string>
#include <iostream>

#include <boost/process.hpp>
#include <boost/log/trivial.hpp>
#include <boost/asio/read_until.hpp>

#include "Gfsh.h"
#include "config.h"

class GfshProcess : public Gfsh {
 public:
  GfshProcess() : stdout(ios) { startChild(); }

  ~GfshProcess() noexcept override { stopChild(); }

 protected:
  void execute(const std::string &command) override {
    BOOST_LOG_TRIVIAL(info) << "Gfsh::execute: " << command;

    using namespace boost::process;

    stdin << command << std::endl;
    BOOST_LOG_TRIVIAL(info) << "Gfsh::execute: sent: " << command;
    boost::asio::streambuf buf;
    boost::asio::async_read_until(
        stdout, buf, "gfsh>",
        [&](const boost::system::error_code &ec, std::size_t size) {
          buf.commit(size);
          BOOST_LOG_TRIVIAL(info) << "Gfsh::execute: ready";
        });

    ios.run();
    BOOST_LOG_TRIVIAL(info) << "Gfsh::execute: end";
  }

 private:
  boost::asio::io_service ios;
  boost::process::async_pipe stdout;
  boost::process::opstream stdin;
  boost::process::child gfsh;

  void startChild() {
    using namespace boost::process;

    auto env = boost::this_process::environment();
    env["JLINE_TERMINAL"] = "-Djline.terminal=jline.UnsupportedTerminal";

    gfsh = child(
        GFSH_EXECUTABLE,
        env, std_out > stdout, std_in < stdin);

    boost::asio::streambuf buf;
    boost::asio::async_read_until(
        stdout, buf, "gfsh>",
        [&](const boost::system::error_code &ec, std::size_t size) {
          buf.commit(size);
          BOOST_LOG_TRIVIAL(info) << "Gfsh::startChild: ready";
        });

    ios.run();
  }

  void stopChild() {
    stdin << "exit" << std::endl;
    BOOST_LOG_TRIVIAL(info) << "Gfsh::stopChild: sent: exit";

    gfsh.wait();
  }
};

#endif  // INTEGRATION_TEST_FRAMEWORK_GFSHPROCESS_H
