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
#include <geode/CqStatistics.hpp>
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
      /// Defines common statistical information for a cq.
      /// </summary>
      public ref class CqStatistics sealed
      {
      public:

        /// <summary>
        /// get number of inserts qualified by this Cq
        /// </summary>
          System::UInt32 numInserts( );

        /// <summary>
        /// get number of deletes qualified by this Cq
        /// </summary>
          System::UInt32 numDeletes( );

        /// <summary>
        /// get number of updates qualified by this Cq
        /// </summary>
          System::UInt32 numUpdates( );

        /// <summary>
        /// get number of events qualified by this Cq
        /// </summary>
          System::UInt32 numEvents( );

      internal:

        /// <summary>
        /// Internal factory function to wrap a native object pointer inside
        /// this managed class with null pointer check.
        /// </summary>
        /// <param name="nativeptr">The native object pointer</param>
        /// <returns>
        /// The managed wrapper object; null if the native pointer is null.
        /// </returns>
        inline static CqStatistics^ Create( std::shared_ptr<apache::geode::client::CqStatistics> nativeptr )
        {
          return __nullptr == nativeptr ? nullptr :
            gcnew CqStatistics( nativeptr );
        }


      private:

        /// <summary>
        /// Private constructor to wrap a native object pointer
        /// </summary>
        /// <param name="nativeptr">The native object pointer</param>
        inline CqStatistics( std::shared_ptr<apache::geode::client::CqStatistics> nativeptr )
        {
          m_nativeptr = gcnew native_shared_ptr<native::CqStatistics>(nativeptr);
        }

        native_shared_ptr<native::CqStatistics>^ m_nativeptr;

      };
    }  // namespace Client
  }  // namespace Geode
}  // namespace Apache

