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

#include "../begin_native.hpp"
#include "SerializationRegistry.hpp"
#include "../end_native.hpp"

#include "PdxType.hpp"
#include "PdxRemotePreservedData.hpp"
#include "../IPdxSerializable.hpp"
#include "WeakhashMap.hpp"
#include "EnumInfo.hpp"

using namespace System;
using namespace System::Threading;
using namespace System::Collections::Generic;

namespace Apache
{
  namespace Geode
  {
    namespace Client
    {

      namespace Internal
      {
        private ref class PdxTypeRegistry
        {
        public:

          PdxTypeRegistry(Cache^ cache) :m_cache(cache) {}

          //test hook;
          int testGetNumberOfPdxIds();

          //test hook
          int testNumberOfPreservedData();
          
          void AddPdxType(Int32 typeId, PdxType^ pdxType);
          
          PdxType^ GetPdxType(Int32 typeId);
          
          void AddLocalPdxType(String^ localType, PdxType^ pdxType);
          
          PdxType^ GetLocalPdxType(String^ localType);
          
          void SetMergedType(Int32 remoteTypeId, PdxType^ mergedType);
          
          PdxType^ GetMergedType(Int32 remoteTypeId);
          
          void SetPreserveData(IPdxSerializable^ obj, PdxRemotePreservedData^ preserveData);
          
          PdxRemotePreservedData^ GetPreserveData(IPdxSerializable^ obj);      

          void clear();

          Int32 GetPDXIdForType(Type^ type, native::Pool* pool, PdxType^ nType, bool checkIfThere);

          Int32 GetPDXIdForType(PdxType^ type, native::Pool*);

          Int32 GetEnumValue(EnumInfo^ ei);

          EnumInfo^ GetEnum(Int32 enumVal);

        private:
          Cache^ m_cache;

          Dictionary<Int32, PdxType^>^ typeIdToPdxType = gcnew Dictionary<Int32, PdxType^>();

          Dictionary<PdxType^, Int32>^ pdxTypeToTypeId = gcnew Dictionary<PdxType^, Int32>();

          Dictionary<Int32, PdxType^>^ remoteTypeIdToMergedPdxType = gcnew Dictionary<Int32, PdxType^>();

          Dictionary<String^, PdxType^>^ localTypeToPdxType = gcnew Dictionary<String^, PdxType^>();

          Dictionary<EnumInfo^, Int32>^ enumToInt = gcnew Dictionary<EnumInfo^, Int32>();

          Dictionary<Int32, EnumInfo^>^ intToEnum = gcnew Dictionary<Int32, EnumInfo^>();

          WeakHashMap^ preserveData = gcnew WeakHashMap();          

          ReaderWriterLock^ g_readerWriterLock = gcnew ReaderWriterLock();

          bool pdxIgnoreUnreadFields = false;
          bool pdxReadSerialized = false;
        };
      }
    }  // namespace Client
  }  // namespace Geode
}  // namespace Apache

