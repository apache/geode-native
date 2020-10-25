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



#include "MemoryPressureTask.hpp"
#include <windows.h>
#include <psapi.h>
#include "../Log.hpp"

namespace Apache
{
  namespace Geode
  {
    namespace Client
    {

      System::Int64 g_prevUnmanagedSize = 0;

      bool MemoryPressureTask::on_expire()
      {
        HANDLE hProcess = GetCurrentProcess( );

        PROCESS_MEMORY_COUNTERS pmc;

        if ( GetProcessMemoryInfo( hProcess, &pmc, sizeof(pmc)) ) {
          System::Int64 totalmem  = (System::Int64)pmc.WorkingSetSize;
          System::Int64 curr_managed_size = GC::GetTotalMemory( false );
          System::Int64 curr_unmanagedMemory = totalmem - curr_managed_size;
          Log::Finest( "Current total memory usage: {0}, managed memory: {1}, "
              "unmanaged memory: {2}", totalmem, curr_managed_size,
              curr_unmanagedMemory );
          if ( curr_unmanagedMemory > 0 ) {
            System::Int64 increase = curr_unmanagedMemory - g_prevUnmanagedSize;
            if ( Math::Abs( increase ) > 20*1024*1024 ) {
              if ( increase > 0 ) {
                Log::Fine( "Adding memory pressure information to assist .NET GC: {0} bytes", increase );
                GC::AddMemoryPressure( increase );
              }
              else {
                Log::Fine( "Removing memory pressure information to assist .NET GC: {0} bytes", -increase );
                GC::RemoveMemoryPressure( -increase );
              }
              g_prevUnmanagedSize = curr_unmanagedMemory;
            }
          }
        }
        else {
          Log::Error( "Error {0} while obtaining process memory info", GetLastError());
        }

        return true;
      }
    }  // namespace Client
  }  // namespace Geode
}  // namespace Apache
