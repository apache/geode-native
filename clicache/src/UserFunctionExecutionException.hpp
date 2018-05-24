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
#include <geode/UserFunctionExecutionException.hpp>
#include "end_native.hpp"

#include "native_shared_ptr.hpp"
#include "ISerializable.hpp"
#include "DataInput.hpp"
#include "DataOutput.hpp"

using namespace System;

namespace Apache
{
  namespace Geode
  {
    namespace Client
    {
      namespace native = apache::geode::client;

      /// <summary>
      /// UserFunctionExecutionException class is used to encapsulate geode sendException in case of Function execution. 
      /// </summary>
      public ref class UserFunctionExecutionException sealed
        : public ISerializable
      {
      public:
        // ISerializable members

        virtual property System::UInt64 ObjectSize
        {
          virtual System::UInt64 get();
        }

        // End: ISerializable members   

        /// <summary>
        /// return as String the Exception message returned from geode sendException api.          
        /// </summary>
        /// <returns>the String Exception Message</returns>
        property String^ Message
        {
          String^ get();
        }

        /// <summary>
        /// return as String the Exception name returned from geode sendException api.          
        /// </summary>
        /// <returns>the String Exception Name</returns>
        property String^ Name
        {
          String^ get();
        }

      internal:

        /// <summary>
        /// Private constructor to wrap a native object pointer.
        /// </summary>
        /// <param name="nativeptr">The native object pointer</param>
        inline UserFunctionExecutionException(std::shared_ptr<apache::geode::client::UserFunctionExecutionException> nativeptr)
				{
          m_nativeptr = gcnew native_shared_ptr<native::UserFunctionExecutionException>(nativeptr);
        }
        
        native_shared_ptr<native::UserFunctionExecutionException>^ m_nativeptr;   
      };
    }  // namespace Client
  }  // namespace Geode
}  // namespace Apache


