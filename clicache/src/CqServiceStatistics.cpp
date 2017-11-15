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


#include "CqServiceStatistics.hpp"


namespace Apache
{
  namespace Geode
  {
    namespace Client
    {
      using namespace System;

      System::UInt32 CqServiceStatistics::numCqsActive()
      {
        try
        {
          return m_nativeptr->get()->numCqsActive();
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }
      }
      System::UInt32 CqServiceStatistics::numCqsCreated()
      {
        try
        {
          return m_nativeptr->get()->numCqsCreated();
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }
      }
      System::UInt32 CqServiceStatistics::numCqsClosed()
      {
        try
        {
          return m_nativeptr->get()->numCqsClosed();
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }
      }
      System::UInt32 CqServiceStatistics::numCqsStopped()
      {
        try
        {
          return m_nativeptr->get()->numCqsStopped();
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }
      }
      System::UInt32 CqServiceStatistics::numCqsOnClient()
      {
        try
        {
          return m_nativeptr->get()->numCqsOnClient();
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }
      }
    }  // namespace Client
  }  // namespace Geode
}  // namespace Apache
