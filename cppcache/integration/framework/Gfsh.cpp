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

#include "Gfsh.h"

#include <iterator>
#include <sstream>

Gfsh::Start Gfsh::start() { return Start{*this}; }

Gfsh::Stop Gfsh::stop() { return Stop{*this}; }

Gfsh::Create Gfsh::create() { return Create{*this}; }

Gfsh::Destroy Gfsh::destroy() { return Destroy(*this); }

Gfsh::Connect Gfsh::connect() { return Connect{*this}; }

Gfsh::Shutdown Gfsh::shutdown() { return Shutdown{*this}; }

Gfsh::Deploy Gfsh::deploy() { return Deploy(*this); }

Gfsh::ExecuteFunction Gfsh::executeFunction() { return ExecuteFunction(*this); }

Gfsh::Verb::Verb(Gfsh &gfsh) : gfsh_(gfsh) {}

Gfsh::Start::Start(Gfsh &gfsh) : gfsh_(gfsh) {}

Gfsh::Start::Server Gfsh::Start::server() { return Server{gfsh_}; }

Gfsh::Start::Locator Gfsh::Start::locator() { return Locator{gfsh_}; }

Gfsh::Start::Locator::Locator(Gfsh &gfsh) : Command(gfsh, "start locator") {}

Gfsh::Start::Locator &Gfsh::Start::Locator::withName(const std::string &name) {
  command_ += " --name=" + name;
  return *this;
}

Gfsh::Start::Locator &Gfsh::Start::Locator::withDir(const std::string &dir) {
  command_ += " --dir=" + dir;
  return *this;
}

Gfsh::Start::Locator &Gfsh::Start::Locator::withBindAddress(
    const std::string &bindAddress) {
  command_ += " --bind-address=" + bindAddress;
  return *this;
}

Gfsh::Start::Locator &Gfsh::Start::Locator::withPort(const uint16_t &port) {
  command_ += " --port=" + std::to_string(port);
  return *this;
}

Gfsh::Start::Locator &Gfsh::Start::Locator::withRemoteLocators(
    const std::vector<std::string> &remoteLocators) {
  // Example: --J='-Dgemfire.remote-locators=localhost[9009],localhost[9010]'
  if (!remoteLocators.empty()) {
    std::ostringstream command;
    command << " --J='-Dgemfire.remote-locators=";
    std::copy(remoteLocators.begin(), remoteLocators.end(),
              std::ostream_iterator<std::string>(command, ","));
    command << "'";
    command_ += command.str();
  }
  return *this;
}

Gfsh::Start::Locator &Gfsh::Start::Locator::withDistributedSystemId(
    const uint16_t &dsId) {
  if (dsId != 0) {
    command_ += " --J=-Dgemfire.distributed-system-id=" + std::to_string(dsId);
  }
  return *this;
}

Gfsh::Start::Locator &Gfsh::Start::Locator::withJmxManagerPort(
    const uint16_t &jmxManagerPort) {
  command_ +=
      " --J=-Dgemfire.jmx-manager-port=" + std::to_string(jmxManagerPort);
  return *this;
}

Gfsh::Start::Locator &Gfsh::Start::Locator::withHttpServicePort(
    const uint16_t &httpServicePort) {
  command_ += " --http-service-port=" + std::to_string(httpServicePort);
  return *this;
}

Gfsh::Start::Locator &Gfsh::Start::Locator::withLogLevel(
    const std::string &logLevel) {
  command_ += " --log-level=" + logLevel;
  return *this;
}

Gfsh::Start::Locator &Gfsh::Start::Locator::withMaxHeap(
    const std::string &maxHeap) {
  command_ += " --max-heap=" + maxHeap;
  return *this;
}

Gfsh::Start::Locator &Gfsh::Start::Locator::withClasspath(
    const std::string classpath) {
  if (!classpath.empty()) {
    command_ += " --classpath=" + classpath;
  }
  return *this;
}

Gfsh::Start::Locator &Gfsh::Start::Locator::withSecurityManager(
    const std::string securityManager) {
  if (!securityManager.empty()) {
    command_ += " --J=-Dgemfire.security-manager=" + securityManager;
  }
  return *this;
}

Gfsh::Start::Locator &Gfsh::Start::Locator::withPreferIPv6(bool useIPv6) {
  if (useIPv6) {
    command_ += " --J=-Djava.net.preferIPv6Addresses=true";
  }
  return *this;
}

Gfsh::Start::Locator &Gfsh::Start::Locator::withSslEnabledComponents(
    const std::string &components) {
  command_ += " --J=-Dgemfire.ssl-enabled-components=" + components;
  return *this;
}

Gfsh::Start::Locator &Gfsh::Start::Locator::withSslKeystore(
    const std::string &keystore) {
  command_ += " --J=-Dgemfire.ssl-keystore=" + keystore;
  return *this;
}

