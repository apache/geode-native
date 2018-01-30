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


#include "begin_native.hpp"
#include <chrono>
#include <geode/internal/chrono/duration.hpp>
#include "end_native.hpp"

namespace Apache
{
  namespace Geode
  {
    namespace Client
    {
      using namespace System;
      using namespace apache::geode::internal::chrono::duration;

      using ticks = std::chrono::duration<long long, std::ratio<1, 10000000>>;
      
      class TimeUtils
      {
      public:
        template <class _Duration>
        inline static _Duration TimeSpanToDurationCeil(TimeSpan timeSpan)
        {
          return _ceil<_Duration>(TimeSpanToDuration(timeSpan));
        }
      
        inline static ticks TimeSpanToDuration(TimeSpan timespan)
        {
          return ticks(timespan.Ticks);
        }

        inline static TimeSpan DurationToTimeSpan(ticks duration)
        {
          return TimeSpan::FromTicks(duration.count());
        }    

        inline static DateTime TimePointToDateTime(std::chrono::system_clock::time_point timePoint) {
          using namespace std::chrono;
          auto t = duration_cast<ticks>(timePoint.time_since_epoch());
          t += epochDifference;
          return DateTime(t.count());
        }

        inline static std::chrono::system_clock::time_point DateTimeToTimePoint(DateTime dateTime) {
          using namespace std::chrono;
          auto t = ticks(dateTime.Ticks);
          t -= epochDifference;
          return system_clock::time_point(t);
        }

      private:
        static constexpr auto epochDifference = ticks(621355968000000000);
      };
    }  // namespace Client
  }  // namespace Geode
}  // namespace Apache

