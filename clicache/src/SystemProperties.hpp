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
#include "begin_native.hpp"
#include <geode/SystemProperties.hpp>
#include "end_native.hpp"

#include "Log.hpp"
#include "Properties.hpp"

using namespace System;

namespace Apache
{
  namespace Geode
  {
    namespace Client
    {
      namespace native = apache::geode::client;

      /// <summary>
      /// A class for internal use, that encapsulates the properties that can be
      /// set through <see cref="DistributedSystem.Connect" />
      /// or a geode.properties file.
      /// </summary>
      public ref class SystemProperties sealed
      {
      public:

        /// <summary>
        /// Prints all settings to the process log.
        /// </summary>
        void LogSettings();

        /// <summary>
        /// Returns the sampling interval, that is,
        /// how often the statistics thread writes to disk, in seconds.
        /// </summary>
        /// <returns>the statistics sampling interval</returns>
        property System::Int32 StatisticsSampleInterval
        {
          System::Int32 get();
        }

        /// <summary>
        /// True if statistics are enabled (archived).
        /// </summary>
        /// <returns>true if enabled</returns>
        property bool StatisticsEnabled
        {
          bool get();
        }

        /// <summary>
        /// Returns the name of the statistics archive file.
        /// </summary>
        /// <returns>the filename</returns>
        property String^ StatisticsArchiveFile
        {
          String^ get();
        }

        /// <summary>
        /// Returns the name of the message log file.
        /// </summary>
        /// <returns>the filename</returns>
        property String^ LogFileName
        {
          String^ get();
        }

        /// <summary>
        /// Returns the message logging level.
        /// </summary>
        /// <returns>the log level</returns>
        property LogLevel GFLogLevel
        {
          LogLevel get();
        }

        /// <summary>
        /// Returns  a boolean that specifies if heapLRULimit has been enabled for the
        /// process. If enabled, the HeapLRULimit specifies the maximum amount of memory
        /// that values in a cache can use to store data before overflowing to disk or
        /// destroying entries to ensure that the server process never runs out of
        /// memory
        /// </summary>
        /// <returns>true if enabled</returns>
        property bool HeapLRULimitEnabled
        {
          bool get();
        }

        /// <summary>
        /// Returns  the HeapLRULimit value (in bytes), the maximum memory that values
        /// in a cache can use to store data before overflowing to disk or destroying
        /// entries to ensure that the server process never runs out of memory due to
        /// cache memory usage
        /// </summary>
        /// <returns>the HeapLRULimit value</returns>
        property size_t HeapLRULimit
        {
          size_t get();
        }

        /// <summary>
        /// Returns  the HeapLRUDelta value (a percent value). This specifies the
        /// percentage of entries the system will evict each time it detects that
        /// it has exceeded the HeapLRULimit. Defaults to 10%
        /// </summary>
        /// <returns>the HeapLRUDelta value</returns>
        property System::Int32 HeapLRUDelta
        {
          System::Int32 get();
        }

        /// <summary>
        /// Returns  the maximum socket buffer size to use
        /// </summary>
        /// <returns>the MaxSocketBufferSize value</returns>
        property System::Int32 MaxSocketBufferSize
        {
          System::Int32 get();
        }

        /// <summary>
        /// Returns  the time between two consecutive ping to servers
        /// </summary>
        /// <returns>the PingInterval value</returns>
        property System::Int32 PingInterval
        {
          System::Int32 get();
        }

        /// <summary>
        /// Returns  the time between two consecutive checks for redundancy for HA
        /// </summary>
        /// <returns>the RedundancyMonitorInterval value</returns>
        property System::Int32 RedundancyMonitorInterval
        {
          System::Int32 get();
        }

        /// <summary>
        /// Returns the periodic notify ack interval
        /// </summary>
        /// <returns>the NotifyAckInterval value</returns>
        property System::Int32 NotifyAckInterval
        {
          System::Int32 get();
        }

        /// <summary>
        /// Returns the expiry time of an idle event id map entry for duplicate notification checking
        /// </summary>
        /// <returns>the NotifyDupCheckLife value</returns>
        property System::Int32 NotifyDupCheckLife
        {
          System::Int32 get();
        }

        /// <summary>
        /// True if the stack trace is enabled.
        /// </summary>
        /// <returns>true if enabled</returns>
        property bool DebugStackTraceEnabled
        {
          bool get();
        }

        /// <summary>
        /// True if the crash dump generation for unhandled fatal exceptions
        /// is enabled. If "log-file" property has been specified then they are
        /// created in the same directory as the log file, and having the same
        /// prefix as log file. By default crash dumps are created in the
        /// current working directory and have the "geode_cpp" prefix.
        ///
        /// The actual dump file will have timestamp and process ID
        /// in the full name.
        /// </summary>
        /// <returns>true if enabled</returns>
        property bool CrashDumpEnabled
        {
          bool get();
        }

        /// <summary>
        /// Whether client is running in multiple AppDomain or not.
        /// Default value is "false".
        /// </summary>
        /// <returns>true if enabled</returns>
        property bool AppDomainEnabled
        {
          bool get();
        }

        /// <summary>
        /// Returns the system name.
        /// </summary>
        /// <returns>the name</returns>
        property String^ Name
        {
          String^ get();
        }

        /// <summary>
        /// Returns the name of the "cache.xml" file.
        /// </summary>
        /// <returns>the filename</returns>
        property String^ CacheXmlFile
        {
          String^ get();
        }

        /// <summary>
        /// Returns the maximum log file size, in bytes, or 0 if unlimited.
        /// </summary>
        /// <returns>the maximum limit</returns>
        property System::Int32 LogFileSizeLimit
        {
          System::Int32 get();
        }

        /// <summary>
        /// Returns the maximum log Disk size, in bytes, or 0 if unlimited.
        /// </summary>
        /// <returns>the maximum limit</returns>
        property System::Int32 LogDiskSpaceLimit
        {
          System::Int32 get();
        }

        /// <summary>
        /// Returns the maximum statistics file size, in bytes, or 0 if unlimited.
        /// </summary>
        /// <returns>the maximum limit</returns>
        property System::Int32 StatsFileSizeLimit
        {
          System::Int32 get();
        }

        /// <summary>
        /// Returns the maximum statistics Disk size, in bytes, or 0 if unlimited.
        /// </summary>
        /// <returns>the maximum limit</returns>
        property System::Int32 StatsDiskSpaceLimit
        {
          System::Int32 get();
        }

        /// <summary>
        /// Returns the max queue size for notification messages
        /// </summary>
        /// <returns>the max queue size</returns>
        property System::UInt32 MaxQueueSize
        {
          System::UInt32 get();
        }

        /// <summary>
        /// True if ssl connection support is enabled.
        /// </summary>
        /// <returns>true if enabled</returns>
        property bool SSLEnabled
        {
          bool get();
        }

        /// <summary>
        /// Returns the SSL private keystore file path.
        /// </summary>
        /// <returns>the SSL private keystore file path</returns>
        property String^ SSLKeyStore
        {
          String^ get();
        }

        /// <summary>
        /// Returns the SSL public certificate trust store file path.
        /// </summary>
        /// <returns>the SSL public certificate trust store file path</returns>
        property String^ SSLTrustStore
        {
          String^ get();
        }

        // adongre
        /// <summary>
        /// Returns the client keystore password..
        /// </summary>
        /// <returns>Returns the client keystore password.</returns>
        property String^ SSLKeystorePassword
        {
          String^ get();
        }

        /// <summary>
        /// Returns all the security properties
        /// </summary>
        /// <returns>the security properties</returns>
        //generic <class TPropKey, class TPropValue>
        property Properties<String^, String^>^ GetSecurityProperties {
          Properties<String^, String^>^ get();
        }

        /// <summary>
        /// Returns the durable client's ID.
        /// </summary>
        /// <returns>the durable client ID</returns>
        property String^ DurableClientId
        {
          String^ get();
        }

        /// <summary>
        /// Returns the durable client's timeout.
        /// </summary>
        /// <returns>the durable client timeout</returns>
        property System::UInt32 DurableTimeout
        {
          System::UInt32 get();
        }

        /// <summary>
        /// Returns the connect timeout used for server and locator handshakes.
        /// </summary>
        /// <returns>the connect timeout used for server and locator handshakes</returns>
        property System::UInt32 ConnectTimeout
        {
          System::UInt32 get();
        }

        /// <summary>
        /// Returns the conflate event's option
        /// </summary>
        /// <returns>the conflate event option</returns>
        property String^ ConflateEvents
        {
          String^ get();
        }

        /// <summary>
        /// Returns the timeout after which suspended transactions are rolled back.
        /// </summary>
        /// <returns>the timeout for suspended transactions</returns>
        property System::UInt32 SuspendedTxTimeout
        {
          System::UInt32 get();
        }

        /// <summary>
        /// This can be called to know whether read timeout unit is in milli second.
        /// </summary>
        /// <returns>true if enabled or false by default.</returns>
        property bool ReadTimeoutUnitInMillis
        {
          bool get();
        }
        /// <summary>
        /// True if app want to clear pdx types ids on client disconnect
        /// </summary>
        /// <returns>true if enabled</returns>
        property bool OnClientDisconnectClearPdxTypeIds
        {
          bool get();
        }

      internal:

        /// <summary>
        /// Internal factory function to wrap a native object pointer inside
        /// this managed class, with null pointer check.
        /// </summary>
        /// <param name="nativeptr">native object pointer</param>
        /// <returns>
        /// the managed wrapper object, or null if the native pointer is null.
        /// </returns>
        inline static SystemProperties^ Create(
          native::SystemProperties* nativeptr)
        {
          return (nativeptr != nullptr ?
                  gcnew SystemProperties(nativeptr) : nullptr);
        }


      private:

        /// <summary>
        /// Private constructor to wrap a native object pointer
        /// </summary>
        /// <param name="nativeptr">The native object pointer</param>
        inline SystemProperties(native::SystemProperties* nativeptr)
          : m_nativeptr(nativeptr)
        {
        }

        native::SystemProperties* m_nativeptr;
      };
    }  // namespace Client
  }  // namespace Geode
}  // namespace Apache

