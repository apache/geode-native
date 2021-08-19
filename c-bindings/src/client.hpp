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

#include <map>
#include <string>
#include <memory>

#include "geode/internal/geode_base.h"

// All wrappers are passed their creator object for retention.
// The parent objects implement this interface.
class Client {
  virtual void do_AddRecord(void *, const std::string &) = 0;
  virtual void do_RemoveRecord(void *) = 0;

 public:
  // All wrappers call this in their constructors.
  void AddRecord(void *, const std::string &);
  // All wrappers call this in their destructors.
  void RemoveRecord(void *);
};

// Anything that creates another wrapper derives from this.
// Adding and removing from this defers to the parent retained within.
class ClientKeeper : public Client {
  void do_AddRecord(void *, const std::string &) final override;
  void do_RemoveRecord(void *) final override;
};

// This is the top level parent. Adding and removing from any client will
// ultimately add and remove from an instance of this. This is the client
// object the user creates from the C interface. Because the wrappers form a
// hierarchy, it's very important to destroy them in reverse order along that
// hierarchy.
class ClientWrapper : public Client {
  struct ClientObjectRecord {
    std::string className;
    std::string allocationCallstack;
  };

  typedef std::map<void *, ClientObjectRecord> ClientObjectRegistry;

  ClientObjectRegistry registry_;

  void do_AddRecord(void *, const std::string &) final override;
  void do_RemoveRecord(void *) final override;

 public:
  int checkForLeaks();

  virtual ~ClientWrapper();
};

class PermaClient {
 public:
    static std::shared_ptr<ClientWrapper> &instance();
};