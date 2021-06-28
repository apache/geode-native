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
#include "fw_dunit.hpp"
#include "ThinClientHelper.hpp"

#include <atomic>

using apache::geode::client::Exception;

class GetRegionThread {
 public:
  GetRegionThread(const char *path, const char *subPath)
      : m_running(false),
        m_path(path),
        m_subPath(subPath),
        m_regionCreateDone(false),
        m_subRegionCreateDone(false) {}

  void run() {
    while (m_running) {
      SLEEP(40);
      try {
        auto rptr = getHelper()->getRegion(m_path.c_str());
        if (rptr != nullptr) {
          ASSERT(m_regionCreateDone, "regionCreate Not Done");
        }
      } catch (Exception &ex) {
        LOG(ex.what());
        continue;
      } catch (std::exception &ex) {
        LOG(ex.what());
        continue;
      } catch (...) {
        LOG("unknown exception");
        continue;
      }
      try {
        auto rptr = getHelper()->getRegion(m_subPath.c_str());
        if (rptr != nullptr) {
          ASSERT(m_subRegionCreateDone, "subRegionCreate Not Done");
          return;
        }
      } catch (Exception &ex) {
        LOG(ex.what());
      } catch (std::exception &ex) {
        LOG(ex.what());
      } catch (...) {
        LOG("getRegion: unknown exception");
      }
    }
  }

  void setRegionFlag() { m_regionCreateDone = true; }

  void setSubRegionFlag() { m_subRegionCreateDone = true; }

  void start() {
    m_running = true;
    thread_ = std::thread{[this]() { run(); }};
  }

  void stop() {
    m_running = false;
    if (thread_.joinable()) {
      thread_.join();
    }
  }

 protected:
  bool m_running;
  std::string m_path;
  std::thread thread_;
  std::string m_subPath;
  std::atomic<bool> m_regionCreateDone;
  std::atomic<bool> m_subRegionCreateDone;
};

static int numberOfLocators = 1;
bool isLocalServer = true;
bool isLocator = true;
const std::string locHostPort =
    CacheHelper::getLocatorHostPort(isLocator, isLocalServer, numberOfLocators);
GetRegionThread *getThread = nullptr;
std::shared_ptr<Region> regionPtr;
DUNIT_TASK(s1p1, Setup)
  {
    CacheHelper::initLocator(1);
    CacheHelper::initServer(1, "cacheserver_notify_subscription.xml",
                            locHostPort);
    LOG("SERVER started");
  }
ENDTASK

/* setup a normal region */
DUNIT_TASK(s2p2, CreateNormalRegion)
  {
    initClientWithPool(true, "__TEST_POOL1__", locHostPort, "ServerGroup1",
                       nullptr, 0, true);
    LOG("create normal region");
    getThread =
        new GetRegionThread("DistRegionAck", "DistRegionAck/AuthSubregion");
    getThread->start();
    regionPtr = getHelper()->createPooledRegion(
        "DistRegionAck", USE_ACK, locHostPort, "__TEST_POOL1__", true, true);
    getThread->setRegionFlag();
    RegionAttributesFactory regionAttributesFactory;
    auto regionAttributes = regionAttributesFactory.create();
    getThread->setSubRegionFlag();
    LOG("create normal region successful");
  }
END_TASK(CreateNormalRegion)

DUNIT_TASK(s2p2, CloseCache2)
  {
    getThread->stop();
    delete getThread;
    cleanProc();
  }
END_TASK(CloseCache2)

DUNIT_TASK(s1p1, CloseCache)
  {
    CacheHelper::closeServer(1);
    LOG("SERVER closed");
  }
ENDTASK
