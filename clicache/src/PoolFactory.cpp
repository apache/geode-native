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





#include "Pool.hpp"
#include "PoolFactory.hpp"

#include "ExceptionTypes.hpp"
#include "Cache.hpp"
#include "TimeUtils.hpp"

namespace Apache
{
  namespace Geode
  {
    namespace Client
    {

      using namespace System;

      PoolFactory^ PoolFactory::SetFreeConnectionTimeout( TimeSpan connectionTimeout )
		  {
			  _GF_MG_EXCEPTION_TRY2/* due to auto replace */

			  try
			  {
			    m_nativeptr->get()->setFreeConnectionTimeout( TimeUtils::TimeSpanToDurationCeil<std::chrono::milliseconds>(connectionTimeout) );
			  }
			  finally
			  {
			    GC::KeepAlive(m_nativeptr);
			  }

			  _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */
          return this;
		  }


		  PoolFactory^ PoolFactory::SetLoadConditioningInterval( TimeSpan loadConditioningInterval )
		  {
			  _GF_MG_EXCEPTION_TRY2/* due to auto replace */

			  try
			  {
			    m_nativeptr->get()->setLoadConditioningInterval( TimeUtils::TimeSpanToDurationCeil<std::chrono::milliseconds>(loadConditioningInterval) );
			  }
			  finally
			  {
			    GC::KeepAlive(m_nativeptr);
			  }

			  _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */
          return this;
		  }


		  PoolFactory^ PoolFactory::SetSocketBufferSize( Int32 bufferSize )
      {
			  _GF_MG_EXCEPTION_TRY2/* due to auto replace */

			  try
			  {
			    m_nativeptr->get()->setSocketBufferSize( bufferSize );
			  }
			  finally
			  {
			    GC::KeepAlive(m_nativeptr);
			  }

			  _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */
          return this;
		  }


		  PoolFactory^ PoolFactory::SetReadTimeout( TimeSpan timeout )
      {
			  _GF_MG_EXCEPTION_TRY2/* due to auto replace */

			  try
			  {
			    m_nativeptr->get()->setReadTimeout( TimeUtils::TimeSpanToDurationCeil<std::chrono::milliseconds>(timeout) );
			  }
			  finally
			  {
			    GC::KeepAlive(m_nativeptr);
			  }

			  _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */
          return this;
		  }


		  PoolFactory^ PoolFactory::SetMinConnections( Int32 minConnections )
      {
			  _GF_MG_EXCEPTION_TRY2/* due to auto replace */

			  try
			  {
			    m_nativeptr->get()->setMinConnections( minConnections );
			  }
			  finally
			  {
			    GC::KeepAlive(m_nativeptr);
			  }

			  _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */
          return this;
		  }


		  PoolFactory^ PoolFactory::SetMaxConnections( Int32 maxConnections )
      {
			  _GF_MG_EXCEPTION_TRY2/* due to auto replace */

			  try
			  {
			    m_nativeptr->get()->setMaxConnections( maxConnections );
			  }
			  finally
			  {
			    GC::KeepAlive(m_nativeptr);
			  }

			  _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */
          return this;
		  }


		  PoolFactory^ PoolFactory::SetIdleTimeout( TimeSpan idleTimeout )
      {
			  _GF_MG_EXCEPTION_TRY2/* due to auto replace */

			  try
			  {
			    m_nativeptr->get()->setIdleTimeout( TimeUtils::TimeSpanToDurationCeil<std::chrono::milliseconds>(idleTimeout) );
			  }
			  finally
			  {
			    GC::KeepAlive(m_nativeptr);
			  }

			  _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */
          return this;
		  }


		  PoolFactory^ PoolFactory::SetRetryAttempts( Int32 retryAttempts )
      {
			  _GF_MG_EXCEPTION_TRY2/* due to auto replace */

			  try
			  {
			    m_nativeptr->get()->setRetryAttempts( retryAttempts );
			  }
			  finally
			  {
			    GC::KeepAlive(m_nativeptr);
			  }

			  _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */
          return this;
		  }


		  PoolFactory^ PoolFactory::SetPingInterval( TimeSpan pingInterval )
      {
			  _GF_MG_EXCEPTION_TRY2/* due to auto replace */

			  try
			  {
			    m_nativeptr->get()->setPingInterval( TimeUtils::TimeSpanToDurationCeil<std::chrono::milliseconds>(pingInterval) );
			  }
			  finally
			  {
			    GC::KeepAlive(m_nativeptr);
			  }

			  _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */
          return this;
		  }


		  PoolFactory^ PoolFactory::SetUpdateLocatorListInterval( TimeSpan updateLocatorListInterval )
      {
			  _GF_MG_EXCEPTION_TRY2/* due to auto replace */

			  try
			  {
			    m_nativeptr->get()->setUpdateLocatorListInterval( TimeUtils::TimeSpanToDurationCeil<std::chrono::milliseconds>(updateLocatorListInterval) );
			  }
			  finally
			  {
			    GC::KeepAlive(m_nativeptr);
			  }

			  _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */
          return this;
		  }


      PoolFactory^ PoolFactory::SetStatisticInterval( TimeSpan statisticInterval )
      {
			  _GF_MG_EXCEPTION_TRY2/* due to auto replace */

			  try
			  {
			    m_nativeptr->get()->setStatisticInterval( TimeUtils::TimeSpanToDurationCeil<std::chrono::milliseconds>(statisticInterval) );
			  }
			  finally
			  {
			    GC::KeepAlive(m_nativeptr);
			  }

			  _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */
          return this;
		  }

