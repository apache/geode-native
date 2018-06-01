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
#include <geode/EntryEvent.hpp>
#include "end_native.hpp"

#include "IRegion.hpp"

using namespace System;

namespace Apache
{
  namespace Geode
  {
    namespace Client
    {
      namespace native = apache::geode::client;

      interface class ISerializable;

     // ref class Region;
      //interface class ICacheableKey;

      /// <summary>
      /// This class encapsulates events that occur for an entry in a region.
      /// </summary>
      generic<class TKey, class TValue>
      public ref class EntryEvent sealed
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
        /// Returns the key this event describes.
        /// </summary>
        property TKey Key
        {
          TKey get( );
        }

        /// <summary>
        /// Returns 'null' if there was no value in the cache. If the prior state
        ///  of the entry was invalid, or non-existent/destroyed, then the old
        /// value will be 'null'.
        /// </summary>
        property TValue OldValue
        {
          TValue get( );
        }

        /// <summary>
        /// Return the updated value from this event. If the event is a destroy
        /// or invalidate operation, then the new value will be NULL.
        /// </summary>
        property TValue NewValue
        {
          TValue get( );
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
        /// If the event originated in a remote process, returns true.
        /// </summary>
        property bool RemoteOrigin
        {
          bool get( );
        }

      internal:
        const native::EntryEvent* GetNative()
        {
          return m_nativeptr;
        }

        /// <summary>
        /// Private constructor to wrap a native object pointer
        /// </summary>
        /// <param name="nativeptr">The native object pointer</param>
        inline EntryEvent<TKey, TValue>( const native::EntryEvent* nativeptr )
          : m_nativeptr( nativeptr )
        {
        }
      private:
        const native::EntryEvent* m_nativeptr;
      };
    }  // namespace Client
  }  // namespace Geode
}  // namespace Apache

