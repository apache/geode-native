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

#ifndef INTEGRATION_TEST_FRAMEWORK_GFSH_H
#define INTEGRATION_TEST_FRAMEWORK_GFSH_H

#include <string>
#include <iostream>

#include <boost/process.hpp>
#include <boost/log/trivial.hpp>

class Gfsh {
 public:
  Gfsh() = default;
  virtual ~Gfsh() = default;

  class Start;
  Start start() { return Start{*this}; }

  class Stop;
  Stop stop() { return Stop{*this}; }

  class Create;
  Create create() { return Create{*this}; }

  class Connect;
  Connect connect() { return Connect{*this}; }

  class Shutdown;
  Shutdown shutdown() { return Shutdown{*this}; }

  class Verb {
   public:
   protected:
    Verb(Gfsh &gfsh) : gfsh_(gfsh) {}
    Gfsh &gfsh_;
  };

  template <class Result>
  class Command {
   public:
    virtual Result execute() { Result{gfsh_}.parse(gfsh_.execute(command_)); }

   protected:
    Command(Gfsh &gfsh, std::string command)
        : gfsh_(gfsh), command_(std::move(command)) {}
    Gfsh &gfsh_;
    std::string command_;
  };

  class Start {
   public:
    Start(Gfsh &gfsh) : gfsh_(gfsh) {}

    class Server;
    Server server() { return Server{gfsh_}; };

    class Locator;
    Locator locator() { return Locator{gfsh_}; };

    class Locator : public Command<void> {
     public:
      Locator(Gfsh &gfsh) : Command(gfsh, "start locator") {}

      Locator &withName(const std::string &name) {
        command_ += " --name=" + name;
        return *this;
      };

      Locator &withDir(const std::string &dir) {
        command_ += " --dir=" + dir;
        return *this;
      };

      Locator &withBindAddress(const std::string &bindAddress) {
        command_ += " --bind-address=" + bindAddress;
        return *this;
      };

      Locator &withPort(uint16_t port) {
        command_ += " --port=" + std::to_string(port);
        return *this;
      };

      Locator &withJmxManagerPort(uint16_t jmxManagerPort) {
        command_ +=
            " --J=-Dgemfire.jmx-manager-port=" + std::to_string(jmxManagerPort);
        return *this;
      };

      Locator &withHttpServicePort(uint16_t httpServicePort) {
        command_ += " --http-service-port=" + std::to_string(httpServicePort);
        return *this;
      };

      Locator &withLogLevel(const std::string logLevel) {
        command_ += " --log-level=" + logLevel;
        return *this;
      };
    };

    class Server : public Command<void> {
     public:
      Server(Gfsh &gfsh) : Command(gfsh, "start server") {}

      Server &withName(const std::string &name) {
        command_ += " --name=" + name;
        return *this;
      };

      Server &withDir(const std::string &dir) {
        command_ += " --dir=" + dir;
        return *this;
      };

      Server &withBindAddress(const std::string &bindAddress) {
        command_ += " --bind-address=" + bindAddress;
        return *this;
      };

      Server &withPort(uint16_t serverPort) {
        command_ += " --server-port=" + std::to_string(serverPort);
        return *this;
      };

      Server &withLocators(const std::string locators) {
        command_ += " --locators=" + locators;
        return *this;
      };

      Server &withLogLevel(const std::string logLevel) {
        command_ += " --log-level=" + logLevel;
        return *this;
      };
    };

   private:
    Gfsh &gfsh_;
  };

  class Stop {
   public:
    Stop(Gfsh &gfsh) : gfsh_(gfsh) {}

    class Server;
    Server server() { return Server{gfsh_}; };

    class Locator;
    Locator locator() { return Locator{gfsh_}; };

    class Locator : public Command<void> {
     public:
      Locator(Gfsh &gfsh) : Command(gfsh, "stop locator") {}

      Locator &withName(const std::string &name) {
        command_ += " --name=" + name;
        return *this;
      };

      Locator &withDir(const std::string &dir) {
        command_ += " --dir=" + dir;
        return *this;
      };
    };

    class Server : public Command<void> {
     public:
      Server(Gfsh &gfsh) : Command(gfsh, "stop server") {}

      Server &withName(const std::string &name) {
        command_ += " --name=" + name;
        return *this;
      };

      Server &withDir(const std::string &dir) {
        command_ += " --dir=" + dir;
        return *this;
      };
    };

   private:
    Gfsh &gfsh_;
  };

  class Create : public Verb {
   public:
    Create(Gfsh &gfsh) : Verb{gfsh} {}

    class Region;
    Region region() { return Region{gfsh_}; };

    class Region : public Command<void> {
     public:
      Region(Gfsh &gfsh) : Command(gfsh, "create region") {}

      Region &withName(const std::string &name) {
        command_ += " --name=" + name;
        return *this;
      };

      Region &withType(const std::string &type) {
        command_ += " --type=" + type;
        return *this;
      };
    };
  };

  class Connect : public Command<void> {
   public:
    Connect(Gfsh &gfsh) : Command{gfsh, "connect"} {}

    Connect &withJmxManager(const std::string &jmxManager) {
      command_ += " --jmx-manager=" + jmxManager;
      return *this;
    };
  };

  class Shutdown : public Command<void> {
   public:
    Shutdown(Gfsh &gfsh) : Command{gfsh, "shutdown"} {}

    Shutdown &withIncludeLocators(bool includeLocators) {
      command_ += " --include-locators=" +
                  std::string(includeLocators ? "true" : "false");
      return *this;
    };
  };

 protected:
  virtual void execute(const std::string &command) = 0;
};

template <>
inline void Gfsh::Command<void>::execute() {
  gfsh_.execute(command_);
}

#endif  // INTEGRATION_TEST_FRAMEWORK_GFSH_H
