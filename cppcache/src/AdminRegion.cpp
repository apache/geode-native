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

#include "AdminRegion.hpp"

#include <geode/SystemProperties.hpp>

#include "CacheImpl.hpp"
#include "TcrConnectionManager.hpp"
#include "ThinClientPoolDM.hpp"
#include "ThinClientRegion.hpp"
#include "util/exception.hpp"

namespace apache {
namespace geode {
namespace client {

std::shared_ptr<AdminRegion> AdminRegion::create(CacheImpl* cache,
                                                 ThinClientBaseDM* distMan) {
  auto adminRegion = std::make_shared<AdminRegion>();

  auto& props = cache->getDistributedSystem().getSystemProperties();
  if (props.statisticsEnabled()) {
    // no need to create a region .. just create a cacheDistribution Manager
    adminRegion->connection_mgr_ = &(cache->tcrConnectionManager());
    if (!distMan) {
      adminRegion->dist_mgr_ =
          new ThinClientCacheDistributionManager(*adminRegion->connection_mgr_);
      cache->getStatisticsManager().RegisterAdminRegion(adminRegion);
    } else {
      adminRegion->dist_mgr_ = distMan;
    }
  }

  return adminRegion;
}

void AdminRegion::init() {
  // Init distribution manager if it is not a pool
  if (dist_mgr_ && !dynamic_cast<ThinClientPoolDM*>(dist_mgr_)) {
    dist_mgr_->init();
  }
}

TcrConnectionManager* AdminRegion::getConnectionManager() {
  return connection_mgr_;
}

void AdminRegion::put(const std::shared_ptr<CacheableKey>& keyPtr,
                      const std::shared_ptr<Cacheable>& valuePtr) {
  GfErrType err = putNoThrow(keyPtr, valuePtr);
  throwExceptionIfError("AdminRegion::put", err);
}

GfErrType AdminRegion::putNoThrow(const std::shared_ptr<CacheableKey>& keyPtr,
                                  const std::shared_ptr<Cacheable>& valuePtr) {
  // put obj to region
  GfErrType err = GF_NOERR;

  TcrMessagePut request(
      new DataOutput(connection_mgr_->getCacheImpl()->createDataOutput()),
      nullptr, keyPtr, valuePtr, nullptr, false, dist_mgr_, true, false,
      full_path_.c_str());
  request.setMetaRegion(true);
  TcrMessageReply reply(true, dist_mgr_);
  reply.setMetaRegion(true);
  err = dist_mgr_->sendSyncRequest(request, reply, true, true);
  if (err != GF_NOERR) {
    return err;
  }

  // put the object into local region
  switch (reply.getMessageType()) {
    case TcrMessage::REPLY: {
      LOGDEBUG(
          "AdminRegion::put: entry is written into remote server "
          "at region %s",
          full_path_.c_str());
      break;
    }
    case TcrMessage::EXCEPTION: {
      const auto& exceptionMsg = reply.getException();
      err = ThinClientRegion::handleServerException("AdminRegion::put",
                                                    exceptionMsg);
      break;
    }
    case TcrMessage::PUT_DATA_ERROR: {
      LOGERROR("A write error occurred on the endpoint %s",
               dist_mgr_->getActiveEndpoint()->name().c_str());
      err = GF_CACHESERVER_EXCEPTION;
      break;
    }
    default: {
      LOGERROR("Unknown message type in put reply %d", reply.getMessageType());
      err = GF_MSG;
    }
  }
  return err;
}

void AdminRegion::close() {
  boost::unique_lock<decltype(mutex_)> guard{mutex_};

  if (destroy_pending_) {
    return;
  }

  destroy_pending_ = true;

  // Close distribution manager if it is not a pool
  ThinClientPoolDM* pool = dynamic_cast<ThinClientPoolDM*>(dist_mgr_);
  if (pool == nullptr) {
    dist_mgr_->destroy();
    _GEODE_SAFE_DELETE(dist_mgr_);
  }
}

AdminRegion::~AdminRegion() {
  // destructor should be single threaded in any case, so no need of guard
  if (dist_mgr_ != nullptr) {
    close();
  }
}

const bool& AdminRegion::isDestroyed() { return destroy_pending_; }

boost::shared_lock<boost::shared_mutex> AdminRegion::make_shared_lock() {
  mutex_.lock_shared();
  return {mutex_, boost::adopt_lock};
}

}  // namespace client
}  // namespace geode
}  // namespace apache
