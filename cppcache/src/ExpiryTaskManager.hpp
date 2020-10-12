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

#ifndef GEODE_EXPIRYTASKMANAGER_H_
#define GEODE_EXPIRYTASKMANAGER_H_

#include <chrono>
#include <condition_variable>
#include <mutex>
#include <thread>

#include <boost/asio/executor_work_guard.hpp>
#include <boost/asio/io_context.hpp>

#include "ExpiryTask.hpp"

namespace apache {
namespace geode {
namespace client {

/**
 * @class ExpiryTaskManager ExpiryTaskManager.hpp
 *
 * This class manages all the ExpiryTaskManagers
 * It uses Boost.Asio and Boost timers to schedule ExpiryTasks
 */
class ExpiryTaskManager {
 public:
  /**
   * Default class constructor
   */
  ExpiryTaskManager();

  /**
   * Class destructor
   */
  virtual ~ExpiryTaskManager() noexcept;

  /**
   * Starts the manager
   * @throw IllegalStateException An exception is thrown if the manager is
   *                              already started.
   */
  void start();

  /**
   * Stops the manager
   * @throw IllegalStateException An exception is thrown if the manager is
   *                              already stopped.
   */
  void stop();

  /**
   * Returns whether or not the manager is running.
   */
  bool running() const { return running_; }

  /**
   * Returns the number of scheduled tasks.
   */
  std::size_t count() const { return task_map_.size(); }

  /**
   * Schedules a new expiry task.
   * @param task Reference to the task.
   * @param delay Amount of nano-seconds in which the task expires.
   * @param interval Amount of nano-seconds in which the task is executed again.
   *                 If this parameter is set to zero, the task will only
   *                 execute once and finish, otherwise it will execute
   *                 continuously every interval nano-seconds until cancelled.
   * @return ID of the created task. If there was any error during the
   *         task creation ExpiryTask::invalid() is returned as ID.
   * @note Possible errors are:<br>
   *        - Manager is not running
   */
  ExpiryTask::id_t schedule(std::shared_ptr<ExpiryTask> task,
                            const ExpiryTask::duration_t &delay,
                            const ExpiryTask::duration_t &interval);

  /**
   * Schedules a new expiry task.
   * @param task Task reference.
   * @param delay Amount of nano-seconds in which the task expires.
   * @return ID of the created task. If there was any error during the
   *         task creation ExpiryTask::invalid() is returned as ID.
   * @note Possible errors are:<br>
   *        - Manager is not running
   */
  ExpiryTask::id_t schedule(std::shared_ptr<ExpiryTask> task,
                            const ExpiryTask::duration_t &delay) {
    return schedule(std::move(task), delay, ExpiryTask::duration_t::zero());
  }

  /**
   * Re-triggers a task to run in the given delay.
   * @param task_id ID of the task to be re-triggered.
   * @param delay Amount of nano-seconds in which the task expires.
   * @return Returns -1 if the task did not existed, and otherwise it
   *         returns the number of pending executions. Take into account
   *         that if 0 is returned it means the task was being executed
   *         while reset was called.
   */
  int32_t reset(ExpiryTask::id_t task_id, const ExpiryTask::duration_t &delay);

  /**
   * Cancels an already scheduled expiry task
   * @param task_id ID of the task to be cancelled.
   * @return Returns -1 if the task did not existed, and otherwise it
   *         returns the number of pending executions. Take into account
   *         that if 0 is returned it means the task was being executed
   *         while cancel was called.
   */
  int32_t cancel(ExpiryTask::id_t task_id);

 protected:
  /// Internal types

  using duration_t = std::chrono::nanoseconds;
  using task_map_t = std::map<ExpiryTask::id_t, std::shared_ptr<ExpiryTask>>;

 protected:
  friend class ExpiryTask;

  /**
   * Cancels all scheduled tasks
   */
  void cancel_all();

  /**
   * Removes the reference to the task matching the given ID
   * @param task_id ID of the task
   */
  void remove(ExpiryTask::id_t task_id);

  /**
   * Returns Boost IO context
   */
  boost::asio::io_context &io_context() { return io_context_; }

 protected:
  /// Class member attributes

  /**
   * Flag indicating whether or not the manager is running.
   */
  bool running_;

  /**
   * Thread running the io_context.
   */
  std::thread runner_;

  /*
   * Boost IO context processing expiry tasks events.
   */
  boost::asio::io_context io_context_;

  /**
   * Executor guard. It keeps the IO context running even when there are
   * no task scheduled.
   */
  boost::asio::executor_work_guard<boost::asio::io_context::executor_type>
      work_guard_;

  /**
   * Manager mutex. It has the following uses:<br>
   *  - Avoids race conditions involving the task container.
   *  - Avoids creation of new task while the expiry task is being stopped.
   *  - Avoids concurrent modification of the task counter.
   */
  std::mutex mutex_;

  /**
   * Task container
   */
  task_map_t task_map_;

  /**
   * Task counter. It's used to assign tasks an UID.
   */
  ExpiryTask::id_t last_task_id_;
};
}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_EXPIRYTASKMANAGER_H_
