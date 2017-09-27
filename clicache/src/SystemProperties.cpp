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

#include "SystemProperties.hpp"
#include "impl/SafeConvert.hpp"

using namespace System;

namespace Apache
{
  namespace Geode
  {
    namespace Client
    {
      namespace native = apache::geode::client;

      void SystemProperties::LogSettings( )
      {
        m_nativeptr->logSettings( );
      }

      System::Int32 SystemProperties::StatisticsSampleInterval::get( )
      {
        return m_nativeptr->statisticsSampleInterval( );
      }

      bool SystemProperties::StatisticsEnabled::get( )
      {
        return m_nativeptr->statisticsEnabled( );
      }

      String^ SystemProperties::StatisticsArchiveFile::get( )
      {
        return ManagedString::Get( m_nativeptr->statisticsArchiveFile( ) );
      }

      String^ SystemProperties::LogFileName::get( )
      {
        return ManagedString::Get( m_nativeptr->logFilename( ) );
      }

      LogLevel SystemProperties::GFLogLevel::get( )
      {
        return static_cast<LogLevel>( m_nativeptr->logLevel( ) );
      }

      bool SystemProperties::HeapLRULimitEnabled::get( )
      {
        return m_nativeptr->heapLRULimitEnabled( );
      }
      
      size_t SystemProperties::HeapLRULimit::get( )
      {
        return m_nativeptr->heapLRULimit( );
      }
      
      System::Int32 SystemProperties::HeapLRUDelta::get( )
      {
        return m_nativeptr->heapLRUDelta( );
      }
      
      System::Int32 SystemProperties::MaxSocketBufferSize::get( )
      {
        return m_nativeptr->maxSocketBufferSize( );
      }
      
      System::Int32 SystemProperties::PingInterval::get( )
      {
        return m_nativeptr->pingInterval( );
      }
      
      System::Int32 SystemProperties::RedundancyMonitorInterval::get( )
      {
        return m_nativeptr->redundancyMonitorInterval( );
      }
      
      System::Int32 SystemProperties::NotifyAckInterval::get( )
      {
        return m_nativeptr->notifyAckInterval( );
      }
      
      System::Int32 SystemProperties::NotifyDupCheckLife::get( )
      {
        return m_nativeptr->notifyDupCheckLife( );
      }
      
      bool SystemProperties::DebugStackTraceEnabled::get( )
      {
        return m_nativeptr->debugStackTraceEnabled( );
      }

      bool SystemProperties::CrashDumpEnabled::get( )
      {
        return m_nativeptr->crashDumpEnabled();
      }

      bool SystemProperties::AppDomainEnabled::get( )
      {
        return m_nativeptr->isAppDomainEnabled();
      }

      String^ SystemProperties::Name::get( )
      {
        return ManagedString::Get( m_nativeptr->name( ) );
      }

      String^ SystemProperties::CacheXmlFile::get( )
      {
        return ManagedString::Get( m_nativeptr->cacheXMLFile( ) );
      }

      System::Int32 SystemProperties::LogFileSizeLimit::get( )
      {
        return m_nativeptr->logFileSizeLimit( );
      }

	  System::Int32 SystemProperties::LogDiskSpaceLimit::get( )
      {
		  return m_nativeptr->logDiskSpaceLimit( );
      }

      System::Int32 SystemProperties::StatsFileSizeLimit::get( )
      {
        return m_nativeptr->statsFileSizeLimit( );
      }

	  System::Int32 SystemProperties::StatsDiskSpaceLimit::get( )
      {
		  return m_nativeptr->statsDiskSpaceLimit( );
      }

      System::UInt32 SystemProperties::MaxQueueSize::get( )
      {
        return m_nativeptr->maxQueueSize( );
      }

      bool SystemProperties::SSLEnabled::get( )
      {
        return m_nativeptr->sslEnabled();
      }

      String^ SystemProperties::SSLKeyStore::get()
      {
        return ManagedString::Get(m_nativeptr->sslKeyStore());
      }

      String^ SystemProperties::SSLTrustStore::get()
      {
        return ManagedString::Get(m_nativeptr->sslTrustStore());
      }
      
      // adongre
      String^ SystemProperties::SSLKeystorePassword::get()
      {
        return ManagedString::Get(m_nativeptr->sslKeystorePassword());
      }


      Properties<String^, String^>^ SystemProperties::GetSecurityProperties::get( )
      {
        return Properties<String^, String^>::Create(m_nativeptr->getSecurityProperties());
      }

      String^ SystemProperties::DurableClientId::get( )
      {
        return ManagedString::Get( m_nativeptr->durableClientId( ) );
      }

      System::UInt32 SystemProperties::DurableTimeout::get( )
      {
        return m_nativeptr->durableTimeout( );
      }

      System::UInt32 SystemProperties::ConnectTimeout::get( )
      {
        return m_nativeptr->connectTimeout( );
      }

      String^ SystemProperties::ConflateEvents::get( )
      {
        return ManagedString::Get( m_nativeptr->conflateEvents( ) );
      }

      System::UInt32 SystemProperties::SuspendedTxTimeout::get( )
      {
        return m_nativeptr->suspendedTxTimeout( );
      }

      bool SystemProperties::ReadTimeoutUnitInMillis::get( )
      {
        return m_nativeptr->readTimeoutUnitInMillis( );
      }

       bool SystemProperties::OnClientDisconnectClearPdxTypeIds::get( )
      {
        return m_nativeptr->onClientDisconnectClearPdxTypeIds( );
    }  // namespace Client
  }  // namespace Geode
}  // namespace Apache

}