Gfsh::Start::Locator &Gfsh::Start::Locator::withSslTruststore(
    const std::string &truststore) {
  command_ += " --J=-Dgemfire.ssl-truststore=" + truststore;
  return *this;
}

Gfsh::Start::Locator &Gfsh::Start::Locator::withSslKeystorePassword(
    const std::string &keystorePassword) {
  command_ += " --J=-Dgemfire.ssl-keystore-password=" + keystorePassword;
  return *this;
}

Gfsh::Start::Locator &Gfsh::Start::Locator::withSslTruststorePassword(
    const std::string &truststorePassword) {
  command_ += " --J=-Dgemfire.ssl-truststore-password=" + truststorePassword;
  return *this;
}

Gfsh::Start::Locator &Gfsh::Start::Locator::withConnect(const bool connect) {
  command_ += " --connect=" + std::string(connect ? "true" : "false");
  return *this;
}

Gfsh::Start::Locator &Gfsh::Start::Locator::withJmxManagerStart(
    const bool startJmxManager) {
  command_ += " --J=-Dgemfire.jmx-manager-start=" +
              std::string(startJmxManager ? "true" : "false");
  return *this;
}

Gfsh::Start::Locator &Gfsh::Start::Locator::withSslRequireAuthentication(
    const bool require) {
  command_ += " --J=-Dgemfire.ssl-require-authentication=" +
              std::string(require ? "true" : "false");
  return *this;
}

Gfsh::Start::Locator &Gfsh::Start::Locator::withPropertiesFile(
    const std::string file) {
  command_ += " --properties-file=" + file;
  return *this;
}

Gfsh::Start::Locator &Gfsh::Start::Locator::withSecurityPropertiesFile(
    const std::string file) {
  command_ += " --security-properties-file=" + file;
  return *this;
}

Gfsh::Start::Locator &Gfsh::Start::Locator::withHostNameForClients(
    const std::string hostName) {
  command_ += " --hostname-for-clients=" + hostName;
  return *this;
}

Gfsh::Start::Server::Server(Gfsh &gfsh) : Command(gfsh, "start server") {}

Gfsh::Start::Server &Gfsh::Start::Server::withName(const std::string &name) {
  command_ += " --name=" + name;
  return *this;
}

Gfsh::Start::Server &Gfsh::Start::Server::withDir(const std::string &dir) {
  command_ += " --dir=" + dir;
  return *this;
}

Gfsh::Start::Server &Gfsh::Start::Server::withBindAddress(
    const std::string &bindAddress) {
  command_ += " --bind-address=" + bindAddress;
  return *this;
}

Gfsh::Start::Server &Gfsh::Start::Server::withPort(const uint16_t &serverPort) {
  command_ += " --server-port=" + std::to_string(serverPort);
  return *this;
}

Gfsh::Start::Server &Gfsh::Start::Server::withLocators(
    const std::string &locators) {
  command_ += " --locators=" + locators;
  return *this;
}

Gfsh::Start::Server &Gfsh::Start::Server::withLogLevel(
    const std::string &logLevel) {
  command_ += " --log-level=" + logLevel;
  return *this;
}

Gfsh::Start::Server &Gfsh::Start::Server::withMaxHeap(
    const std::string &maxHeap) {
  command_ += " --max-heap=" + maxHeap;
  return *this;
}

Gfsh::Start::Server &Gfsh::Start::Server::withClasspath(
    const std::string classpath) {
  if (!classpath.empty()) {
    command_ += " --classpath=" + classpath;
  }
  return *this;
}

Gfsh::Start::Server &Gfsh::Start::Server::withSecurityManager(
    const std::string securityManager) {
  if (!securityManager.empty()) {
    command_ += " --J=-Dgemfire.security-manager=" + securityManager;
  }
  return *this;
}

Gfsh::Start::Server &Gfsh::Start::Server::withUser(const std::string user) {
  if (!user.empty()) {
    command_ += " --user=" + user;
  }

  return *this;
}

Gfsh::Start::Server &Gfsh::Start::Server::withPassword(
    const std::string password) {
  if (!password.empty()) {
    command_ += " --password=" + password;
  }
  return *this;
}

Gfsh::Start::Server &Gfsh::Start::Server::withCacheXMLFile(
    const std::string file) {
  if (!file.empty()) {
    command_ += " --cache-xml-file=" + file;
  }
  return *this;
}

Gfsh::Start::Server &Gfsh::Start::Server::withPreferIPv6(bool useIPv6) {
  if (useIPv6) {
    command_ += " --J=-Djava.net.preferIPv6Addresses=true";
  }
  return *this;
}

Gfsh::Start::Server &Gfsh::Start::Server::withSslEnabledComponents(
    const std::string &components) {
  command_ += " --J=-Dgemfire.ssl-enabled-components=" + components;
  return *this;
}

