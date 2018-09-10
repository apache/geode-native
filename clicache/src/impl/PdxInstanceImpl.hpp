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


#include "../IPdxInstance.hpp"
#include "../IPdxSerializable.hpp"
#include "../DataInput.hpp"
#include "PdxLocalWriter.hpp"
#include "../IWritablePdxInstance.hpp"

namespace apache {
namespace geode {
namespace client {

  class CachePerfStats;

}  // namespace client
}  // nanespace geode
}  // namespace apache

namespace Apache
{
  namespace Geode
  {
    namespace Client
    {
      ref class Cache;

      namespace Internal
      {
        using namespace System;

        namespace native = apache::geode::client;

        ref class PdxType;

        /// <summary>
        /// Serialize the data in geode Portable Data eXchange(Pdx) Format.
        /// This format provides class versioning(forward and backward compability of types) in cache.
        /// This provides ability to query .NET domian objects.
        /// </summary>
        ref class PdxInstanceImpl : public IWritablePdxInstance, public IPdxInstance, public IPdxSerializable
        {
        private:
          static array<SByte>^ Boolean_DefaultBytes = gcnew array<SByte>{0};
          static array<SByte>^ Byte_DefaultBytes = gcnew array<SByte>{0};
          static array<SByte>^ Char_DefaultBytes = gcnew array<SByte>{0, 0};
          static array<SByte>^ Short_DefaultBytes = gcnew array<SByte>{0, 0};
          static array<SByte>^ Int_DefaultBytes = gcnew array<SByte>{0, 0, 0, 0};
          static array<SByte>^ Long_DefaultBytes = gcnew array<SByte>{0, 0, 0, 0, 0, 0, 0, 0};
          static array<SByte>^ Float_DefaultBytes = gcnew array<SByte>{0, 0, 0, 0};
          static array<SByte>^ Double_DefaultBytes = gcnew array<SByte>{0, 0, 0, 0, 0, 0, 0, 0};
          static array<SByte>^ Date_DefaultBytes = gcnew array<SByte>{-1, -1, -1, -1, -1, -1, -1, -1};
          static array<SByte>^ String_DefaultBytes = gcnew array<SByte>{static_cast<SByte>(apache::geode::client::internal::DSCode::CacheableNullString)};
          static array<SByte>^ Object_DefaultBytes = gcnew array<SByte>{static_cast<SByte>(apache::geode::client::internal::DSCode::NullObj)};
          static array<SByte>^ NULL_ARRAY_DefaultBytes = gcnew array<SByte>{-1};

          static PdxFieldType^ Default_PdxFieldType = gcnew PdxFieldType("default", "default", -1,
                                                                         -1/*field index*/,
                                                                         false, 1, -1/*var len field idx*/);

          bool hasDefaultBytes(PdxFieldType^ pField, DataInput^ dataInput, int start, int end);
          bool compareDefaulBytes(DataInput^ dataInput, int start, int end, array<SByte>^ defaultBytes);

          void cleanup();


          native::CachePerfStats* m_cachePerfStats;
          System::Byte* m_buffer;
          int m_bufferLength;
          int m_typeId;
          bool m_own;
          PdxType^ m_pdxType;
          Apache::Geode::Client::Cache^ m_cache;
        internal:
          Dictionary<String^, Object^>^ m_updatedFields;

          Object^ readField(DataInput^ dataInput, String^ fieldName, int typeId);

          bool checkType(Type^ fieldType, int typeId);

          void writeField(IPdxWriter^ writer, String^ fieldName, int typeId, Object^ value);

          int getOffset(DataInput^ dataInput, PdxType^ pt, int sequenceId);

          int getSerializedLength(DataInput^ dataInput, PdxType^ pt);

          void writeUnmodifieldField(DataInput^ dataInput, int startPos, int endPos, PdxLocalWriter^ localWriter);

          int getNextFieldPosition(DataInput^ dataInput, int fieldId, PdxType^ pt);

          IList<PdxFieldType^>^ getIdentityPdxFields(PdxType^ pt);

          bool isPrimitiveArray(Object^ object);

          int getRawHashCode(PdxType^ pt, PdxFieldType^ pField, DataInput^ dataInput);

          static int deepArrayHashCode(Object^ obj);

          generic <class T>where T:System::Collections::ICollection, System::Collections::IList, System::Collections::IEnumerable
          static int primitiveArrayHashCode(T objArray);

          static int enumerableHashCode(System::Collections::IEnumerable^ enumObj);

          static int enumerateDictionary(System::Collections::IDictionary^ iDict);

          void setOffsetForObject(DataInput^ dataInput, PdxType^ pt, int sequenceId);

          static int comparePdxField(PdxFieldType^ a, PdxFieldType^ b);

          void equatePdxFields(IList<PdxFieldType^>^ my, IList<PdxFieldType^>^ other);

          //bool compareRawBytes(PdxInstanceImpl^ other, PdxType^ myPT,  PdxFieldType^ myF, PdxType^ otherPT,  PdxFieldType^ otherF);
          bool compareRawBytes(PdxInstanceImpl^ other, PdxType^ myPT, PdxFieldType^ myF, DataInput^ myDataInput, PdxType^ otherPT, PdxFieldType^ otherF, DataInput^ otherDataInput);

          static bool enumerateDictionaryForEqual(System::Collections::IDictionary^ iDict, System::Collections::IDictionary^ otherIDict);

          static bool enumerableEquals(System::Collections::IEnumerable^ enumObj, System::Collections::IEnumerable^ enumOtherObj);

          static bool deepArrayEquals(Object^ obj, Object^ otherObj);

          void updatePdxStream(System::Byte* newPdxStream, int len);

        public:
          PdxInstanceImpl(System::Byte* buffer, int length, int typeId, bool own, Apache::Geode::Client::Cache^ cache)
          {
            m_buffer = buffer;
            m_bufferLength = length;
            m_typeId = typeId;
            m_updatedFields = nullptr;
            m_own = own;
            m_pdxType = nullptr;
            m_cache = cache;
          }

          //for pdxInstance factory
          PdxInstanceImpl(Dictionary<String^, Object^>^ fieldVsValue, PdxType^ pdxType, Cache^ cache);

          ~PdxInstanceImpl()
          {
            cleanup();
          }
          !PdxInstanceImpl()
          {
            cleanup();
          }

          PdxType^ getPdxType();

          void setPdxId(Int32 typeId);

          virtual String^ GetClassName();

          virtual Object^ GetObject();

          virtual bool HasField(String^ fieldName);

          virtual IList<String^>^ GetFieldNames();

          virtual bool IsIdentityField(String^ fieldName);

          virtual  Object^ GetField(String^ fieldName);

          virtual bool Equals(Object^ other) override;

          virtual int GetHashCode() override;

          virtual String^ ToString() override;

          virtual IWritablePdxInstance^ CreateWriter();

          virtual void SetField(String^ fieldName, Object^ value);

          virtual void ToData(IPdxWriter^ writer);

          virtual void FromData(IPdxReader^ reader);

        };
      }  // namespace Internal
    }  // namespace Client
  }  // namespace Geode
}  // namespace Apache
