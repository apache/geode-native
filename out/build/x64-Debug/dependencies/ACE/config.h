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
 
#define ACE_LACKS_IOSTREAM_TOTALLY 1
#define ACE_HAS_VERSIONED_NAMESPACE 1

#if __cplusplus >= 201103L

#if defined(__SUNPRO_CC)
#define ACE_HAS_CPP11 1
#define _RWSTD_ALLOCATOR 1
#endif

#endif // __cplusplus >= 201103L

/* #undef WITH_IPV6 */
#ifdef WITH_IPV6
#define ACE_HAS_IPV6 1
#endif

#include "ace/config-win32.h"

