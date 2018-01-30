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
#include <geode/CqServiceStatistics.hpp>
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
      /// Defines common statistical information for cqservice 
      /// </summary>
      public ref class CqServiceStatistics sealed
      {
      public:

        /// <summary>
        ///Get the number of CQs currently active. 
        ///Active CQs are those which are executing (in running state).
        /// </summary>
          System::UInt32 numCqsActive( );

        /// <summary>
        ///Get the total number of CQs created. This is a cumulative number.
        /// </summary>
          System::UInt32 numCqsCreated( );

        /// <summary>
        ///Get the total number of closed CQs. This is a cumulative number.
        /// </summary>
          System::UInt32 numCqsClosed( );

        /// <summary>
        ///Get the number of stopped CQs currently.
        /// </summary>
          System::UInt32 numCqsStopped( );

        /// <summary>
        ///Get number of CQs that are currently active or stopped. 
        ///The CQs included in this number are either running or stopped (suspended).
        ///Closed CQs are not included.
        /// </summary>
          System::UInt32 numCqsOnClient( );

      internal:

        /// <summary>
        /// Internal factory function to wrap a native object pointer inside
        /// this managed class with null pointer check.
        /// </summary>
        /// <param name="nativeptr">The native object pointer</param>
        /// <returns>
        /// The managed wrapper object; null if the native pointer is null.
        /// </returns>
        inline static CqServiceStatistics^ Create( std::shared_ptr<apache::geode::client::CqServiceStatistics> nativeptr )
        {
          return __nullptr == nativeptr ? nullptr :
            gcnew CqServiceStatistics( nativeptr );
        }


      private:

        /// <summary>
        /// Private constructor to wrap a native object pointer
        /// </summary>
        /// <param name="nativeptr">The native object pointer</param>
        inline CqServiceStatistics( std::shared_ptr<apache::geode::client::CqServiceStatistics> nativeptr )
        {
          m_nativeptr = gcnew native_shared_ptr<native::CqServiceStatistics>(nativeptr);
        }
        
        native_shared_ptr<native::CqServiceStatistics>^ m_nativeptr;
      };
    }  // namespace Client
  }  // namespace Geode
}  // namespace Apache

