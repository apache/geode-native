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


#include "begin_native.hpp"
#include <geode/RegionService.hpp>
#include "end_native.hpp"

#include "FunctionService.hpp"
#include "Pool.hpp"
#include "Region.hpp"
#include "Execution.hpp"

#include "impl/AuthenticatedView.hpp"

using namespace System;

namespace Apache
{
  namespace Geode
  {
    namespace Client
    {
      namespace native = apache::geode::client;

      generic <class TResult>
      generic <class TKey, class TValue>
      Execution<TResult>^ FunctionService<TResult>::OnRegion( IRegion<TKey, TValue>^ rg )
      {
        try {
          
          auto nativeRegion = ((Region<TKey, TValue>^)rg)->GetNative();
          auto execution = native::FunctionService::onRegion(nativeRegion);
          return Execution<TResult>::Create( std::move(execution), nullptr );

        }
        catch (const apache::geode::client::Exception& ex) {
          throw Apache::Geode::Client::GeodeException::Get(ex);
        }
        catch (System::AccessViolationException^ ex) {
          throw ex;
        }
      }

      generic <class TResult>
      Execution<TResult>^ FunctionService<TResult>::OnServer( Pool^ pl )
      {
        try {

          auto nativeptr = native::FunctionService::onServer(pl->GetNative());
          return Execution<TResult>::Create(std::move(nativeptr) , nullptr);

        }
        catch (const apache::geode::client::Exception& ex) {
          throw Apache::Geode::Client::GeodeException::Get(ex);
        }
        catch (System::AccessViolationException^ ex) {
          throw ex;
        }
      }
      
      generic <class TResult>
      Execution<TResult>^ FunctionService<TResult>::OnServers( Pool^ pl )
      {
        try {

          auto nativeptr = native::FunctionService::onServers(pl->GetNative());
          return Execution<TResult>::Create(std::move(nativeptr) , nullptr);

        }
        catch (const apache::geode::client::Exception& ex) {
          throw Apache::Geode::Client::GeodeException::Get(ex);
        }
        catch (System::AccessViolationException^ ex) {
          throw ex;
        }
      }

      generic<class TResult>
      Execution<TResult>^ FunctionService<TResult>::OnServer( IRegionService^ cache )
      {
        try {

          if(auto realCache = dynamic_cast<Cache^>(cache))
          {
            auto nativeptr = native::FunctionService::onServer(*realCache->GetNative());
            return Execution<TResult>::Create(std::move(nativeptr), nullptr );
          }
          else
          {
            auto authCache = dynamic_cast<AuthenticatedView^>(cache);
            auto nativeptr = native::FunctionService::onServer(authCache->GetNative());
            return Execution<TResult>::Create(std::move(nativeptr), nullptr );
          }

        }
        catch (const apache::geode::client::Exception& ex) {
          throw Apache::Geode::Client::GeodeException::Get(ex);
        }
        catch (System::AccessViolationException^ ex) {
          throw ex;
        }
      }

      generic<class TResult>
      Execution<TResult>^ FunctionService<TResult>::OnServers( IRegionService^ cache )
      {
        try {

          if(auto realCache = dynamic_cast<Cache^>(cache))
          {
            auto nativeptr = native::FunctionService::onServers(*realCache->GetNative());
            return Execution<TResult>::Create(std::move(nativeptr), nullptr );
          }
          else
          {
            auto authCache = dynamic_cast<AuthenticatedView^>(cache);
            auto nativeptr = native::FunctionService::onServers(authCache->GetNative());
            return Execution<TResult>::Create(std::move(nativeptr), nullptr );
          }

        }
        catch (const apache::geode::client::Exception& ex) {
          throw Apache::Geode::Client::GeodeException::Get(ex);
        }
        catch (System::AccessViolationException^ ex) {
          throw ex;
        }
      }
    }  // namespace Client
  }  // namespace Geode
}  // namespace Apache
