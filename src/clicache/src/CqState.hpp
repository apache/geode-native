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
#include <geode/CqState.hpp>
#include "end_native.hpp"



using namespace System;

namespace Apache
{
  namespace Geode
  {
    namespace Client
    {
      namespace native = apache::geode::client;

      /// <summary>
      /// Enumerated type for cq state
      /// @nativeclient
      /// For Native Clients:
      /// @endnativeclient      
      /// </summary>
      public enum class CqStateType
      {
        STOPPED = 0,
        RUNNING,
        CLOSED,
        CLOSING,
        INVALID
      };


      /// <summary>
      /// Static class containing convenience methods for <c>CqState</c>.
      /// </summary>
      public ref class CqState sealed
      {
      public:

        /// <summary>
        /// Returns the state in string form.
        /// </summary>
        virtual String^ ToString( ) override;

        /// <summary>
        /// Returns true if the CQ is in Running state.
        /// </summary>
        bool IsRunning(); 

        /// <summary>
        /// Returns true if the CQ is in Stopped state.
	      /// </summary>
        bool IsStopped();

        /// <summary>
        /// Returns true if the CQ is in Closed state.
        /// </summary>
        bool IsClosed(); 

        /// <summary>
        /// Returns true if the CQ is in Closing state.
	      /// </summary>
        bool IsClosing();
	      void SetState(CqStateType state);
	      CqStateType GetState();
  
      internal:

        /// <summary>
        /// Internal constructor to wrap a native object pointer
        /// </summary>
        /// <param name="nativeptr">The native object pointer</param>
        inline CqState( native::CqState* nativeptr )
          : m_nativeptr(nativeptr)
        {
        }
              
      private:
        native::CqState* m_nativeptr;
      };
    }  // namespace Client
  }  // namespace Geode
}  // namespace Apache

