#pragma once

#ifndef GEODE_HOSTASM_H_
#define GEODE_HOSTASM_H_

#ifdef __ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#endif

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

/*

This file wraps the assembly spinlock routines, and related atomic update
routines in the class apache::geode::client::util::util_impl::Host. Some ace is
included.

*/

#include "config.h"
#include <geode/geode_globals.hpp>
#include <ace/ACE.h>
#include <ace/Time_Value.h>
#include <ace/OS_NS_time.h>
#include <ace/OS_NS_sys_time.h>
#include <ace/Thread.h>
#ifdef _X86_SOLARIS
#include <ace/Thread.h>
#include <sys/atomic.h>
#endif
#if defined(_MACOSX)
#include <libkern/OSAtomic.h>
#endif
/**
 * @file
 */

namespace apache {
namespace geode {
namespace client {


#ifdef _SPARC_SOLARIS
// implemented in hostsolaris.asm
extern "C" {
int32_t InterlockedExchangeAdd(volatile int32_t*, int32_t);
// int64_t InterlockedExchangeAddLong(volatile int64_t *, int64_t);
}
#endif
/*
#ifdef _X86_SOLARIS
  extern "C" {
typedef long LONG;
  int32_t InterlockedExchangeAdd(volatile int32_t *, int32_t);
int32_t InterlockedCompareExchange(volatile LONG*, int32_t, int32_t);
}
#endif*/
/**
 *  hold static wrappers for spinlocks and atomic updates..
 */
class CPPCACHE_EXPORT HostAsm {
 public:

#if defined(_LINUX) || defined(_X86_SOLARIS)
  inline static int32_t InterlockedExchangeAdd(volatile int32_t* val,
                                               int32_t add) {
#if defined(_LINUX)
    int32_t ret;
    __asm__ __volatile__("lock; xaddl %0, %1"
                         : "=r"(ret), "=m"(*val)
                         : "0"(add), "m"(*val));

    return (ret);
#endif
#if defined(_X86_SOLARIS)
    int32_t ret = *val;
    atomic_add_32((volatile uint32_t*)val, add);
    return ret + add;
#endif
  }
// _SOLARIS case is handled in hostsolaris.asm
#endif

  // _SOLARIS case is handled in hostsolaris.asm

  /**
   * Name - atomicAdd
   * Purpose -
   *   Add 'increment' to  the counter pointed to be 'ctrPtr'.
   *   Returns the value of the counter after the addition
   */
  inline static int32_t atomicAdd(volatile int32_t& counter, int32_t delta) {
#ifdef _WIN32
    return InterlockedExchangeAdd((volatile LONG*)(&counter), delta) + delta;
#endif

#ifdef _X86_SOLARIS
    int32_t ret = counter;
    atomic_add_32((volatile uint32_t*)&counter, delta);
    return ret + delta;
#endif

#if defined(_LINUX) || defined(_SPARC_SOLARIS)
    return InterlockedExchangeAdd(&counter, delta) + delta;
#endif

#if defined(_MACOSX)
    return OSAtomicAdd32Barrier(delta, &counter);
#endif
  }

  /**
   * Name - atomicAddPostfix
   * Purpose -
   *   Add 'increment' to  the counter pointed to be 'ctrPtr'.
   *   Returns the value of the counter before the addition
   */
  inline static int32_t atomicAddPostfix(volatile int32_t& counter,
                                         int32_t delta) {
#if defined(_WIN32)
    return InterlockedExchangeAdd((volatile LONG*)(&counter), delta);
#elif defined(_X86_SOLARIS)
    int32_t ret = counter;
    atomic_add_32((volatile uint32_t*)(&counter), delta);
    return ret;
#elif defined(_LINUX) || defined(_SPARC_SOLARIS)
    return InterlockedExchangeAdd(&counter, delta);
#elif defined(_MACOSX)
    int32_t ret = counter;
    OSAtomicAdd32Barrier(delta, &counter);
    return ret;
#else
#error Port incomplete
#endif
  }

  /**
   * Name - AtomicAnd
   *   Atomically AND the mask value into the given address
   */
  static void atomicAnd(volatile uint32_t& ctrField, uint32_t mask);

  static uint32_t atomicCompareAndExchange(volatile uint32_t& oldValue,
                                           uint32_t newValue,
                                           uint32_t valueToCompare);

  /**
   * Name - AtomicOr
   *  Atomically OR the mask value into the given address
   */
  static void atomicOr(volatile uint32_t& ctrField, uint32_t mask);

  /**
   * Atomically set masked bits to 1 in data.
   */
  inline static void atomicSetBits(volatile uint32_t& data, uint32_t mask) {
    return atomicOr(data, mask);
  }

  /**
   * Atomically set value of data to the given value.
   */
  static void atomicSet(volatile uint32_t& data, uint32_t newValue);

  /**
   * Atomically set masked bits to 0 in data.
   */
  inline static void atomicClearBits(volatile uint32_t& data, uint32_t mask) {
    return atomicAnd(data, ~mask);
  }

};
}  // namespace client
}  // namespace geode
}  // namespace apache

#ifdef __ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__
#pragma clang diagnostic pop
#endif

#endif  // GEODE_HOSTASM_H_
