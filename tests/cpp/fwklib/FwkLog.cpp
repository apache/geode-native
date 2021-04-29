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

#include <fwklib/FwkLog.hpp>
#include <iomanip>
#include <iostream>

#include <boost/asio/ip/host_name.hpp>
#include <boost/process.hpp>
#include <boost/thread.hpp>

#include <geode/Exception.hpp>

#include "hacks/AceThreadId.h"

namespace apache {
namespace geode {
namespace client {
namespace testframework {

const char* dirAndFile(const char* str) {
  if (!str) {
    return "NULL";
  }

  const char* ptr = str + strlen(str);
  int32_t found = 0;
  while (ptr > str) {
    ptr--;
    if ((*ptr == '/') || (*ptr == '\\')) {
      found++;
      if (found >= 2) {
        if (ptr == str) {
          return str;
        }
        return ++ptr;
      }
    }
  }
  return ptr;
}

void plog(const char* l, const char* s, const char* filename, int32_t lineno) {
  using std::chrono::duration_cast;
  using std::chrono::microseconds;
  using std::chrono::system_clock;

  auto now = system_clock::now();
  auto in_time_t = system_clock::to_time_t(now);
  auto localtime = std::localtime(&in_time_t);
  auto usec =
      duration_cast<microseconds>(now.time_since_epoch()).count() % 1000;

  const char* fil = dirAndFile(filename);
  std::cout << '[' << std::put_time(localtime, "%Y/%m/%d %H:%M:%S") << '.'
            << std::setfill('0') << std::setw(6) << usec << std::setw(0)
            << std::put_time(localtime, " %Z ") << boost::asio::ip::host_name()
            << ":P" << boost::this_process::get_id() << ":T"
            << boost::this_thread::get_id() << "]::" << fil << "::" << lineno
            << "  " << l << "  " << s << std::endl
            << std::flush;
}
}  // namespace testframework
}  // namespace client
}  // namespace geode
}  // namespace apache
