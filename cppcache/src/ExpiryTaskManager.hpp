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
#include <memory>
#include <mutex>
#include <type_traits>

#include <ace/Reactor.h>
#include <ace/Task.h>
#include <ace/Timer_Heap.h>

#include <geode/internal/chrono/duration.hpp>
#include <geode/internal/geode_globals.hpp>

#include "ReadWriteLock.hpp"
#include "util/Log.hpp"

namespace apache {
namespace geode {
namespace client {

/**
 * @class ExpiryTaskManager ExpiryTaskManager.hpp
 *
 * This class starts a reactor's event loop for taking care of expiry
 * tasks. The scheduling of event also happens through this manager.
 */
class ExpiryTaskManager : public ACE_Task_Base {
 public:
  typedef decltype(std::declval<ACE_Reactor>().schedule_timer(
      nullptr, nullptr, std::declval<ACE_Time_Value>())) id_type;

  /**
   * This class allows resetting of the timer to take immediate effect when
   * done from inside ACE_Event_Handler::handle_timeout(). With the default
   * implementation in ACE_Timer_Heap, the reset takes affect after the next
   * timeout so expiry does not work correctly with it where we need to
   * reset timer everytime an entry/region is accessed/modified. It also
   * avoids alternative methods like rescheduling eveytime which is
   * expensive in terms of object (ASCE_Timer_Node_T) creation,deletion.
   *
   * Here is the sequence as in ACE_Timer_Heap:
   *
   * 1) expire() is invoked by reactor to remove all expired entries
   *
   * 2) expire() invokes dispatch_info_i() that checks the first element
   *    in the heap and removes it, if required.
   *
   * 3) If the element is a recurring timer then the elemant is reinserted
   *    into the heap bu dispatch_info_i()
   *
   * 4) expire() then invokes ACE_Event_Handler::handle_timeout() on the
   *    element returned (this->upcall calls handle_timeout) by
   *    dispatch_info_t(), if any
   *
   * So when reset_timer_interval() is called from inside handle_timeout()
   * the it does not take effect immediately. This implementation reverses
   * the order of steps 3 and 4, so that resetting timer interval from
   * within handle_timeout() works as expected.
   *
   *
   */
  template <typename TYPE, typename FUNCTOR, typename ACE_LOCK>
  class GF_Timer_Heap_ImmediateReset_T
      : public ACE_Timer_Heap_T<TYPE, FUNCTOR, ACE_LOCK> {
   public:
    int expire_single(ACE_Command_Base& pre_dispatch_command) override {
      ACE_TRACE("GF_Timer_Heap_ImmediateReset_T::expire_single");
      ACE_Timer_Node_Dispatch_Info_T<TYPE> info;
      ACE_Time_Value cur_time;
      ACE_Timer_Node_T<TYPE>* expired = nullptr;

      // Create a scope for the lock ...
      {
        ACE_MT(ACE_GUARD_RETURN(ACE_LOCK, ace_mon, this->mutex_, -1))

        if (this->is_empty()) return 0;

        // Get the current time
        cur_time = this->gettimeofday_static() + this->timer_skew();

        expired = this->getFirstNode(cur_time, info);

        if (expired == nullptr) return 0;
      }

      const void* upcall_act = nullptr;

      // Preinvoke (handles refcount if needed, etc.)
      this->preinvoke(info, cur_time, upcall_act);

      // Release the token before expiration upcall.
      pre_dispatch_command.execute();

      // call the functor
      this->upcall(info, cur_time);

      // Postinvoke (undo refcount if needed, etc.)
      this->postinvoke(info, cur_time, upcall_act);

      // Create a scope for the lock ...
      {
        ACE_MT(ACE_GUARD_RETURN(ACE_LOCK, ace_mon, this->mutex_, -1))

        // Reschedule after doing the upcall in expire method
        // to let updated expiry interval, if any, take effect correctly
        expired = this->remove_first();

        // Check if this is an interval timer.
        if (expired->get_interval() > ACE_Time_Value::zero) {
          // Make sure that we skip past values that have already
          // "expired".
          this->recompute_next_abs_interval_time(expired, cur_time);
          // Since this is an interval timer, we need to reschedule
          // it.
          this->reschedule(expired);
        } else {
          // Delete the underlying object
          delete expired->get_type();
          // Call the factory method to free up the node.
          this->free_node(expired);
        }
      }

      // We have dispatched a timer
      return 1;
    }

    int expire() override {
      return ACE_Timer_Queue_T<TYPE, FUNCTOR, ACE_LOCK>::expire();
    }

    int expire(const ACE_Time_Value& cur_time) override {
      ACE_TRACE("GF_Timer_Heap_ImmediateReset_T::expire");
      ACE_MT(ACE_GUARD_RETURN(ACE_LOCK, ace_mon, this->mutex_, -1))

      // Keep looping while there are timers remaining and the earliest
      // timer is <= the <cur_time> passed in to the method.
      if (this->is_empty()) {
        return 0;
      }
      int number_of_timers_expired = 0;
      ACE_Timer_Node_T<TYPE>* expired = nullptr;
      ACE_Timer_Node_Dispatch_Info_T<TYPE> info;
      while ((expired = this->getFirstNode(cur_time, info)) != nullptr) {
        const void* upcall_act = nullptr;
        this->preinvoke(info, cur_time, upcall_act);

        this->upcall(info, cur_time);

        this->postinvoke(info, cur_time, upcall_act);

        //  reschedule after doing the upcall in expire method
        // to let updated expiry interval, if any, take affect correctly
        expired = this->remove_first();
        // Check if this is an interval timer.
        if (expired->get_interval() > ACE_Time_Value::zero) {
          // Make sure that we skip past values that have already
          // "expired".
          do {
            expired->set_timer_value(expired->get_timer_value() +
                                     expired->get_interval());
          } while (expired->get_timer_value() <= cur_time);
          // Since this is an interval timer, we need to reschedule
          // it.
          this->reschedule(expired);
        } else {
          // Delete the underlying object
          delete expired->get_type();
          // Call the factory method to free up the node.
          this->free_node(expired);
        }

        ++number_of_timers_expired;
      }
      return number_of_timers_expired;
    }

    ACE_Timer_Node_T<TYPE>* getFirstNode(
        const ACE_Time_Value& cur_time,
        ACE_Timer_Node_Dispatch_Info_T<TYPE>& info) {
      ACE_TRACE("GF_Timer_Heap_ImmediateReset_T::getFirstNode");

      if (this->is_empty()) {
        return nullptr;
      }
      ACE_Timer_Node_T<TYPE>* expired = nullptr;
      if (this->earliest_time() <= cur_time) {
        expired = this->get_first();
        // Get the dispatch info
        expired->get_dispatch_info(info);
        return expired;
      }
      return nullptr;
    }
  };

