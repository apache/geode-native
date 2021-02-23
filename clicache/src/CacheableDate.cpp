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


#include "CacheableDate.hpp"
#include "DataInput.hpp"
#include "DataOutput.hpp"
#include "Log.hpp"
#include "Objects.hpp"

using namespace System;

namespace Apache
{
  namespace Geode
  {
    namespace Client
    {

      CacheableDate::CacheableDate(DateTime dateTime)
        : m_dateTime(dateTime), m_hashcode(0)
      {

        // Round off dateTime to the nearest millisecond.
        System::Int64 ticksToAdd = m_dateTime.Ticks % TimeSpan::TicksPerMillisecond;
        ticksToAdd = (ticksToAdd >= (TimeSpan::TicksPerMillisecond / 2) ?
                      (TimeSpan::TicksPerMillisecond - ticksToAdd) : -ticksToAdd);
        m_dateTime = m_dateTime.AddTicks(ticksToAdd);

      }

      void CacheableDate::ToData(DataOutput^ output)
      {
        //put as universal time
        TimeSpan epochSpan = m_dateTime.ToUniversalTime() - EpochTime;
        System::Int64 millisSinceEpoch =
          epochSpan.Ticks / TimeSpan::TicksPerMillisecond;
        output->WriteInt64(millisSinceEpoch);

        //Log::Fine("CacheableDate::Todata time " + m_dateTime.Ticks);
      }

      void CacheableDate::FromData(DataInput^ input)
      {
        DateTime epochTime = EpochTime;
        System::Int64 millisSinceEpoch = input->ReadInt64();
        m_dateTime = epochTime.AddTicks(
          millisSinceEpoch * TimeSpan::TicksPerMillisecond);
        m_dateTime = m_dateTime.ToLocalTime();
        //Log::Fine("CacheableDate::Fromadata time " + m_dateTime.Ticks);
      }

      System::UInt64 CacheableDate::ObjectSize::get()
      {
        return sizeof(DateTime);
      }

      int8_t CacheableDate::DsCode::get()
      {
        return static_cast<int8_t>(native::internal::DSCode::CacheableDate);
      }

      String^ CacheableDate::ToString()
      {
        return m_dateTime.ToString(
          System::Globalization::CultureInfo::CurrentCulture);
      }

      System::Int32 CacheableDate::GetHashCode()
      {
        if (m_hashcode == 0) {
          m_hashcode = Objects::GetHashCode(m_dateTime);
        }
        return m_hashcode;
      }

      bool CacheableDate::Equals(ICacheableKey^ other)
      {
        return Equals((Object^) other);
      }

      bool CacheableDate::Equals(Object^ obj)
      {
        CacheableDate^ otherDate =
          dynamic_cast<CacheableDate^>(obj);

        if (otherDate != nullptr) {
          return (m_dateTime == otherDate->m_dateTime);
        }
        return false;
      }
    }  // namespace Client
  }  // namespace Geode
}  // namespace Apache

