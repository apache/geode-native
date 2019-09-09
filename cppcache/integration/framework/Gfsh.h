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

#include <iostream>
#include <string>

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

  class Deploy;
  Deploy deploy() { return Deploy(*this); }

  class Verb {
   public:
   protected:
    explicit Verb(Gfsh &gfsh) : gfsh_(gfsh) {}
    Gfsh &gfsh_;
  };

  template <class Result>
  class Command {
   public:
    virtual Result execute(const std::string &user, const std::string &password) { Result{gfsh_}.parse(gfsh_.execute(command_, user, password)); }
    virtual Result execute() { Result{gfsh_}.parse(gfsh_.execute(command_, "", "")); }

   protected:
    Command(Gfsh &gfsh, std::string command)
        : gfsh_(gfsh), command_(std::move(command)) {}
    Gfsh &gfsh_;
    std::string command_;
  };

  class Start {
   public:
    explicit Start(Gfsh &gfsh) : gfsh_(gfsh) {}

    class Server;
    Server server() { return Server{gfsh_}; };

    class Locator;
    Locator locator() { return Locator{gfsh_}; };

    class Locator : public Command<void> {
     public:
      explicit Locator(Gfsh &gfsh) : Command(gfsh, "start locator") {}

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

      Locator &withMaxHeap(const std::string maxHeap) {
        command_ += " --max-heap=" + maxHeap;
        return *this;
      };

      Locator &withClasspath(const std::string classpath) {
        if (!classpath.empty()) {
          command_ += " --classpath=" + classpath;
        }
        return *this;
      };

      Locator &withSecurityManager(const std::string securityManager) {
        if (!securityManager.empty()) {
          command_ += " --J=-Dgemfire.security-manager=" + securityManager;
        }
        return *this;
      };

      Locator &withConnect(const std::string connect) {
        command_ += " --connect=" + connect;
        return *this;
      };

      Locator &withPreferIPv6(bool useIPv6) {
        if (useIPv6) {
          command_ += " --J=-Djava.net.preferIPv6Addresses=true";
        }
        return *this;
      };
    };

    class Server : public Command<void> {
     public:
      explicit Server(Gfsh &gfsh) : Command(gfsh, "start server") {}

      Server &withName(const std::string &name) {
        command_ += " --name=" + name;
        return *this;
      }

      Server &withDir(const std::string &dir) {
        command_ += " --dir=" + dir;
        return *this;
      }

      Server &withBindAddress(const std::string &bindAddress) {
        command_ += " --bind-address=" + bindAddress;
        return *this;
      }

      Server &withPort(uint16_t serverPort) {
        command_ += " --server-port=" + std::to_string(serverPort);
        return *this;
      }

      Server &withLocators(const std::string locators) {
        command_ += " --locators=" + locators;
        return *this;
      }

      Server &withLogLevel(const std::string logLevel) {
        command_ += " --log-level=" + logLevel;
        return *this;
      }

      Server &withMaxHeap(const std::string maxHeap) {
        command_ += " --max-heap=" + maxHeap;
        return *this;
      }

      Server &withClasspath(const std::string classpath) {
        if (!classpath.empty()) {
          command_ += " --classpath=" + classpath;
        }
        return *this;
      }

      Server &withSecurityManager(const std::string securityManager) {
        if (!securityManager.empty()) {
          command_ += " --J=-Dgemfire.security-manager=" + securityManager;
        }
        return *this;
      }

      Server &withUser(const std::string user) {
        if (!user.empty()) {
          command_ += " --user=" + user;
        }

        return *this;
      }

      Server &withPassword(const std::string password) {
        if (!password.empty()) {
          command_ += " --password=" + password;
        }
        return *this;
      }

      Server &withCacheXMLFile(const std::string file) {
        if (!file.empty()) {
          command_ += " --cache-xml-file=" + file;
        }
        return *this;
      }

      Server &withPreferIPv6(bool useIPv6) {
        if (useIPv6) {
          command_ += " --J=-Djava.net.preferIPv6Addresses=true";
        }
        return *this;
      };
    };

   private:
    Gfsh &gfsh_;
  };

  class Stop {
   public:
    explicit Stop(Gfsh &gfsh) : gfsh_(gfsh) {}

    class Server;
    Server server() { return Server{gfsh_}; };

    class Locator;
    Locator locator() { return Locator{gfsh_}; };

    class Locator : public Command<void> {
     public:
      explicit Locator(Gfsh &gfsh) : Command(gfsh, "stop locator") {}

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
      explicit Server(Gfsh &gfsh) : Command(gfsh, "stop server") {}

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
    explicit Create(Gfsh &gfsh) : Verb{gfsh} {}

    class Region;
    Region region() { return Region{gfsh_}; };

    class Region : public Command<void> {
     public:
      explicit Region(Gfsh &gfsh) : Command(gfsh, "create region") {}

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
    explicit Connect(Gfsh &gfsh) : Command{gfsh, "connect"} {}

    Connect &withJmxManager(const std::string &jmxManager) {
      command_ += " --jmx-manager=" + jmxManager;
      return *this;
    };

    Connect &withUser(const std::string &user) {
      command_ += " --user=" + user;
      return *this;
    };

    Connect &withPassword(const std::string &password) {
      command_ += " --password=" + password;
      return *this;
    };
  };

  class Shutdown : public Command<void> {
   public:
    explicit Shutdown(Gfsh &gfsh) : Command{gfsh, "shutdown"} {}

    Shutdown &withIncludeLocators(bool includeLocators) {
      command_ += " --include-locators=" +
                  std::string(includeLocators ? "true" : "false");
      return *this;
    };
  };

  class Deploy : public Command<void> {
   public:
    explicit Deploy(Gfsh &gfsh) : Command{gfsh, "deploy"} {}

    Deploy &jar(const std::string &jarFile) {
      command_ += " --jars=" + jarFile;

      return *this;
    }
  };

 protected:
  virtual void execute(const std::string &command, const std::string &user, const std::string &password) = 0;
};

template <>
inline void Gfsh::Command<void>::execute(const std::string &user, const std::string &password) {
  gfsh_.execute(command_, user, password);
}

template <>
inline void Gfsh::Command<void>::execute() {
  gfsh_.execute(command_, "", "");
}


#endif  // INTEGRATION_TEST_FRAMEWORK_GFSH_H