  typedef GF_Timer_Heap_ImmediateReset_T<
      ACE_Event_Handler*, ACE_Event_Handler_Handle_Timeout_Upcall,
      ACE_SYNCH_RECURSIVE_MUTEX>
      GF_Timer_Heap_ImmediateReset;

  /**
   * Constructor
   */
  ExpiryTaskManager();

  /**
   * Destructor. Stops the reactors event loop if it is not running
   * and then exits.
   */
  ~ExpiryTaskManager() override;

  /**
   * For scheduling a task for expiration.
   */
  template <class ExpRep, class ExpPeriod, class IntRep, class IntPeriod>
  ExpiryTaskManager::id_type scheduleExpiryTask(
      ACE_Event_Handler* handler,
      std::chrono::duration<ExpRep, ExpPeriod> expTime,
      std::chrono::duration<IntRep, IntPeriod> interval,
      bool cancelExistingTask = false) {
    using ::apache::geode::internal::chrono::duration::to_string;

    LOGFINER(
        "ExpiryTaskManager: expTime %s, interval %s, cancelExistingTask %d",
        to_string(expTime).c_str(), to_string(interval).c_str(),
        cancelExistingTask);
    if (cancelExistingTask) {
      m_reactor->cancel_timer(handler, 1);
    }

    ACE_Time_Value expTimeValue(expTime);
    ACE_Time_Value intervalValue(interval);
    LOGFINER("Scheduled expiration ... in " + to_string(expTime));
    return m_reactor->schedule_timer(handler, nullptr, expTimeValue,
                                     intervalValue);
  }

  /**
   * for resetting the interval an already registered task.
   * returns '0' if successful '-1' on failure.
   * id - the id assigned to the expiry task initially.
   * millisec - The time after which you would like it to get
   * invoked from the current time.
   */
  int resetTask(id_type id, uint32_t sec);

  template <class Rep, class Period>
  int resetTask(id_type id, std::chrono::duration<Rep, Period> duration) {
    ACE_Time_Value interval(duration);
    return m_reactor->reset_timer_interval(id, interval);
  }

  /**
   * for cancelling an already registered task.
   * returns '0' if successful '-1' on failure.
   * id - the id assigned to the expiry task initially.
   */
  int cancelTask(id_type id);

  /**
   * A separate thread is started in which the reactor event loop
   * is kept running unless explicitly stopped or when this object
   * goes out of scope.
   */
  int svc() override;

  /**
   * For explicitly stopping the reactor's event loop.
   */
  void stopExpiryTaskManager();

  /** activate the thread and wait for it to be running. */
  void begin();

 private:
  ACE_Reactor* m_reactor;

  bool m_reactorEventLoopRunning;  // flag to indicate if the reactor event
                                   // loop is running or not.
  static const char* NC_ETM_Thread;

  std::mutex m_mutex;
  std::condition_variable m_condition;

  std::unique_ptr<GF_Timer_Heap_ImmediateReset> m_timer;
};
}  // namespace client
}  // namespace geode
}  // namespace apache

#endif  // GEODE_EXPIRYTASKMANAGER_H_
