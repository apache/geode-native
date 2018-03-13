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


#include "TypeRegistry.hpp"

namespace Apache
{
  namespace Geode
  {
    namespace Client
    {

      String^ TypeRegistry::GetPdxTypeName(String^ localTypeName)
      {
        if (pdxTypeMapper == nullptr)
        {
          return localTypeName;
        }

        String^ pdxTypeName;
        if (localTypeNameToPdx->TryGetValue(localTypeName, pdxTypeName)) {
          return pdxTypeName;
        }


        pdxTypeName = pdxTypeMapper->ToPdxTypeName(localTypeName);
        if (pdxTypeName == nullptr)
        {
          throw gcnew IllegalStateException("PdxTypeName should not be null for local type " + localTypeName);
        }

        localTypeNameToPdx[localTypeName] = pdxTypeName;
        pdxTypeNameToLocal[pdxTypeName] = localTypeName;

        return pdxTypeName;
      }

      String^ TypeRegistry::GetLocalTypeName(String^ pdxTypeName)
      {
        if (pdxTypeMapper == nullptr)
        {
          return pdxTypeName;
        }

        String^ localTypeName;
        if (pdxTypeNameToLocal->TryGetValue(pdxTypeName, localTypeName))
        {
          return localTypeName;
        }

        localTypeName = pdxTypeMapper->FromPdxTypeName(pdxTypeName);
        if (localTypeName == nullptr)
        {
          throw gcnew IllegalStateException("LocalTypeName should not be null for pdx type " + pdxTypeName);
        }

        localTypeNameToPdx[localTypeName] = pdxTypeName;
        pdxTypeNameToLocal[pdxTypeName] = localTypeName;

        return localTypeName;
      }

      Type^ TypeRegistry::GetType(String^ className)
      {
        Type^ retVal = nullptr;

        if (classNameVsType->TryGetValue(className, retVal)) {
          return retVal;
        }

        Dictionary<Assembly^, bool>^ referedAssembly = gcnew Dictionary<Assembly^, bool>();
        AppDomain^ MyDomain = AppDomain::CurrentDomain;
        array<Assembly^>^ AssembliesLoaded = MyDomain->GetAssemblies();
        for each(Assembly^ assembly in AssembliesLoaded)
        {
          retVal = GetTypeFromRefrencedAssemblies(className, referedAssembly, assembly);
          if (retVal) {
            return retVal;
          }
        }
        
        return retVal;
      }

      Type^ TypeRegistry::GetTypeFromRefrencedAssemblies(String^ className, Dictionary<Assembly^, bool>^ referedAssembly, Assembly^ currentAssembly)
      {
        Type^ type = currentAssembly->GetType(className);
        if (type != nullptr)
        {
          classNameVsType[className] = type;
          return type;
        }

        //if (referedAssembly->ContainsKey(currentAssembly))
        //  return nullptr;
        //referedAssembly[currentAssembly] = true;

        ////get all refrenced assembly
        //array<AssemblyName^>^ ReferencedAssemblies = currentAssembly->GetReferencedAssemblies();
        //for each(AssemblyName^ assembly in ReferencedAssemblies)
        //{
        //  try
        //  {
        //    Assembly^ loadedAssembly = Assembly::Load(assembly);
        //    if (loadedAssembly != nullptr && (!referedAssembly->ContainsKey(loadedAssembly)))
        //    {
        //      type = GetTypeFromRefrencedAssemblies(className, referedAssembly, loadedAssembly);
        //      if (!type)
        //        return type;
        //    }
        //  }
        //  catch (System::Exception^){//ignore
        //  }
        //}
        return nullptr;
      }

    
    }  // namespace Client
  }  // namespace Geode
}  // namespace Apache
