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

#include "geode_defs.hpp"

namespace Apache
{
  namespace Geode
  {
    namespace Client
    {
      public enum class CqOperation
      {
        OP_TYPE_INVALID = -1,
        OP_TYPE_CREATE = 1,
        OP_TYPE_UPDATE = 2,
        OP_TYPE_INVALIDATE = 4,
        OP_TYPE_REGION_CLEAR = 8,
        OP_TYPE_DESTROY = 16,
        OP_TYPE_MARKER = 32
      };

    }  // namespace Client
  }  // namespace Geode
}  // namespace Apache


