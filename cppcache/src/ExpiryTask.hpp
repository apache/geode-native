
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

#ifndef GEODE_EXPIRYTASK_H_
#define GEODE_EXPIRYTASK_H_

#include <chrono>
#include <functional>
#include <limits>
#include <memory>
#include <mutex>

#include <boost/asio/io_context.hpp>
#include <boost/asio/steady_timer.hpp>

namespace apache {
namespace geode {
namespace client {

class ExpiryTaskManager;

/**
 * @class ExpiryTask ExpiryTask.hpp
 *
 * This is the base class for all expiry tasks.
 */
class ExpiryTask : public std::enable_shared_from_this<ExpiryTask> {
 public:
  using id_t = uint64_t;

 public:
  /// Class constructors

  /**
   * Class constructor
   * @param manager A reference to the ExpiryTaskManager
   */
  explicit ExpiryTask(ExpiryTaskManager& manager);

  /**
   * Class destructor
   */
  virtual ~ExpiryTask() = default;

  /**
   * Returns the task ID
   */
  id_t id() const { return id_; }

  /**
   * Returns whether the task is periodic
   */
  bool periodic() const {
    return interval_ != std::chrono::nanoseconds::zero();
  }

  /**
   * Returns an ID which represents an invalid task
   */
  static constexpr id_t invalid() { return (std::numeric_limits<id_t>::max)(); }

 protected:
  using timer_t = boost::asio::steady_timer;
  using clock_t = timer_t::clock_type;
  using time_point_t = timer_t::time_point;
  using duration_t = time_point_t::duration;

 protected:
  friend class ExpiryTaskManager;

  /**
   * Callback called upon task expiration
   * @return Returns true if the task can normally complete and false if it has
   *         been reset. Specifically if false is returned it will mean:<br>
   *          - For periodic tasks. They won't re-trigger, as the task was
   *            re-scheduled.
   *          - For non-periodic tasks. They won't be removed from the manager,
   *            as the task was re-scheduled.
   * @note Note that as this is an abstract method, each implementer of
   * ExpiryTask should write its own version
   */
  virtual bool on_expire() = 0;

  /**
   * Sets task ID
   * @param id Task ID
   * @return A reference to itself
   */
  ExpiryTask& id(id_t id) {
    id_ = id;
    return *this;
  }

  /**
   * Sets the task execution interval
   * @param interval Task execution interval
   * @return A reference to itself
   */
  ExpiryTask& interval(const duration_t& interval) {
    interval_ = interval;
    return *this;
  }

  /**
   * Cancels the task
   * @return Returns the number of pending executions. Take into account
   *         that if 0 is returned it means the task was being executed
   *         while cancel was called.
   */
  int32_t cancel();

  /**
   * Resets the task timer
   * @param at Time point at which the task is to be re-triggered.
   * @return Returns -1 if the task was cancelled, and otherwise it
   *         returns the number of pending executions. Take into account
   *         that if 0 is returned it means the task was being executed
   *         while reset was called.
   */
  int32_t reset(const time_point_t& at);

  /**
   * Resets the task timer
   * @param delay Amount of nano-seconds until the task is triggered.
   * @return Returns -1 if the task was cancelled, and otherwise it
   *         returns the number of pending executions. Take into account
   *         that if 0 is returned it means the task was being executed
   *         while reset was called.
   */
  int32_t reset(const duration_t& delay);

  /**
   * Function triggered by the timer implementation.
   * @param err Boost error passed by the timer.
   */
  void on_callback(const boost::system::error_code& err);

 protected:
  /// Member attributes

  /**
   * Unique identifier of the task
   */
  id_t id_;

  /**
   * Specific timer implementation instance
   */
  timer_t timer_;

  /**
   * Reference to the expiry manager
   */
  ExpiryTaskManager& manager_;

  /**
   * Re-triggering interval
   * @note This is used in order to define periodic tasks
   */
  duration_t interval_;

  /**
   * Exclusive mutex used to avoid race conditions for the following member
   * functions:<br>
   *  - cancel
   *  - reset
   */
  std::mutex mutex_;

  /**
   * Flag indicating whether or not the task has been cancelled
   * @note This is necessary in order to guarantee that whenever the task is
   *       reset the task won't be removed from the ExpiryTaskManager.
   *       The reason why is because reset cancels the execution of all the
   *       functions to be called by the timer and given they all share the
   *       same ExpiryTask instance there was no way to tell whether the
   *       task was cancelled or reset.
   */
  bool cancelled_{false};
};

}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_EXPIRYTASKMANAGER_H_
