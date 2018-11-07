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

#if defined(__begin_native__hpp__)
#error Including begin_native.hpp mulitple times without end_native.hpp
#endif
#define __begin_native__hpp__

#pragma push_macro("_ALLOW_KEYWORD_MACROS")
#undef _ALLOW_KEYWORD_MACROS
#define _ALLOW_KEYWORD_MACROS

#pragma push_macro("nullptr")
#undef nullptr
#define nullptr __nullptr

#pragma push_macro("_M_CEE")
#undef _M_CEE

#pragma warning(push)

// Disable XML warnings
#pragma warning(disable: 4635)
#pragma warning(disable: 4638)
#pragma warning(disable: 4641)

// Disable native code generation warning
#pragma warning(disable: 4793)

// Disable /clr warnings
#pragma warning(disable: 4575)

//#pragma pack(push)

#pragma managed(push, off)

