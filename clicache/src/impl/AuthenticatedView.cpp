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



#include "../begin_native.hpp"
#include "CacheRegionHelper.hpp"
#include "CacheImpl.hpp"
#include "../end_native.hpp"

#include "../Cache.hpp"
#include "../DistributedSystem.hpp"
#include "../Region.hpp"
#include "../RegionAttributes.hpp"
#include "../QueryService.hpp"
#include "../FunctionService.hpp"
#include "../Execution.hpp"
#include "../String.hpp"
#include "AuthenticatedView.hpp"
#include "PdxInstanceFactoryImpl.hpp"

using namespace System;

namespace Apache
{
  namespace Geode
  {
    namespace Client
    {

      bool AuthenticatedView::IsClosed::get( )
      {
        try
        {
          return m_nativeptr->get()->isClosed( );
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }
      }

      void AuthenticatedView::Close( )
      {
        try {

          try
          {
            m_nativeptr->get()->close(  );
          }
          finally
          {
            GC::KeepAlive(m_nativeptr);
          }

        }
        catch (const apache::geode::client::Exception& ex) {
          throw Apache::Geode::Client::GeodeException::Get(ex);
        }
        catch (System::AccessViolationException^ ex) {
          throw ex;
        }
      }
      
			//TODO::split
      generic<class TKey, class TValue>
      IRegion<TKey, TValue>^ AuthenticatedView::GetRegion( String^ path )
      {
        try {

          try
          {
            auto nativeptr = m_nativeptr->get()->getRegion(to_utf8(path) );
            return Client::Region<TKey, TValue>::Create( nativeptr );
          }
          finally
          {
            GC::KeepAlive(m_nativeptr);
          }

        }
        catch (const apache::geode::client::Exception& ex) {
          throw Apache::Geode::Client::GeodeException::Get(ex);
        }
        catch (System::AccessViolationException^ ex) {
          throw ex;
        }
      }
      
      Client::QueryService^ AuthenticatedView::GetQueryService( )
      {
        try {

          try
          {
            return Client::QueryService::Create(m_nativeptr->get()->getQueryService( ));
          }
          finally
          {
            GC::KeepAlive(m_nativeptr);
          }

        }
        catch (const apache::geode::client::Exception& ex) {
          throw Apache::Geode::Client::GeodeException::Get(ex);
        }
        catch (System::AccessViolationException^ ex) {
          throw ex;
        }
      }

      generic<class TKey, class TValue>
      array<IRegion<TKey, TValue>^>^ AuthenticatedView::RootRegions( )
      {
        std::vector<std::shared_ptr<apache::geode::client::Region>> vrr;
        try
        {
          vrr = m_nativeptr->get()->rootRegions( );
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }
        auto rootRegions = gcnew array<IRegion<TKey, TValue>^>( static_cast<int>(vrr.size( )) );

        for( System::Int32 index = 0; index < vrr.size( ); index++ )
        {
          auto& nativeptr( vrr[ index ] );
          rootRegions[ index ] = Client::Region<TKey, TValue>::Create( nativeptr );
        }
        return rootRegions;
      }

      IPdxInstanceFactory^ AuthenticatedView::CreatePdxInstanceFactory(String^ className)
      {
        return gcnew Internal::PdxInstanceFactoryImpl(className, nullptr);
      }
    }  // namespace Client
  }  // namespace Geode
}  // namespace Apache
