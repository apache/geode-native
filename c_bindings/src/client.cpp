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

// Standard headers
#include <string>
#include <utility>

// C++ client public headers
#include "geode/Exception.hpp"

// C++ client private headers
#include "util/Log.hpp"

// C client public headers
#include "geode/client.h"

// C client private headers
#include "client.hpp"

apache_geode_client_t* apache_geode_ClientInitialize() {
  return reinterpret_cast<apache_geode_client_t*>(new ClientWrapper());
}

int apache_geode_ClientUninitialize(apache_geode_client_t* client) {
  if(auto wrapper = reinterpret_cast<ClientWrapper*>(client)) {
    const int result = wrapper->checkForLeaks();

    delete wrapper;

    return result;
  }

  return 0;
}
  
void Client::AddRecord(void *value, const std::string &className) {
  do_AddRecord(value, className);
}

void Client::RemoveRecord(void *value) {
  do_RemoveRecord(value);
}

void ClientKeeper::do_AddRecord(void *value, const std::string &className) {
  client->AddRecord(value, className);
}

void ClientKeeper::do_RemoveRecord(void *value) {
  client->RemoveRecord(value);
}

ClientKeeper::ClientKeeper(Client *client):client{client} {}

int ClientWrapper::checkForLeaks() {
  int result = 0;
  if (!registry_.empty()) {
    for (auto recordPair : registry_) {
      auto object = recordPair.first;
      auto record = recordPair.second;

      LOGERROR("Leaked object of type \"%s\" (pointer value %p), callstack %s",
               record.className.c_str(), object,
               record.allocationCallstack.c_str());
      result = -1;
    }
  }
  return result;
}

void ClientWrapper::do_AddRecord(void* value, const std::string& className) {
  using apache::geode::client::Exception;

  registry_.insert({value, {className, Exception("").getStackTrace()}});
}

void ClientWrapper::do_RemoveRecord(void* value) { registry_.erase(value); }
