/*
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
#include <vector>

class Gfsh {
 public:
  Gfsh() = default;
  virtual ~Gfsh() = default;

  class Start;
  Start start();

  class Stop;
  Stop stop();

  class Create;
  Create create();

  class Destroy;
  Destroy destroy();

  class Connect;
  Connect connect();

  class Shutdown;
  Shutdown shutdown();

  class Deploy;
  Deploy deploy();

  class Rebalance;
  Rebalance rebalance();

  class ExecuteFunction;
  ExecuteFunction executeFunction();

  class Verb {
   public:
   protected:
    explicit Verb(Gfsh &gfsh);
    Gfsh &gfsh_;
  };

  template <class Result>
  class Command {
   public:
    virtual Result execute(const std::string &user, const std::string &password,
                           const std::string &keyStorePath,
                           const std::string &trustStorePath,
                           const std::string &keyStorePassword,
                           const std::string &trustStorePassword) {
      Result{gfsh_}.parse(gfsh_.execute(command_, user, password, keyStorePath,
                                        trustStorePath, keyStorePassword,
                                        trustStorePassword));
    }
    virtual Result execute() {
      Result{gfsh_}.parse(gfsh_.execute(command_, "", "", "", "", "", ""));
    }
    virtual std::string toString() { return command_; }

   protected:
    Command(Gfsh &gfsh, std::string command)
        : gfsh_(gfsh), command_(std::move(command)) {}
    Gfsh &gfsh_;
    std::string command_;
  };

  class Start {
   public:
    explicit Start(Gfsh &gfsh);

    class Server;
    Server server();

    class Locator;
    Locator locator();

    class Locator : public Command<void> {
     public:
      explicit Locator(Gfsh &gfsh);

      Locator &withName(const std::string &name);

      Locator &withDir(const std::string &dir);

      Locator &withBindAddress(const std::string &bindAddress);

      Locator &withPort(const uint16_t &port);

      Locator &withRemoteLocators(
          const std::vector<std::string> &remoteLocators);

      Locator &withAllowAttach(const bool allowAttach,
                               const std::string &bindAddress);

      Locator &withDistributedSystemId(const uint16_t &dsId);

      Locator &withJmxManagerPort(const uint16_t &jmxManagerPort);

      Locator &withHttpServicePort(const uint16_t &httpServicePort);

      Locator &withLogLevel(const std::string &logLevel);

      Locator &withMaxHeap(const std::string &maxHeap);

      Locator &withClasspath(const std::string classpath);

      Locator &withSecurityManager(const std::string securityManager);

      Locator &withPreferIPv6(bool useIPv6);

      Locator &withSslEnabledComponents(const std::string &components);

      Locator &withSslKeystore(const std::string &keystore);

      Locator &withSslTruststore(const std::string &truststore);

      Locator &withSslKeystorePassword(const std::string &keystorePassword);

      Locator &withSslTruststorePassword(const std::string &truststorePassword);

      Locator &withConnect(const bool connect);

      Locator &withJmxManagerStart(const bool startJmxManager);

      Locator &withSslRequireAuthentication(const bool require);

      Locator &withPropertiesFile(const std::string file);

      Locator &withSecurityPropertiesFile(const std::string file);

      Locator &withHostNameForClients(const std::string hostName);
    };

    class Server : public Command<void> {
     public:
      explicit Server(Gfsh &gfsh);

      Server &withName(const std::string &name);

      Server &withDir(const std::string &dir);

      Server &withBindAddress(const std::string &bindAddress);

      Server &withPort(const uint16_t &serverPort);

      Server &withLocators(const std::string &locators);

      Server &withLogLevel(const std::string &logLevel);

      Server &withMaxHeap(const std::string &maxHeap);

      Server &withClasspath(const std::string classpath);

      Server &withSecurityManager(const std::string securityManager);

      Server &withUser(const std::string user);

      Server &withPassword(const std::string password);

      Server &withCacheXMLFile(const std::string file);

      Server &withPreferIPv6(bool useIPv6);

      Server &withAllowAttach(const bool allowAttach,
                              const std::string &bindAddress);

      Server &withSslEnabledComponents(const std::string &components);

      Server &withSslKeystore(const std::string &keystore);

      Server &withSslTruststore(const std::string &truststore);

      Server &withSslKeystorePassword(const std::string &keystorePassword);

      Server &withSslTruststorePassword(const std::string &truststorePassword);

      Server &withSslRquireAuthentication(const bool require);

      Server &withPropertiesFile(const std::string file);

      Server &withSecurityPropertiesFile(const std::string file);

      Server &withHostNameForClients(const std::string hostName);

      Server &withSystemProperty(const std::string &key,
                                 const std::string &value);
      Server &withConserveSockets(bool conserveSockets);
    };

   private:
    Gfsh &gfsh_;
  };

  class Stop {
   public:
    explicit Stop(Gfsh &gfsh);

    class Server;
    Server server();

    class Locator;
    Locator locator();

    class Locator : public Command<void> {
     public:
      explicit Locator(Gfsh &gfsh);

      Locator &withName(const std::string &name);

      Locator &withDir(const std::string &dir);
    };

    class Server : public Command<void> {
     public:
      explicit Server(Gfsh &gfsh);

      Server &withName(const std::string &name);

      Server &withDir(const std::string &dir);
    };

   private:
    Gfsh &gfsh_;
  };

  class Create : public Verb {
   public:
    explicit Create(Gfsh &gfsh);

    class Region;
    Region region();

    class GatewaySender;
    GatewaySender gatewaySender();

    class GatewayReceiver;
    GatewayReceiver gatewayReceiver();

    class Region : public Command<void> {
     public:
      explicit Region(Gfsh &gfsh);

      Region &withName(const std::string &name);

      Region &withType(const std::string &type);

      Region &withRedundantCopies(const std::string &copies);

      Region &withBuckets(const std::string &totalNumBuckets);

      Region &withGatewaySenderId(const std::string &gatewaySenderId);

      Region &withPartitionResolver(const std::string &partitionResolver);
    };

    class GatewaySender : public Command<void> {
     public:
      explicit GatewaySender(Gfsh &gfsh);

      GatewaySender &withId(const std::string &id);

      GatewaySender &withRemoteDSId(const std::string &remoteDSId);
    };

    class GatewayReceiver : public Command<void> {
     public:
      explicit GatewayReceiver(Gfsh &gfsh);
    };
  };

  class Destroy : public Verb {
   public:
    explicit Destroy(Gfsh &gfsh);

    class Region;
    Region region();

    class Region : public Command<void> {
     public:
      explicit Region(Gfsh &gfsh);

      Region &withName(const std::string &name);

      Region &ifExists();
    };
  };

  class Connect : public Command<void> {
   public:
    explicit Connect(Gfsh &gfsh);

    Connect &withJmxManager(const std::string &jmxManager);

    Connect &withUser(const std::string &user);

    Connect &withPassword(const std::string &password);

    Connect &withUseSsl(const bool useSsl);

    Connect &withKeystore(const std::string &keystore);

    Connect &withTruststore(const std::string &truststore);

    Connect &withKeystorePassword(const std::string &keystorePassword);

    Connect &withTruststorePassword(const std::string &truststorePassword);
  };

  class Shutdown : public Command<void> {
   public:
    explicit Shutdown(Gfsh &gfsh);

    Shutdown &withIncludeLocators(bool includeLocators);
  };

  class Deploy : public Command<void> {
   public:
    explicit Deploy(Gfsh &gfsh);

    Deploy &jar(const std::string &jarFile);
  };

  class Rebalance : public Command<void> {
   public:
    explicit Rebalance(Gfsh &gfsh);
  };

  class ExecuteFunction : public Command<void> {
   public:
    explicit ExecuteFunction(Gfsh &gfsh);

    ExecuteFunction &withId(const std::string &functionName);
    ExecuteFunction &withMember(const std::string &withMember);
  };

 protected:
  virtual void execute(const std::string &command, const std::string &user,
                       const std::string &password,
                       const std::string &keyStorePath,
                       const std::string &trustStorePath,
                       const std::string &keyStorePassword,
                       const std::string &trustStorePassword) = 0;
};

template <>
void Gfsh::Command<void>::execute(const std::string &user,
                                  const std::string &password,
                                  const std::string &keyStorePath,
                                  const std::string &trustStorePath,
                                  const std::string &keyStorePassword,
                                  const std::string &trustStorePassword);

template <>
void Gfsh::Command<void>::execute();

#endif  // INTEGRATION_TEST_FRAMEWORK_GFSH_H
