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

#ifndef GEODE_CONFIG_H_
#define GEODE_CONFIG_H_

/* #undef HAVE_SYS_MOUNT_H */

/* #undef CMAKE_USE_PTHREADS_INIT */
#if defined(CMAKE_USE_PTHREADS_INIT)
/* #undef HAVE_PTHREAD_H */
/* #undef HAVE_PTHREAD_HX */
/* #undef HAVE_pthread_setname_np */
#endif

/* #undef HAVE_uname */
/* #undef HAVE_SIGSTKFLT */
/* #undef HAVE_ACE_Select_Reactor */

// TODO replace with better CMake checks
/* #undef _LINUX */
/* #undef _MACOSX */
// TODO already defined #define _WIN32
/* #undef _SOLARIS */

#define PRODUCT_VENDOR "Apache"
#define PRODUCT_VENDOR_NAME "The Apache Software Foundation"
#define PRODUCT_NAME "Geode Native"
#define PRODUCT_BITS "64bit"
#define PRODUCT_LIB_NAME "apache-geode"
#define PRODUCT_DLL_NAME "Apache.Geode"

#define GEODE_SYSTEM_PROCESSOR "AMD64"
#define GEODE_SYSTEM_NAME "Windows"

// TODO relace with CMake checks
#define WITH_ACE_Select_Reactor 1

// ACE_Thread is a pointer on MacOS *only*
#if defined(_MACOSX)
#define ACE_Thread_NULL nullptr
#else
#define ACE_Thread_NULL 0
#endif

/* #undef WITH_IPV6 */

#endif  // GEODE_CONFIG_H_
