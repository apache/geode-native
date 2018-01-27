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
#include "TimeUtils.hpp"


namespace Apache
{
  namespace Geode
  {
    namespace Client
    {
      using namespace System;
      using namespace msclr::interop;

      namespace native = apache::geode::client;

      void SystemProperties::LogSettings( )
      {
        m_nativeptr->logSettings( );
      }

      TimeSpan SystemProperties::StatisticsSampleInterval::get( )
      {
        return TimeUtils::DurationToTimeSpan(m_nativeptr->statisticsSampleInterval( ));
      }

      bool SystemProperties::StatisticsEnabled::get( )
      {
        return m_nativeptr->statisticsEnabled( );
      }

      String^ SystemProperties::StatisticsArchiveFile::get( )
      {
        return marshal_as<String^>( m_nativeptr->statisticsArchiveFile( ) );
      }

      String^ SystemProperties::LogFileName::get( )
      {
        return marshal_as<String^>( m_nativeptr->logFilename( ) );
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
      
      TimeSpan SystemProperties::PingInterval::get( )
      {
        return TimeUtils::DurationToTimeSpan(m_nativeptr->pingInterval( ));
      }
      
      TimeSpan SystemProperties::RedundancyMonitorInterval::get( )
      {
        return TimeUtils::DurationToTimeSpan(m_nativeptr->redundancyMonitorInterval( ));
      }
      
      TimeSpan SystemProperties::NotifyAckInterval::get( )
      {
        return TimeUtils::DurationToTimeSpan(m_nativeptr->notifyAckInterval( ));
      }
      
      TimeSpan SystemProperties::NotifyDupCheckLife::get( )
      {
        return TimeUtils::DurationToTimeSpan(m_nativeptr->notifyDupCheckLife( ));
      }
      
      String^ SystemProperties::Name::get( )
      {
        return marshal_as<String^>( m_nativeptr->name( ) );
      }

      String^ SystemProperties::CacheXmlFile::get( )
      {
        return marshal_as<String^>( m_nativeptr->cacheXMLFile( ) );
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

      bool SystemProperties::SSLEnabled::get( )
      {
        return m_nativeptr->sslEnabled();
      }

      String^ SystemProperties::SSLKeyStore::get()
      {
        return marshal_as<String^>(m_nativeptr->sslKeyStore());
      }

      String^ SystemProperties::SSLTrustStore::get()
      {
        return marshal_as<String^>(m_nativeptr->sslTrustStore());
      }
      
      // adongre
      String^ SystemProperties::SSLKeystorePassword::get()
      {
        return marshal_as<String^>(m_nativeptr->sslKeystorePassword());
      }


      Properties<String^, String^>^ SystemProperties::GetSecurityProperties::get( )
      {
        return Properties<String^, String^>::Create(m_nativeptr->getSecurityProperties());
      }

      String^ SystemProperties::DurableClientId::get( )
      {
        return marshal_as<String^>( m_nativeptr->durableClientId( ) );
      }

      TimeSpan SystemProperties::DurableTimeout::get( )
      {
        return TimeUtils::DurationToTimeSpan(m_nativeptr->durableTimeout( ));
      }

      TimeSpan SystemProperties::ConnectTimeout::get( )
      {
        return TimeUtils::DurationToTimeSpan(m_nativeptr->connectTimeout( ));
      }

      String^ SystemProperties::ConflateEvents::get( )
      {
        return marshal_as<String^>( m_nativeptr->conflateEvents( ) );
      }

      TimeSpan SystemProperties::SuspendedTxTimeout::get( )
      {
        return TimeUtils::DurationToTimeSpan(m_nativeptr->suspendedTxTimeout( ));
      }

       bool SystemProperties::OnClientDisconnectClearPdxTypeIds::get( )
      {
        return m_nativeptr->onClientDisconnectClearPdxTypeIds( );
      }
    }  // namespace Client
  }  // namespace Geode
}  // namespace Apache