Gfsh::Start::Server &Gfsh::Start::Server::withSslKeystore(
    const std::string &keystore) {
  command_ += " --J=-Dgemfire.ssl-keystore=" + keystore;
  return *this;
}

Gfsh::Start::Server &Gfsh::Start::Server::withSslTruststore(
    const std::string &truststore) {
  command_ += " --J=-Dgemfire.ssl-truststore=" + truststore;
  return *this;
}

Gfsh::Start::Server &Gfsh::Start::Server::withSslKeystorePassword(
    const std::string &keystorePassword) {
  command_ += " --J=-Dgemfire.ssl-keystore-password=" + keystorePassword;
  return *this;
}

Gfsh::Start::Server &Gfsh::Start::Server::withSslTruststorePassword(
    const std::string &truststorePassword) {
  command_ += " --J=-Dgemfire.ssl-truststore-password=" + truststorePassword;
  return *this;
}

Gfsh::Start::Server &Gfsh::Start::Server::withSslRquireAuthentication(
    const bool require) {
  command_ += " --J=-Dgemfire.ssl-require-authentication=" +
              std::string(require ? "true" : "false");
  return *this;
}

Gfsh::Start::Server &Gfsh::Start::Server::withPropertiesFile(
    const std::string file) {
  command_ += " --properties-file=" + file;
  return *this;
}

Gfsh::Start::Server &Gfsh::Start::Server::withSecurityPropertiesFile(
    const std::string file) {
  command_ += " --security-properties-file=" + file;
  return *this;
}

Gfsh::Start::Server &Gfsh::Start::Server::withHostNameForClients(
    const std::string hostName) {
  command_ += " --hostname-for-clients=" + hostName;
  return *this;
}

Gfsh::Start::Server &Gfsh::Start::Server::withSystemProperty(
    const std::string &key, const std::string &value) {
  command_ += " --J=-D" + key + "=" + value;
  return *this;
}

Gfsh::Start::Server &Gfsh::Start::Server::withConserveSockets(
    bool conserveSockets) {
  if (conserveSockets) {
    command_ += " --J=-Dgemfire.conserve-sockets=true";
  }
  return *this;
}

Gfsh::Stop::Stop(Gfsh &gfsh) : gfsh_(gfsh) {}

Gfsh::Stop::Server Gfsh::Stop::server() { return Server{gfsh_}; }

Gfsh::Stop::Locator Gfsh::Stop::locator() { return Locator{gfsh_}; }

Gfsh::Stop::Locator::Locator(Gfsh &gfsh) : Command(gfsh, "stop locator") {}

Gfsh::Stop::Locator &Gfsh::Stop::Locator::withName(const std::string &name) {
  command_ += " --name=" + name;
  return *this;
}

Gfsh::Stop::Locator &Gfsh::Stop::Locator::withDir(const std::string &dir) {
  command_ += " --dir=" + dir;
  return *this;
}

Gfsh::Stop::Server::Server(Gfsh &gfsh) : Command(gfsh, "stop server") {}

Gfsh::Stop::Server &Gfsh::Stop::Server::withName(const std::string &name) {
  command_ += " --name=" + name;
  return *this;
}

Gfsh::Stop::Server &Gfsh::Stop::Server::withDir(const std::string &dir) {
  command_ += " --dir=" + dir;
  return *this;
}

Gfsh::Create::Create(Gfsh &gfsh) : Verb{gfsh} {}

Gfsh::Create::Region Gfsh::Create::region() { return Region{gfsh_}; }

Gfsh::Create::Region::Region(Gfsh &gfsh) : Command(gfsh, "create region") {}

Gfsh::Create::Region &Gfsh::Create::Region::withName(const std::string &name) {
  command_ += " --name=" + name;
  return *this;
}

Gfsh::Create::Region &Gfsh::Create::Region::withType(const std::string &type) {
  command_ += " --type=" + type;
  return *this;
}

Gfsh::Create::Region &Gfsh::Create::Region::withRedundantCopies(
    const std::string &copies) {
  command_ += " --redundant-copies=" + copies;
  return *this;
}

Gfsh::Create::Region &Gfsh::Create::Region::withBuckets(
    const std::string &totalNumBuckets) {
  command_ += " --total-num-buckets=" + totalNumBuckets;
  return *this;
}

Gfsh::Create::Region &Gfsh::Create::Region::withGatewaySenderId(
    const std::string &gatewaySenderId) {
  command_ += " --gateway-sender-id=" + gatewaySenderId;
  return *this;
}

Gfsh::Create::GatewaySender Gfsh::Create::gatewaySender() {
  return GatewaySender{gfsh_};
}

