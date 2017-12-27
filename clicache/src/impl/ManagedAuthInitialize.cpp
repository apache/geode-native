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


#include <string>

#include "ManagedAuthInitialize.hpp"
#include "../IAuthInitialize.hpp"
#include "../ExceptionTypes.hpp"
#include "../Properties.hpp"


using namespace System;
using namespace System::Text;
using namespace System::Reflection;

namespace apache
{
  namespace geode
  {
    namespace client
    {
      using namespace msclr::interop;

      std::shared_ptr<Properties> ManagedAuthInitializeGeneric::getCredentials(const std::shared_ptr<Properties>&
                                                                 securityprops, const std::string& server)
      {
        try {
          auto mprops = Apache::Geode::Client::Properties<String^, String^>::Create(securityprops);

          return m_getCredentials->Invoke(mprops, marshal_as<String^>(server))->GetNative();
        }
        catch (Apache::Geode::Client::GeodeException^ ex) {
          ex->ThrowNative();
        }
        catch (System::Exception^ ex) {
          Apache::Geode::Client::GeodeException::ThrowNative(ex);
        }
        return nullptr;
      }

      void ManagedAuthInitializeGeneric::close()
      {
        try {
          m_close->Invoke();
        }
        catch (Apache::Geode::Client::GeodeException^ ex) {
          ex->ThrowNative();
        }
        catch (System::Exception^ ex) {
          Apache::Geode::Client::GeodeException::ThrowNative(ex);
        }
      }

    }  // namespace client
  }  // namespace geode
}  // namespace apache
