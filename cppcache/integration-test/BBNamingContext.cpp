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
#ifndef _WIN32

#include "BBNamingContext.hpp"

#include <ace/ACE.h>
#include "fwklib/FwkBBServer.hpp"
#include "fwklib/FwkBBClient.hpp"
#include "fwklib/FwkStrCvt.hpp"
#include <fwklib/FwkException.hpp>

#define ERR_MAX 10

namespace apache {
namespace geode {
namespace client {
namespace testframework {

static int hashcode(char *str) {
  if (str == nullptr) {
    return 0;
  }
  int localHash = 0;

  int prime = 31;
  char *data = str;
  for (int i = 0; i < 50 && (data[i] != '\0'); i++) {
    localHash = prime * localHash + data[i];
  }
  if (localHash > 0) return localHash;
  return -1 * localHash;
}

static int getRandomNum() {
  char *testName = std::getenv("TESTNAME");

  int seed = hashcode(testName) + 11;

  printf("seed for BBPort process %d\n", seed);
  // The integration tests rely on the pseudo-random
  // number generator being seeded with a very particular
  // value specific to the test by way of the test name.
  // Whilst this approach is pessimal, it can not be
  // remedied as the test depend upon it.
  std::srand(seed);
  // NOLINTNEXTLINE(clang-analyzer-security.insecureAPI.rand)
  return (std::rand() % 49999) + 14000;
}

static int G_BBPORT = getRandomNum();

class BBNamingContextClientImpl {
  FwkBBClient *m_bbc;
  uint32_t m_errCount;
  bool checkValue(const std::string &k, const std::string &k1,
                  const std::string &value);

 public:
  BBNamingContextClientImpl();
  ~BBNamingContextClientImpl();
  void open();
  void close();
  void dump();
  int rebind(const char *key, const char *value, char *type);
  int resolve(const std::string &key, std::string &value, char *);
};

class BBNamingContextServerImpl {
  FwkBBServer *m_bbServer;
  UDPMessageQueues *m_shared;
  STReceiver *m_recv;
  BBProcessor *m_serv;
  Responder *m_resp;
  Service *m_farm;