Gfsh::Create::Region &Gfsh::Create::Region::withPartitionResolver(
    const std::string &partitionResolver) {
  command_ += " --partition-resolver=" + partitionResolver;
  return *this;
}

Gfsh::Create::GatewaySender::GatewaySender(Gfsh &gfsh)
    : Command(gfsh, "create gateway-sender") {}

Gfsh::Create::GatewaySender &Gfsh::Create::GatewaySender::withId(
    const std::string &id) {
  command_ += " --id=" + id;
  return *this;
}

Gfsh::Create::GatewaySender &Gfsh::Create::GatewaySender::withRemoteDSId(
    const std::string &remoteDSId) {
  command_ += " --remote-distributed-system-id=" + remoteDSId;
  return *this;
}

Gfsh::Create::GatewayReceiver Gfsh::Create::gatewayReceiver() {
  return GatewayReceiver{gfsh_};
}

Gfsh::Create::GatewayReceiver::GatewayReceiver(Gfsh &gfsh)
    : Command(gfsh, "create gateway-receiver") {}

Gfsh::Destroy::Destroy(Gfsh &gfsh) : Verb{gfsh} {}

Gfsh::Destroy::Region Gfsh::Destroy::region() { return Region{gfsh_}; }

Gfsh::Destroy::Region::Region(Gfsh &gfsh) : Command(gfsh, "destroy region") {}

Gfsh::Destroy::Region &Gfsh::Destroy::Region::withName(
    const std::string &name) {
  command_ += " --name=" + name;
  return *this;
}

Gfsh::Destroy::Region &Gfsh::Destroy::Region::ifExists() {
  command_ += " --if-exists";
  return *this;
}

Gfsh::Connect::Connect(Gfsh &gfsh) : Command{gfsh, "connect"} {}

Gfsh::Connect &Gfsh::Connect::withJmxManager(const std::string &jmxManager) {
  command_ += " --jmx-manager=" + jmxManager;
  return *this;
}

Gfsh::Connect &Gfsh::Connect::withUser(const std::string &user) {
  if (!user.empty()) {
    command_ += " --user=" + user;
  }

  return *this;
}

Gfsh::Connect &Gfsh::Connect::withPassword(const std::string &password) {
  if (!password.empty()) {
    command_ += " --password=" + password;
  }

  return *this;
}

Gfsh::Connect &Gfsh::Connect::withUseSsl(const bool useSsl) {
  command_ += " --use-ssl=" + std::string(useSsl ? "true" : "false");
  return *this;
}

Gfsh::Connect &Gfsh::Connect::withKeystore(const std::string &keystore) {
  command_ += " --key-store=" + keystore;
  return *this;
}

Gfsh::Connect &Gfsh::Connect::withTruststore(const std::string &truststore) {
  command_ += " --trust-store=" + truststore;
  return *this;
}

Gfsh::Connect &Gfsh::Connect::withKeystorePassword(
    const std::string &keystorePassword) {
  command_ += " --key-store-password=" + keystorePassword;
  return *this;
}

Gfsh::Connect &Gfsh::Connect::withTruststorePassword(
    const std::string &truststorePassword) {
  command_ += " --trust-store-password=" + truststorePassword;
  return *this;
}

Gfsh::Shutdown::Shutdown(Gfsh &gfsh) : Command{gfsh, "shutdown"} {}

Gfsh::Shutdown &Gfsh::Shutdown::withIncludeLocators(bool includeLocators) {
  command_ +=
      " --include-locators=" + std::string(includeLocators ? "true" : "false");
  return *this;
}

Gfsh::Deploy::Deploy(Gfsh &gfsh) : Command{gfsh, "deploy"} {}

Gfsh::Deploy &Gfsh::Deploy::jar(const std::string &jarFile) {
  command_ += " --jars=" + jarFile;

  return *this;
}

template <>
void Gfsh::Command<void>::execute(const std::string &user,
                                  const std::string &password,
                                  const std::string &keyStorePath,
                                  const std::string &trustStorePath,
                                  const std::string &keyStorePassword,
                                  const std::string &trustStorePassword) {
  gfsh_.execute(command_, user, password, keyStorePath, trustStorePath,
                keyStorePassword, trustStorePassword);
}

template <>
void Gfsh::Command<void>::execute() {
  gfsh_.execute(command_, "", "", "", "", "", "");
}

Gfsh::ExecuteFunction::ExecuteFunction(Gfsh &gfsh)
    : Command{gfsh, "execute function"} {}

Gfsh::ExecuteFunction &Gfsh::ExecuteFunction::withId(
    const std::string &functionId) {
  command_ += " --id=" + functionId;

  return *this;
}

Gfsh::ExecuteFunction &Gfsh::ExecuteFunction::withMember(
    const std::string &memberName) {
  command_ += " --member=" + memberName;

  return *this;
}
