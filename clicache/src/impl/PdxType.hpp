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

#include "PdxFieldType.hpp"

#include "../begin_native.hpp"
#include <PdxFieldType.hpp>
#include "../end_native.hpp"

using namespace System;
using namespace System::Collections::Generic;

namespace Apache
{
  namespace Geode
  {
    namespace Client
    {

      ref class DataOutput;
      ref class DataInput;

      namespace Internal
      {
        private ref class PdxType : public IDataSerializableInternal
        {
        private:
          Object^                 m_lockObj;
          static const String^    m_javaPdxClass = "org.apache.geode.pdx.internal.PdxType";
          IList<PdxFieldType^>^   m_pdxFieldTypes;
          IList<PdxType^>^        m_otherVersions;
          Int32                   m_cachedHashcode;

          //Type^                 m_pdxDomainType;
          String^                 m_className;
          Int32                   m_geodeTypeId;
          bool                    m_isLocal;
          Int32                   m_numberOfVarLenFields;
          Int32                   m_varLenFieldIdx;

          Int32                   m_numberOfFieldsExtra;

          array<Int32>^           m_remoteToLocalFieldMap;
          array<Int32>^           m_localToRemoteFieldMap;

          array<Int32, 2>^        m_positionMap;
          IDictionary<String^, PdxFieldType^>^ m_fieldNameVsPdxType;

          bool                   m_isVarLenFieldAdded;
          bool									 m_noJavaClass;

          void initRemoteToLocal(Cache^ cache);
          void initLocalToRemote(Cache^ cache);
          //first has more fields than second
          PdxType^ isContains(PdxType^ first, PdxType^ second);
          PdxType^ clone();

          void generatePositionMap();
          Int32 variableLengthFieldPosition(PdxFieldType^ varLenField, System::Byte* offsetPosition, Int32 offsetSize, Int32 pdxStreamlen);
          Int32 fixedLengthFieldPosition(PdxFieldType^ fixLenField, System::Byte* offsetPosition, Int32 offsetSize, Int32 pdxStreamlen);

          PdxType^ isLocalTypeContains(PdxType^ otherType);
          PdxType^ isRemoteTypeContains(PdxType^ localType);
        public:
          PdxType()
          {
            m_cachedHashcode = 0;
            m_lockObj = gcnew Object();
            m_pdxFieldTypes = gcnew List<PdxFieldType^>();
            m_otherVersions = gcnew List<PdxType^>();
            m_isLocal = false;
            m_numberOfVarLenFields = 0;
            m_varLenFieldIdx = 0;//start with 0
            m_isVarLenFieldAdded = false;
            m_fieldNameVsPdxType = gcnew Dictionary<String^, PdxFieldType^>();
            m_noJavaClass = false;
            m_geodeTypeId = 0;
            // m_pdxDomainType = nullptr;
          }

          PdxType(String^ pdxDomainClassName,
                  bool  isLocal)
          {
            m_cachedHashcode = 0;
            m_lockObj = gcnew Object();
            m_pdxFieldTypes = gcnew List<PdxFieldType^>();
            m_otherVersions = gcnew List<PdxType^>();
            //  m_className = className;
            // m_pdxDomainType = pdxDomainType;
            m_className = pdxDomainClassName;
            m_isLocal = isLocal;
            m_numberOfVarLenFields = 0;
            m_varLenFieldIdx = 0;//start with 0
            m_isVarLenFieldAdded = false;
            m_fieldNameVsPdxType = gcnew Dictionary<String^, PdxFieldType^>();
            m_noJavaClass = false;
            m_geodeTypeId = 0;
          }

          static ISerializable^ CreateDeserializable()
          {
            return gcnew PdxType();
          }
          property Int32 TotalVarLenFields
          {
            Int32 get() { return m_numberOfVarLenFields; };
          }
          property Int32 Totalfields
          {
            Int32 get() { return m_pdxFieldTypes->Count; };
          }
          property IList<PdxFieldType^>^ PdxFieldList
          {
            IList<PdxFieldType^>^ get(){ return m_pdxFieldTypes; }
          }
          property Int32 TypeId
          {
            Int32 get() { return m_geodeTypeId; }
            void set(Int32 value) { m_geodeTypeId = value; }
          }

          property String^ PdxClassName
          {
            String^ get() { return m_className; }
            void set(String^ className) { m_className = className; }
          }
          property Int32 NumberOfFieldsExtra
          {
            Int32 get(){ return m_numberOfFieldsExtra; }
          }

          PdxFieldType^ GetPdxField(String^ fieldName)
          {
            PdxFieldType^ retVal = nullptr;

            m_fieldNameVsPdxType->TryGetValue(fieldName, retVal);

            return retVal;
          }
          void AddOtherVersion(PdxType^ otherVersion)
          {
            m_otherVersions->Add(otherVersion);
          }

          array<int>^ GetLocalToRemoteMap(Cache^ cache);
          array<int>^ GetRemoteToLocalMap(Cache^ cache);
          property Int32 NumberOfVarLenFields
          {
            Int32 get(){ return m_numberOfVarLenFields; }
          }
          property bool IsLocal
          {
            bool get() { return m_isLocal; }
            void set(bool val) { m_isLocal = val; }
          }
          virtual void ToData(DataOutput^ output);
          virtual void FromData(DataInput^ input);
          virtual property System::UInt64 ObjectSize
          {
            System::UInt64 get(){ return 0; }
          }
          virtual String^ ToString() override
          {
            return "PdxType";
          }
          void AddFixedLengthTypeField(String^ fieldName, String^ className, PdxFieldTypes typeId, Int32 size);
          void AddVariableLengthTypeField(String^ fieldName, String^ className, PdxFieldTypes typeId);
          void InitializeType(Cache^ cache);
          PdxType^ MergeVersion(PdxType^ otherVersion);
          Int32 GetFieldPosition(String^ fieldName, System::Byte* offsetPosition, Int32 offsetSize, Int32 pdxStreamlen);
          Int32 GetFieldPosition(Int32 fieldIdx, System::Byte* offsetPosition, Int32 offsetSize, Int32 pdxStreamlen);

          virtual bool Equals(Object^ otherType) override;
          virtual Int32 GetHashCode() override;
        };
      }  // namespace Client
    }  // namespace Geode
  }  // namespace Apache

}
