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

#include "Pool.hpp"
#include "PoolFactory.hpp"

#include "impl/ManagedString.hpp"
#include "ExceptionTypes.hpp"

#include "Cache.hpp"

using namespace System;

namespace Apache
{
  namespace Geode
  {
    namespace Client
    {


      PoolFactory^ PoolFactory::SetFreeConnectionTimeout( Int32 connectionTimeout )
		  {
			  _GF_MG_EXCEPTION_TRY2/* due to auto replace */

			  try
			  {
			    m_nativeptr->get()->setFreeConnectionTimeout( connectionTimeout );
			  }
			  finally
			  {
			    GC::KeepAlive(m_nativeptr);
			  }

			  _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */
          return this;
		  }


		  PoolFactory^ PoolFactory::SetLoadConditioningInterval( Int32 loadConditioningInterval )
		  {
			  _GF_MG_EXCEPTION_TRY2/* due to auto replace */

			  try
			  {
			    m_nativeptr->get()->setLoadConditioningInterval( loadConditioningInterval );
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


		  PoolFactory^ PoolFactory::SetReadTimeout( Int32 timeout )
      {
			  _GF_MG_EXCEPTION_TRY2/* due to auto replace */

			  try
			  {
			    m_nativeptr->get()->setReadTimeout( timeout );
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


		  PoolFactory^ PoolFactory::SetIdleTimeout( Int32 idleTimeout )
      {
			  _GF_MG_EXCEPTION_TRY2/* due to auto replace */

			  try
			  {
			    m_nativeptr->get()->setIdleTimeout( idleTimeout );
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


		  PoolFactory^ PoolFactory::SetPingInterval( Int32 pingInterval )
      {
			  _GF_MG_EXCEPTION_TRY2/* due to auto replace */

			  try
			  {
			    m_nativeptr->get()->setPingInterval( pingInterval );
			  }
			  finally
			  {
			    GC::KeepAlive(m_nativeptr);
			  }

			  _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */
          return this;
		  }


		  PoolFactory^ PoolFactory::SetUpdateLocatorListInterval( Int32 updateLocatorListInterval )
      {
			  _GF_MG_EXCEPTION_TRY2/* due to auto replace */

			  try
			  {
			    m_nativeptr->get()->setUpdateLocatorListInterval( updateLocatorListInterval );
			  }
			  finally
			  {
			    GC::KeepAlive(m_nativeptr);
			  }

			  _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */
          return this;
		  }


      PoolFactory^ PoolFactory::SetStatisticInterval( Int32 statisticInterval )
      {
			  _GF_MG_EXCEPTION_TRY2/* due to auto replace */

			  try
			  {
			    m_nativeptr->get()->setStatisticInterval( statisticInterval );
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

        ManagedString mg_servergroup( group );
			  try
			  {
			    m_nativeptr->get()->setServerGroup( mg_servergroup.CharPtr );
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

        ManagedString mg_host( host );
			  try
			  {
			    m_nativeptr->get()->addLocator( mg_host.CharPtr, port );
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

			  ManagedString mg_host( host );
			  try
			  {
			    m_nativeptr->get()->addServer( mg_host.CharPtr, port );
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


		  PoolFactory^ PoolFactory::SetSubscriptionMessageTrackingTimeout( Int32 messageTrackingTimeout )
      {
			  _GF_MG_EXCEPTION_TRY2/* due to auto replace */

			  try
			  {
			    m_nativeptr->get()->setSubscriptionMessageTrackingTimeout( messageTrackingTimeout );
			  }
			  finally
			  {
			    GC::KeepAlive(m_nativeptr);
			  }

			  _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */
          return this;
		  }


		  PoolFactory^ PoolFactory::SetSubscriptionAckInterval( Int32 ackInterval )
      {
			  _GF_MG_EXCEPTION_TRY2/* due to auto replace */

			  try
			  {
			    m_nativeptr->get()->setSubscriptionAckInterval( ackInterval );
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


      Pool^ PoolFactory::Create(String^ name, Cache^ cache)
      {
        _GF_MG_EXCEPTION_TRY2/* due to auto replace */

          ManagedString mg_name(name);
          try
          {
            return Pool::Create(m_nativeptr->get()->create(mg_name.CharPtr));
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