 public:
  BBNamingContextServerImpl();
  ~BBNamingContextServerImpl();
};
//
// Impls:
//
BBNamingContextClientImpl::BBNamingContextClientImpl()
    : m_bbc(nullptr), m_errCount(0) {}
BBNamingContextClientImpl::~BBNamingContextClientImpl() { close(); }
void BBNamingContextClientImpl::open() {
  try {
    // char * bbPort = std::getenv( "BB_PORT" );
    std::string endpoint = "localhost:" + std::to_string(G_BBPORT);
    fprintf(stdout, "Blackboard client is talking on %s\n", endpoint.c_str());
    fflush(stdout);
    m_bbc = new FwkBBClient(endpoint);
  } catch (FwkException &e) {
    FWKEXCEPTION("create bb client encounted Exception: " << e.what());
  } catch (...) {
    FWKEXCEPTION("create bb client unknow exception\n");
  }
}
void BBNamingContextClientImpl::close() {
  if (m_bbc != nullptr) {
    delete m_bbc;
    m_bbc = nullptr;
  }
}
int BBNamingContextClientImpl::rebind(const char *key, const char *value,
                                      char *) {
  // fprintf(stdout, "bind: key=%s, value=%s\n", key, value);
  if (m_bbc == nullptr) {
    return -1;
  }
  if (m_errCount > ERR_MAX) {
    close();
    return -1;
  }

  try {
    std::string k(key);
    std::string k1("1");
    std::string v(value);
    m_bbc->set(k, k1, v);
    if (false == checkValue(k, k1, value)) {
      m_errCount++;
    } else {
      if (m_errCount > 0) {
        m_errCount = 0;
      }
      return 0;
    }
  } catch (FwkException &e) {
    m_errCount++;
    FWKEXCEPTION(" rebind encounted Exception: " << e.what());
  } catch (...) {
    m_errCount++;
    FWKEXCEPTION("rebind unknown exception\n");
  }
  return -1;
}
void BBNamingContextClientImpl::dump() {
  if (m_bbc == nullptr) {
    return;
  }
  if (m_errCount > ERR_MAX) {
    close();
    return;
  }
  try {
    std::string bb = m_bbc->dump();
    FWKINFO("Dump Blackboard " << bb);
    if (m_errCount > 0) {
      m_errCount = 0;
    }
  } catch (FwkException &e) {
    m_errCount++;
    FWKEXCEPTION("create dump encounted Exception: " << e.what());
  } catch (...) {
    m_errCount++;
    FWKEXCEPTION("dump unknown exception\n");
  }
}
int BBNamingContextClientImpl::resolve(const std::string &key,
                                       std::string &value, char *) {
  if (m_bbc == nullptr) {
    return -1;
  }
  if (m_errCount > ERR_MAX) {
    close();
    return -1;
  }
  try {
    value = m_bbc->getString(key, "1");
    if (m_errCount > 0) {
      m_errCount = 0;
    }
    return value.length() == 0 ? -1 : 0;
  } catch (FwkException &e) {
    m_errCount++;
    FWKEXCEPTION("create resolve encounted Exception: " << e.what());
  } catch (...) {
    m_errCount++;
    FWKEXCEPTION("resolve unknown exception\n");
  }
}

bool BBNamingContextClientImpl::checkValue(const std::string &k,
                                           const std::string &k1,
                                           const std::string &value) {
  bool valid = false;
  try {
    std::string v = m_bbc->getString(k, k1);
    if (value == v) valid = true;
  } catch (FwkException &e) {
    FWKEXCEPTION("create resolve encounted Exception: " << e.what());
  } catch (...) {
    FWKEXCEPTION("resolve unknown exception\n");
  }
  return valid;
}

BBNamingContextServerImpl::BBNamingContextServerImpl() {
  try {
    // char * bbPort = std::getenv( "BB_PORT" );

    std::string port = std::to_string(G_BBPORT);
    FwkStrCvt bPort(port);
    uint32_t prt = bPort.toUInt32();
    fprintf(stdout, "Blackboard server is on port:%u\n", prt);
    fflush(stdout);
    m_bbServer = new FwkBBServer();
    m_shared = new UDPMessageQueues("BBQueues");
    m_recv = new STReceiver(m_shared, prt);
    m_serv = new BBProcessor(m_shared, m_bbServer);
    m_resp = new Responder(m_shared, prt);
    m_farm = new Service(3);
    m_farm->runThreaded(m_recv, 1);
    m_farm->runThreaded(m_serv, 1);
    m_farm->runThreaded(m_resp, 1);
  } catch (FwkException &e) {
    FWKEXCEPTION("create bb server encounted Exception: " << e.what());
  } catch (...) {
    FWKSEVERE("create bb server unknown exception\n");
  }
}

BBNamingContextServerImpl::~BBNamingContextServerImpl() {
  delete m_farm;
  delete m_bbServer;
  delete m_shared;
  delete m_recv;
  delete m_serv;
  delete m_resp;
}
//
// client
//
BBNamingContextClient::BBNamingContextClient() {
  m_impl = new BBNamingContextClientImpl();
}
BBNamingContextClient::~BBNamingContextClient() {
  if (m_impl) {
    delete m_impl;
    m_impl = nullptr;
  }
}
void BBNamingContextClient::open() { m_impl->open(); }
void BBNamingContextClient::close() { m_impl->close(); }
void BBNamingContextClient::dump() { m_impl->dump(); }
int BBNamingContextClient::rebind(const char *key, const char *value,
                                  char *type) {
  return m_impl->rebind(key, value, type);
}
int BBNamingContextClient::resolve(const std::string &key, std::string &value,
                                   char *type) {
  return m_impl->resolve(key, value, type);
}
//
// server
//
BBNamingContextServer::BBNamingContextServer() {
  m_impl = new BBNamingContextServerImpl();
}
BBNamingContextServer::~BBNamingContextServer() {
  // NOLINTNEXTLINE(clang-analyzer-unix.Malloc): ACE
  if (m_impl != nullptr) {
    delete m_impl;
    m_impl = nullptr;
  }
}

}  // namespace testframework
}  // namespace client
}  // namespace geode
}  // namespace apache

#endif
