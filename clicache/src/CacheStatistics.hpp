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
#include <geode/CacheStatistics.hpp>
#include "end_native.hpp"

#include "native_shared_ptr.hpp"


namespace Apache
{
  namespace Geode
  {
    namespace Client
    {
      namespace native = apache::geode::client;

      /// <summary>
      /// Defines common statistical information for both the region and its entries.
      /// </summary>
      /// <remarks>
      /// All of these methods may throw a <c>CacheClosedException</c>,
      /// <c>RegionDestroyedException</c>, or <c>EntryDestroyedException</c>.
      /// </remarks>
      /// <seealso cref="Region.Statistics" />
      /// <seealso cref="RegionEntry.Statistics" />
      public ref class CacheStatistics sealed
      {
      public:

        /// <summary>
        /// For an entry, returns the time that the entry's value was last modified.
        /// For a region, returns the last time any of the region's entries' values or
        /// the values in subregions' entries were modified.
        /// </summary>
        /// <remarks>
        /// <para>
        /// The modification may have been initiated locally, or it may have been
        /// an update distributed from another cache. It may also have been a new
        /// value provided by a loader. The modification time on a region is
        /// propagated upward to parent regions, transitively, to the root region.
        /// </para><para>
        /// Entry and subregion creation will update the modification time on a
        /// region, but <c>Region.Destroy</c>, <c>Region.DestroyRegion</c>,
        /// <c>Region.Invalidate</c>, and <c>Region.InvalidateRegion</c>
        /// do not update the modification time.
        /// </para>
        /// </remarks>
        /// <returns>
        /// the last modification time of the region or the entry;
        /// </returns>
        /// <seealso cref="Region.Put" />
        /// <seealso cref="Region.Get" />
        /// <seealso cref="Region.Create" />
        /// <seealso cref="Region.CreateSubRegion" />
        property System::DateTime LastModifiedTime
        {
          /// <summary>
          /// Get the last modified time of an entry or a region.
          /// </summary>
          System::DateTime get( );
        }

        /// <summary>
        /// For an entry, returns the last time it was accessed via <c>Region.Get</c>.
        /// For a region, returns the last time any of its entries or the entries of
        /// its subregions were accessed with <c>Region.Get</c>.
        /// </summary>
        /// <remarks>
        /// <para>
        /// Any modifications will also update the <c>LastAccessedTime</c>,
        /// so <c>LastAccessedTime</c> is always greater than or equal to
        /// <c>LastModifiedTime</c>. The <c>LastAccessedTime</c> on a region is
        /// propagated upward to parent regions, transitively, to the the root region.
        /// </para>
        /// </remarks>
        /// <returns>
        /// the last access time of the region or the entry's value;
        /// </returns>
        /// <seealso cref="Region.Get" />
        /// <seealso cref="LastModifiedTime" />
        property System::DateTime LastAccessedTime
        {
          /// <summary>
          /// Get the last accessed time of an entry or a region.
          /// </summary>
          System::DateTime get( );
        }


      internal:

        /// <summary>
        /// Internal factory function to wrap a native object pointer inside
        /// this managed class with null pointer check.
        /// </summary>
        /// <param name="nativeptr">The native object pointer</param>
        /// <returns>
        /// The managed wrapper object; null if the native pointer is null.
        /// </returns>
        inline static CacheStatistics^ Create( std::shared_ptr<apache::geode::client::CacheStatistics> nativeptr )
        {
          return __nullptr == nativeptr ? nullptr :
            gcnew CacheStatistics( nativeptr );
        }


      private:

        /// <summary>
        /// Private constructor to wrap a native object pointer
        /// </summary>
        /// <param name="nativeptr">The native object pointer</param>
        inline CacheStatistics( std::shared_ptr<apache::geode::client::CacheStatistics> nativeptr )
        {
           m_nativeptr = gcnew native_shared_ptr<native::CacheStatistics>(nativeptr);
        }
        native_shared_ptr<native::CacheStatistics>^ m_nativeptr;
      };
    }  // namespace Client
  }  // namespace Geode
}  // namespace Apache

