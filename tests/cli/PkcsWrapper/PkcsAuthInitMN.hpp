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

#include <memory>
#include "native_shared_ptr.hpp"
#include "PkcsAuthInit.hpp"
//#include "IAuthInitialize.hpp"

using namespace System;

using namespace Apache::Geode::Client;

namespace Apache
{
  namespace Geode
  {
    namespace Client
    {
      namespace Tests
      {
        public ref class PkcsAuthInit sealed
          : public Apache::Geode::Client::IAuthInitialize
        {
        public:

          PkcsAuthInit();

          ~PkcsAuthInit();

          //generic <class TPropKey, class TPropValue>
          virtual Apache::Geode::Client::Properties<String^, Object^> ^
            GetCredentials(
            Apache::Geode::Client::Properties<String^, String^>^ props, String^ server);

          virtual void Close();

        internal:
          PkcsAuthInit(const std::shared_ptr<apache::geode::client::PKCSAuthInitInternal>& nativeptr)
          {
            m_nativeptr = gcnew native_shared_ptr<apache::geode::client::PKCSAuthInitInternal>(nativeptr);
          }

        private:
          native_shared_ptr<apache::geode::client::PKCSAuthInitInternal>^ m_nativeptr;
        };
      }
    }
  }
}