      PoolFactory^ PoolFactory::SetServerGroup( String^ group )
      {
			  _GF_MG_EXCEPTION_TRY2/* due to auto replace */

			  try
			  {
			    m_nativeptr->get()->setServerGroup( marshal_as<std::string>(group) );
			  }
			  finally
			  {
			    GC::KeepAlive(m_nativeptr);
			  }

			  _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */
          return this;
		  }


		  PoolFactory^ PoolFactory::AddLocator( String^ host, Int32 port )
      {
			  _GF_MG_EXCEPTION_TRY2/* due to auto replace */

			  try
			  {
			    m_nativeptr->get()->addLocator( marshal_as<std::string>(host), port );
			  }
			  finally
			  {
			    GC::KeepAlive(m_nativeptr);
			  }

			  _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */
          return this;
		  }


      PoolFactory^ PoolFactory::AddServer( String^ host, Int32 port )
      {
			  _GF_MG_EXCEPTION_TRY2/* due to auto replace */

			  try
			  {
			    m_nativeptr->get()->addServer( marshal_as<std::string>(host), port );
			  }
			  finally
			  {
			    GC::KeepAlive(m_nativeptr);
			  }

			  _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */
          return this;
		  }

		  PoolFactory^ PoolFactory::SetSniProxy(String^ hostname, Int32 port)
      {
			  _GF_MG_EXCEPTION_TRY2/* due to auto replace */

			  try
			  {
			    m_nativeptr->get()->setSniProxy( marshal_as<std::string>(hostname), port );
			  }
			  finally
			  {
			    GC::KeepAlive(m_nativeptr);
			  }

			  _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */
          return this;
		  }

		  PoolFactory^ PoolFactory::SetSubscriptionEnabled( Boolean enabled )
      {
			  _GF_MG_EXCEPTION_TRY2/* due to auto replace */

			  try
			  {
			    m_nativeptr->get()->setSubscriptionEnabled( enabled );
			  }
			  finally
			  {
			    GC::KeepAlive(m_nativeptr);
			  }

			  _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */
          return this;
		  }


          PoolFactory^ PoolFactory::SetPRSingleHopEnabled( Boolean enabled )
          {
            _GF_MG_EXCEPTION_TRY2/* due to auto replace */

              try
              {
                m_nativeptr->get()->setPRSingleHopEnabled(enabled);
              }
              finally
              {
                GC::KeepAlive(m_nativeptr);
              }

             _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */
               return this;
          }

    
		  PoolFactory^ PoolFactory::SetSubscriptionRedundancy( Int32 redundancy )
      {
			  _GF_MG_EXCEPTION_TRY2/* due to auto replace */

			  try
			  {
			    m_nativeptr->get()->setSubscriptionRedundancy( redundancy );
			  }
			  finally
			  {
			    GC::KeepAlive(m_nativeptr);
			  }

			  _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */
          return this;
		  }


		  PoolFactory^ PoolFactory::SetSubscriptionMessageTrackingTimeout( TimeSpan messageTrackingTimeout )
      {
			  _GF_MG_EXCEPTION_TRY2/* due to auto replace */

			  try
			  {
			    m_nativeptr->get()->setSubscriptionMessageTrackingTimeout( TimeUtils::TimeSpanToDurationCeil<std::chrono::milliseconds>(messageTrackingTimeout) );
			  }
			  finally
			  {
			    GC::KeepAlive(m_nativeptr);
			  }

			  _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */
          return this;
		  }


		  PoolFactory^ PoolFactory::SetSubscriptionAckInterval( TimeSpan ackInterval )
      {
			  _GF_MG_EXCEPTION_TRY2/* due to auto replace */

			  try
			  {
			    m_nativeptr->get()->setSubscriptionAckInterval( TimeUtils::TimeSpanToDurationCeil<std::chrono::milliseconds>(ackInterval) );
			  }
			  finally
			  {
			    GC::KeepAlive(m_nativeptr);
			  }

			  _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */
          return this;
		  }
      PoolFactory^ PoolFactory::SetThreadLocalConnections( Boolean enabled )
      {
			  _GF_MG_EXCEPTION_TRY2

			  try
			  {
			    m_nativeptr->get()->setThreadLocalConnections( enabled );
			  }
			  finally
			  {
			    GC::KeepAlive(m_nativeptr);
			  }

			  _GF_MG_EXCEPTION_CATCH_ALL2
          return this;
	  }

      PoolFactory^ PoolFactory::SetMultiuserAuthentication( bool multiuserAuthentication )
      {
			  _GF_MG_EXCEPTION_TRY2/* due to auto replace */

          try
          {
            m_nativeptr->get()->setMultiuserAuthentication( multiuserAuthentication );
          }
          finally
          {
            GC::KeepAlive(m_nativeptr);
          }

			  _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */
          return this;
	   }


		  PoolFactory^ PoolFactory::Reset()
      {
			  _GF_MG_EXCEPTION_TRY2/* due to auto replace */

			  try
			  {
			    m_nativeptr->get()->reset( );
			  }
			  finally
			  {
			    GC::KeepAlive(m_nativeptr);
			  }

			  _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */
          return this;
		  }


      Pool^ PoolFactory::Create(String^ name)
      {
        _GF_MG_EXCEPTION_TRY2/* due to auto replace */

          try
          {
            return Pool::Create(m_nativeptr->get()->create(marshal_as<std::string>(name)));
          }
          finally
          {
            GC::KeepAlive(m_nativeptr);
          }
        
        _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */
      }
    }  // namespace Client
  }  // namespace Geode
}  // namespace Apache
