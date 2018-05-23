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
#include <geode/RegionEvent.hpp>
#include "end_native.hpp"

#include "ISerializable.hpp"
#include "IRegion.hpp"
#include "Region.hpp"

using namespace System;

namespace Apache
{
  namespace Geode
  {
    namespace Client
    {
      namespace native = apache::geode::client;

      //ref class Region;

      /// <summary>
      /// This class encapsulates events that occur for a region.
      /// </summary>
      generic<class TKey, class TValue>
      public ref class RegionEvent sealed
      {
      public:

        /// <summary>
        /// Return the region this event occurred in.
        /// </summary>
        property IRegion<TKey, TValue>^ Region
        {
          IRegion<TKey, TValue>^ get( );
        }

        /// <summary>
        /// Returns the callbackArgument passed to the method that generated
        /// this event. See the <see cref="Region" /> interface methods
        /// that take a callbackArgument parameter.
        /// </summary>
        property Object^ CallbackArgument
        {
          Object^ get();
        }

        /// <summary>
        /// Returns true if the event originated in a remote process.
        /// </summary>
        property bool RemoteOrigin
        {
          bool get( );
        }


      internal:

        const native::RegionEvent* GetNative()
        {
          return m_nativeptr;
        }

        /// <summary>
        /// Internal constructor to wrap a native object pointer
        /// </summary>
        /// <param name="nativeptr">The native object pointer</param>
        inline Apache::Geode::Client::RegionEvent<TKey, TValue>( const native::RegionEvent* nativeptr )
          : m_nativeptr( nativeptr )
        {
        }

      private:
        const native::RegionEvent* m_nativeptr;
      };
    }  // namespace Client
  }  // namespace Geode
}  // namespace Apache
 //namespace 
