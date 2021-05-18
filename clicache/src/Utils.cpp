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


#include "Utils.hpp"
#include "begin_native.hpp"
#include <Utils.hpp>
#include "end_native.hpp"

namespace Apache
{
  namespace Geode
  {
    namespace Client
    {

    MethodInfo^ Utils::LoadMethod( String^ assemblyName,
      String^ typeName, String^ methodName)
    {
      Type^ loadType;

      loadType = Type::GetType( typeName + ',' + assemblyName, false, true );
      if (loadType != nullptr)
      {
        return loadType->GetMethod( methodName, BindingFlags::Public |
          BindingFlags::Static | BindingFlags::IgnoreCase );
      }
      return nullptr;
    }

    MethodInfo^ Utils::LoadMethodFrom( String^ assemblyPath,
      String^ typeName, String^ methodName)
    {
      String^ assemblyName;
      Type^ loadType;

      assemblyName = System::IO::Path::GetFileNameWithoutExtension(
        assemblyPath );
      loadType = Type::GetType( typeName + ',' + assemblyName, false, true );
      if (loadType == nullptr)
      {
        Assembly^ assmb = Assembly::LoadFrom( assemblyPath );
        if (assmb != nullptr)
        {
          loadType = assmb->GetType(typeName, false, true);
        }
      }
      if (loadType != nullptr)
      {
        return loadType->GetMethod( methodName, BindingFlags::Public |
          BindingFlags::Static | BindingFlags::IgnoreCase );
      }
      return nullptr;
    }

    }  // namespace Client
  }  // namespace Geode
}  // namespace Apache

