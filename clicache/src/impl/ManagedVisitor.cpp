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

//#include "../../../../geode_includes.hpp"
#include "ManagedVisitor.hpp"
#include "SafeConvert.hpp"
#include "../ExceptionTypes.hpp"


using namespace System;

namespace apache
{
  namespace geode
  {
    namespace client
    {

      void ManagedVisitorGeneric::visit(const std::shared_ptr<native::CacheableKey>& key, const std::shared_ptr<native::Cacheable>& value)
      {
        using namespace Apache::Geode::Client;
        try {
          ICacheableKey^ mg_key(SafeGenericUMKeyConvert<ICacheableKey^>(key));
          IGeodeSerializable^ mg_value(SafeUMSerializableConvertGeneric(value));

          m_visitor->Invoke(mg_key, (Apache::Geode::Client::IGeodeSerializable^)mg_value);
        }
        catch (GeodeException^ ex) {
          ex->ThrowNative();
        }
        catch (System::Exception^ ex) {
          GeodeException::ThrowNative(ex);
        }
      }

    }  // namespace client
  }  // namespace geode
}  // namespace apache
