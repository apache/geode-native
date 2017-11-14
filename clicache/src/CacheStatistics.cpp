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

#include "CacheStatistics.hpp"
#include "TimeUtils.hpp"

namespace Apache
{
  namespace Geode
  {
    namespace Client
    {

      using namespace System;

      System::DateTime CacheStatistics::LastModifiedTime::get( )
      {
        try
        {
          return TimeUtils::TimePointToDateTime(m_nativeptr->get()->getLastModifiedTime());
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }
      }

      System::DateTime CacheStatistics::LastAccessedTime::get()
      {
        try
        {
          return TimeUtils::TimePointToDateTime(m_nativeptr->get()->getLastAccessedTime());
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }
      }
    }  // namespace Client
  }  // namespace Geode
}  // namespace Apache
