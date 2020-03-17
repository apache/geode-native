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


#include "begin_native.hpp"
#include "CacheRegionHelper.hpp"
#include "CacheImpl.hpp"
#include "DataOutputInternal.hpp"
#include "end_native.hpp"


#include "DataOutput.hpp"
#include "IDataSerializable.hpp"
#include "ISerializable.hpp"
#include "CacheableObjectArray.hpp"
#include "impl/PdxHelper.hpp"
#include "impl/PdxWrapper.hpp"
#include "Cache.hpp"

using namespace System;
using namespace System::Runtime::InteropServices;
using namespace apache::geode::client;

namespace Apache
{
  namespace Geode
  {
    namespace Client
    {
      using namespace msclr::interop;

      DataOutput::DataOutput(Apache::Geode::Client::Cache^ cache)
      { 
        m_cache = cache;
        m_nativeptr = gcnew native_conditional_unique_ptr<native::DataOutput>(
          std::make_unique<native::DataOutput>(cache->GetNative()->createDataOutput()));
        m_isManagedObject = true;
        m_cursor = 0;
        try
        {
          m_bytes = const_cast<System::Byte *>(m_nativeptr->get()->getCursor());
          m_remainingBufferLength = (System::Int32)m_nativeptr->get()->getRemainingBufferLength();
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }
        m_ispdxSerialization = false;
      }

      void DataOutput::WriteByte(Byte value)
      {
        EnsureCapacity(1);
        m_bytes[m_cursor++] = value;
      }

      void DataOutput::WriteSByte(SByte value)
      {
        EnsureCapacity(1);
        m_bytes[m_cursor++] = value;
      }

      void DataOutput::WriteBoolean(bool value)
      {
        EnsureCapacity(1);
        if (value)
          m_bytes[m_cursor++] = 0x01;
        else
          m_bytes[m_cursor++] = 0x00;
      }

      void DataOutput::WriteChar(Char value)
      {
        EnsureCapacity(2);
        m_bytes[m_cursor++] = (System::Byte)(value >> 8);
        m_bytes[m_cursor++] = (System::Byte)value;
      }

      void DataOutput::WriteBytes(array<Byte>^ bytes, System::Int32 len)
      {
        if (bytes != nullptr && bytes->Length >= 0)
        {
          if (len >= 0 && len <= bytes->Length)
          {
            WriteArrayLen(len);
            EnsureCapacity(len);
            for (int i = 0; i < len; i++)
              m_bytes[m_cursor++] = bytes[i];
          }
          else
          {
            throw gcnew IllegalArgumentException("DataOutput::WriteBytes argument len is not in byte array range.");
          }
        }
        else
        {
          WriteByte(0xFF);
        }
      }

      void DataOutput::WriteArrayLen(System::Int32 len)
      {
        if (len == -1) {//0xff
          WriteByte(0xFF);
        }
        else if (len <= 252) { // 252 is java's ((byte)-4 && 0xFF) or 0xfc
          WriteByte((Byte)(len));
        }
        else if (len <= 0xFFFF) {
          WriteByte(0xFE);//0xfe
          WriteUInt16((len));
        }
        else {
          WriteByte((0xFD));//0xfd
          WriteUInt32(len);
        }
      }

      void DataOutput::WriteSBytes(array<SByte>^ bytes, System::Int32 len)
      {
        if (bytes != nullptr && bytes->Length >= 0)
        {
          if (len >= 0 && len <= bytes->Length)
          {
            WriteArrayLen(len);
            EnsureCapacity(len);
            for (int i = 0; i < len; i++)
              m_bytes[m_cursor++] = bytes[i];
          }
          else
          {
            throw gcnew IllegalArgumentException("DataOutput::WriteSBytes argument len is not in SByte array range.");
          }
        }
        else
        {
          WriteByte(0xFF);
        }
      }

      void DataOutput::WriteBytesOnly(array<Byte>^ bytes, System::UInt32 len)
      {
        WriteBytesOnly(bytes, len, 0);
      }

      void DataOutput::WriteBytesOnly(array<Byte>^ bytes, System::UInt32 len, System::UInt32 offset)
      {
        if (bytes != nullptr)
        {
          if (len >= 0 && len <= ((System::UInt32)bytes->Length - offset))
          {
            EnsureCapacity(len);
            for (System::UInt32 i = 0; i < len; i++)
              m_bytes[m_cursor++] = bytes[offset + i];
          }
          else
          {
            throw gcnew IllegalArgumentException("DataOutput::WriteBytesOnly argument len is not in Byte array range.");
          }
        }
      }

      void DataOutput::WriteSBytesOnly(array<SByte>^ bytes, System::UInt32 len)
      {
        if (bytes != nullptr)
        {
          if (len >= 0 && len <= (System::UInt32)bytes->Length)
          {
            EnsureCapacity(len);
            for (System::UInt32 i = 0; i < len; i++)
              m_bytes[m_cursor++] = bytes[i];
          }
          else
          {
            throw gcnew IllegalArgumentException("DataOutput::WriteSBytesOnly argument len is not in SByte array range.");
          }
        }
      }

      void DataOutput::WriteUInt16(System::UInt16 value)
      {
        EnsureCapacity(2);
        m_bytes[m_cursor++] = (System::Byte)(value >> 8);
        m_bytes[m_cursor++] = (System::Byte)value;
      }

      void DataOutput::WriteUInt32(System::UInt32 value)
      {
        EnsureCapacity(4);
        m_bytes[m_cursor++] = (System::Byte)(value >> 24);
        m_bytes[m_cursor++] = (System::Byte)(value >> 16);
        m_bytes[m_cursor++] = (System::Byte)(value >> 8);
        m_bytes[m_cursor++] = (System::Byte)value;
      }

      void DataOutput::WriteUInt64(System::UInt64 value)
      {
        EnsureCapacity(8);
        m_bytes[m_cursor++] = (System::Byte)(value >> 56);
        m_bytes[m_cursor++] = (System::Byte)(value >> 48);
        m_bytes[m_cursor++] = (System::Byte)(value >> 40);
        m_bytes[m_cursor++] = (System::Byte)(value >> 32);
        m_bytes[m_cursor++] = (System::Byte)(value >> 24);
        m_bytes[m_cursor++] = (System::Byte)(value >> 16);
        m_bytes[m_cursor++] = (System::Byte)(value >> 8);
        m_bytes[m_cursor++] = (System::Byte)value;
      }

      void DataOutput::WriteInt16(System::Int16 value)
      {
        WriteUInt16(value);
      }

      void DataOutput::WriteInt32(System::Int32 value)
      {
        WriteUInt32(value);
      }

      void DataOutput::WriteInt64(System::Int64 value)
      {
        WriteUInt64(value);
      }

      void DataOutput::WriteFloat(float value)
      {
        array<Byte>^ bytes = BitConverter::GetBytes(value);
        EnsureCapacity(4);
        for (int i = 4 - 1; i >= 0; i--)
          m_bytes[m_cursor++] = bytes[i];
      }

      void DataOutput::WriteDouble(double value)
      {
        array<Byte>^ bytes = BitConverter::GetBytes(value);
        EnsureCapacity(8);
        for (int i = 8 - 1; i >= 0; i--)
          m_bytes[m_cursor++] = bytes[i];
      }

      void DataOutput::WriteDictionary(System::Collections::IDictionary^ dict)
      {
        if (dict != nullptr)
        {
          this->WriteArrayLen(dict->Count);
          for each(System::Collections::DictionaryEntry^ entry in dict)
          {
            this->WriteObject(entry->Key);
            this->WriteObject(entry->Value);
          }
        }
        else
        {
          WriteByte((int8_t)-1);
        }
      }

      void DataOutput::WriteCollection(System::Collections::IList^ collection)
      {
        if (collection != nullptr)
        {
          this->WriteArrayLen(collection->Count);
          for each (Object^ obj in collection) {
            this->WriteObject(obj);
          }
        }
        else
          this->WriteByte((int8_t)-1);
      }

      void DataOutput::WriteDate(System::DateTime date)
      {
        if (date.Ticks != 0L)
        {
          CacheableDate^ cd = gcnew CacheableDate(date);
          cd->ToData(this);
        }
        else
          this->WriteInt64(-1L);
      }

      void DataOutput::WriteDateArray(array<System::DateTime>^ dateArray)
      {
        WriteObject<DateTime>(dateArray);
      }

      void DataOutput::WriteCharArray(array<Char>^ charArray)
      {
        if (charArray != nullptr)
        {
          this->WriteArrayLen(charArray->Length);
          for (int i = 0; i < charArray->Length; i++) {
            this->WriteObject(charArray[i]);
          }
        }
        else
          this->WriteByte((int8_t)-1);
      }

      void DataOutput::WriteObjectArray(List<Object^>^ objectArray)
      {
        if (objectArray != nullptr)
        {
          CacheableObjectArray^ coa = CacheableObjectArray::Create(objectArray);
          coa->ToData(this);
        }
        else
          this->WriteByte((int8_t)-1);
      }

      void DataOutput::WriteDotNetObjectArray(Object^ objectArray)
      {
        System::Collections::IList^ list = (System::Collections::IList^)objectArray;
        this->WriteArrayLen(list->Count);
        WriteByte((int8_t)apache::geode::client::internal::DSCode::Class);
        String^ pdxDomainClassname = m_cache->TypeRegistry->GetPdxTypeName(objectArray->GetType()->GetElementType()->FullName);
        WriteByte((int8_t)DSCode::CacheableASCIIString);
        WriteUTF(pdxDomainClassname);
        for each(Object^ o in list)
          WriteObject(o);
      }

      void DataOutput::WriteArrayOfByteArrays(array<array<Byte>^>^ byteArrays)
      {
        if (byteArrays != nullptr)
        {
          int fdLen = byteArrays->Length;
          this->WriteArrayLen(byteArrays->Length);
          for (int i = 0; i < fdLen; i++) {
            this->WriteBytes(byteArrays[i]);
          }
        }
        else
          this->WriteByte((int8_t)-1);
      }

      void DataOutput::WriteArrayOfSByteArrays(array<array<SByte>^>^ byteArrays)
      {
        if (byteArrays != nullptr)
        {
          int fdLen = byteArrays->Length;
          this->WriteArrayLen(byteArrays->Length);
          for (int i = 0; i < fdLen; i++) {
            this->WriteSBytes(byteArrays[i]);
          }
        }
        else
          this->WriteSByte((int8_t)-1);
      }

      void DataOutput::WriteUTF(String^ value)
      {
        if (value != nullptr) {
          int len = getEncodedLength(value);

          if (len > 0xffff)
            len = 0xffff;

          WriteUInt16(len);
          EnsureCapacity(len);
          EncodeUTF8String(value, len);
        }
        else {
          WriteUInt16(0);
        }
      }

      void DataOutput::WriteStringWithType(String^ value)
      {
        //value will not be null
        int len = getEncodedLength(value);

        if (len > 0xffff)
        {
          if (len == value->Length)//huge ascii
          {
            WriteByte(static_cast<unsigned char>(DSCode::CacheableASCIIStringHuge));
            WriteASCIIHuge(value);
          }
          else//huge utf
          {
            WriteByte(static_cast<unsigned char>(DSCode::CacheableStringHuge));
            WriteUTFHuge(value);
          }
          return;
        }

        if (len == value->Length)
        {
          WriteByte(static_cast<unsigned char>(DSCode::CacheableASCIIString));//ascii string
        }
        else
        {
          WriteByte(static_cast<unsigned char>(DSCode::CacheableString));//utf string
        }
        WriteUInt16(len);
        EnsureCapacity(len);
        EncodeUTF8String(value, len);

      }

      void DataOutput::WriteASCIIHuge(String^ value)
      {
        if (value != nullptr) {
          const int strLength = value->Length;
          WriteUInt32(strLength);
          EnsureCapacity(strLength);
          for (int i = 0; i < strLength; i++) {
            m_bytes[m_cursor++] = (System::Byte)value[i];
          }
        }
        else {
          WriteUInt32(0);
        }
      }

      /* Write UTF-16 */
      void DataOutput::WriteUTFHuge(String^ value)
      {
        if (value != nullptr) {
          WriteUInt32(value->Length);
          EnsureCapacity(value->Length * 2);
          for (int i = 0; i < value->Length; i++)
          {
            Char ch = value[i];
            m_bytes[m_cursor++] = (Byte)((ch & 0xff00) >> 8);
            m_bytes[m_cursor++] = (Byte)(ch & 0xff);
          }
        }
        else {
          WriteUInt32(0);
        }
      }

      /*void DataOutput::WriteObject( Object^ obj )
      {
      WriteObjectInternal((ISerializable^)obj);
      }*/

      /*void DataOutput::WriteObject( Object^ obj )
      {
      WriteObject( (ISerializable^)obj );
      }*/

      int8_t DataOutput::GetTypeId(System::UInt32 classId)
      {
        if (classId >= 0x80000000) {
          return (int8_t)((classId - 0x80000000) % 0x20000000);
        }
        else if (classId <= 0x7F) {
          return (int8_t)DSCode::CacheableUserData;
        }
        else if (classId <= 0x7FFF) {
          return (int8_t)DSCode::CacheableUserData2;
        }
        else {
          return (int8_t)DSCode::CacheableUserData4;
        }
      }

      int8_t DataOutput::DSFID(System::UInt32 classId)
      {
        // convention that [0x8000000, 0xa0000000) is for FixedIDDefault,
        // [0xa000000, 0xc0000000) is for FixedIDByte,
        // [0xc0000000, 0xe0000000) is for FixedIDShort
        // and [0xe0000000, 0xffffffff] is for FixedIDInt
        // Note: depends on fact that FixedIDByte is 1, FixedIDShort is 2
        // and FixedIDInt is 3; if this changes then correct this accordingly
        if (classId >= 0x80000000) {
          return (int8_t)((classId - 0x80000000) / 0x20000000);
        }
        return 0;
      }

      void DataOutput::WriteObject(Object^ obj)
      {

        if (obj == nullptr)
        {
          WriteByte((int8_t)DSCode::NullObj);
          return;
        }

        if (m_ispdxSerialization && obj->GetType()->IsEnum)
        {
          //need to set             
          int enumVal = Internal::PdxHelper::GetEnumValue(obj->GetType()->FullName, Enum::GetName(obj->GetType(), obj), obj->GetHashCode(), m_cache);
          WriteByte(static_cast<int8_t>(DSCode::PDX_ENUM));
          WriteByte(enumVal >> 24);
          WriteArrayLen(enumVal & 0xFFFFFF);
          return;
        }

        DSCode typeId = static_cast<DSCode>(m_cache->TypeRegistry->GetDsCodeForManagedType(obj->GetType()));

        switch (typeId)
        {
        case DSCode::CacheableByte:
        {
          WriteByte(static_cast<unsigned char>(typeId));
          WriteByte((Byte)obj);
          return;
        }
        case DSCode::CacheableBoolean:
        {
          WriteByte(static_cast<unsigned char>(typeId));
          WriteBoolean((bool)obj);
          return;
        }
        case DSCode::CacheableCharacter:
        {
          WriteByte(static_cast<unsigned char>(typeId));
          WriteObject((Char)obj);
          return;
        }
        case DSCode::CacheableDouble:
        {
          WriteByte(static_cast<unsigned char>(typeId));
          WriteDouble((Double)obj);
          return;
        }
        case DSCode::CacheableASCIIString:
        {
          //CacheableString^ cStr = CacheableString::Create((String^)obj);
          ////  TODO: igfser mapping between generic and non generic
          //WriteObjectInternal(cStr);
          WriteStringWithType((String^)obj);
          return;
        }
        case DSCode::CacheableFloat:
        {
          WriteByte(static_cast<unsigned char>(typeId));
          WriteFloat((float)obj);
          return;
        }
        case DSCode::CacheableInt16:
        {
          WriteByte(static_cast<unsigned char>(typeId));
          WriteInt16((Int16)obj);
          return;
        }
        case DSCode::CacheableInt32:
        {
          WriteByte(static_cast<unsigned char>(typeId));
          WriteInt32((Int32)obj);
          return;
        }
        case DSCode::CacheableInt64:
        {
          WriteByte(static_cast<unsigned char>(typeId));
          WriteInt64((Int64)obj);
          return;
        }
        case DSCode::CacheableDate:
        {
          //CacheableDate^ cd = gcnew CacheableDate((DateTime)obj);
          //  TODO: igfser mapping between generic and non generic
          //WriteObjectInternal(cd);
          WriteByte(static_cast<unsigned char>(typeId));
          WriteDate((DateTime)obj);
          return;
        }
        case DSCode::CacheableBytes:
        {
          WriteByte(static_cast<unsigned char>(typeId));
          WriteBytes((array<Byte>^)obj);
          return;
        }
        case DSCode::CacheableDoubleArray:
        {
          WriteByte(static_cast<unsigned char>(typeId));
          WriteObject((array<Double>^)obj);
          return;
        }
        case DSCode::CacheableFloatArray:
        {
          WriteByte(static_cast<unsigned char>(typeId));
          WriteObject((array<float>^)obj);
          return;
        }
        case DSCode::CacheableInt16Array:
        {
          WriteByte(static_cast<unsigned char>(typeId));
          WriteObject((array<Int16>^)obj);
          return;
        }
        case DSCode::CacheableInt32Array:
        {
          WriteByte(static_cast<unsigned char>(typeId));
          WriteObject((array<Int32>^)obj);
          return;
        }
        case DSCode::CacheableInt64Array:
        {
          WriteByte(static_cast<unsigned char>(typeId));
          WriteObject((array<Int64>^)obj);
          return;
        }
        case DSCode::BooleanArray:
        {
          WriteByte(static_cast<unsigned char>(typeId));
          WriteObject((array<bool>^)obj);
          return;
        }
        case DSCode::CharArray:
        {
          WriteByte(static_cast<unsigned char>(typeId));
          WriteObject((array<char>^)obj);
          return;
        }
        case DSCode::CacheableStringArray:
        {
          WriteByte(static_cast<unsigned char>(typeId));
          WriteObject((array<String^>^)obj);
          return;
        }
        case DSCode::CacheableHashTable:
        case DSCode::CacheableHashMap:
        case DSCode::CacheableIdentityHashMap:
        {
          WriteByte(static_cast<unsigned char>(typeId));
          WriteDictionary((System::Collections::IDictionary^)obj);
          return;
        }
        case DSCode::CacheableVector:
        {
          //CacheableVector^ cv = gcnew CacheableVector((System::Collections::IList^)obj);
          ////  TODO: igfser mapping between generic and non generic
          //WriteObjectInternal(cv);
          WriteByte(static_cast<unsigned char>(DSCode::CacheableVector));
          WriteList((System::Collections::IList^)obj);
          return;
        }
        case DSCode::CacheableLinkedList:
        {
          //CacheableArrayList^ cal = gcnew CacheableArrayList((System::Collections::IList^)obj);
          ////  TODO: igfser mapping between generic and non generic
          //WriteObjectInternal(cal);
          WriteByte(static_cast<unsigned char>(DSCode::CacheableLinkedList));
          System::Collections::ICollection^ linkedList = (System::Collections::ICollection^)obj;
          this->WriteArrayLen(linkedList->Count);
          for each (Object^ o in linkedList)
            this->WriteObject(o);
          return;
        }
        case DSCode::CacheableArrayList:
        {
          //CacheableArrayList^ cal = gcnew CacheableArrayList((System::Collections::IList^)obj);
          ////  TODO: igfser mapping between generic and non generic
          //WriteObjectInternal(cal);
          WriteByte(static_cast<unsigned char>(DSCode::CacheableArrayList));
          WriteList((System::Collections::IList^)obj);
          return;
        }
        case DSCode::CacheableStack:
        {
          CacheableStack^ cs = gcnew CacheableStack((System::Collections::ICollection^)obj);
          //  TODO: igfser mapping between generic and non generic
          WriteObjectInternal(cs);
          return;
        }
        default:
        {
          if (auto pdxObj = dynamic_cast<IPdxSerializable^>(obj))
          {
            WriteByte(static_cast<int8_t>(DSCode::PDX));
            Internal::PdxHelper::SerializePdx(this, pdxObj);
            return;
          }
          else
          {
            //pdx serialization and is array of object
            if (m_ispdxSerialization && obj->GetType()->IsArray)
            {
              WriteByte(static_cast<unsigned char>(DSCode::CacheableObjectArray));
              WriteDotNetObjectArray(obj);
              return;
            }

            if (auto ct = dynamic_cast<ISerializable^>(obj)) {
              WriteObjectInternal(ct);
              return;
            }

            if (m_cache->TypeRegistry->PdxSerializer)
            {
              auto pdxObj = gcnew PdxWrapper(obj);
              WriteByte(static_cast<int8_t>(DSCode::PDX));
              Internal::PdxHelper::SerializePdx(this, pdxObj);
              return;
            }
          }

          throw gcnew System::Exception("DataOutput not found appropriate type to write it for object: " + obj->GetType());
        }
        }
      }

      void DataOutput::WriteStringArray(array<String^>^ strArray)
      {
        if (strArray != nullptr)
        {
          this->WriteArrayLen(strArray->Length);
          for (int i = 0; i < strArray->Length; i++)
          {
            // this->WriteUTF(strArray[i]);
            WriteObject(strArray[i]);
          }
        }
        else
          WriteByte(-1);
      }

      void DataOutput::WriteObjectInternal(ISerializable^ obj)
      {
        //CacheableKey^ key = gcnew CacheableKey();
        if (obj == nullptr) {
          WriteByte((int8_t)DSCode::NullObj);
        }
        else if (auto dataSerializablePrimitive = dynamic_cast<IDataSerializablePrimitive^>(obj))
        {
          auto dsCode = dataSerializablePrimitive->DsCode;
          WriteByte(dsCode);
          dataSerializablePrimitive->ToData(this);
        }
        else if (auto dataSerializable = dynamic_cast<IDataSerializable^>(obj))
        {
          auto id = m_cache->TypeRegistry->GetIdForManagedType(dataSerializable->GetType());

          auto dsCode = getDataSerializableDsCode(id);
          WriteByte(static_cast<int8_t>(dsCode));
          switch (dsCode) {
            case DSCode::CacheableUserData:
              WriteByte(static_cast<int8_t>(id));
              break;
            case DSCode::CacheableUserData2:
              WriteInt16(static_cast<int16_t>(id));
              break;
            case DSCode::CacheableUserData4:
              WriteInt32(static_cast<int32_t>(id));
              break;
            default:
              IllegalStateException("Invalid DS Code.");
          }
          dataSerializable->ToData(this);
        }
        else if (auto dataSerializableFixedId = dynamic_cast<IDataSerializableFixedId^>(obj))
        {
          auto id = dataSerializableFixedId->DSFID;
          auto dsCode = getDataSerializableFixedIdDsCode(id);
          WriteByte(dsCode);
          switch (static_cast<DSCode>(dsCode)) {
            case DSCode::FixedIDByte:
              WriteByte(static_cast<int8_t>(id));
              break;
            case DSCode::FixedIDShort:
              WriteInt16(static_cast<int16_t>(id));
              break;
            case DSCode::FixedIDInt:
              WriteInt32(static_cast<int32_t>(id));
              break;
            default:
              IllegalStateException("Invalid DS Code.");
          }
          dataSerializable->ToData(this);
        }
        else
        {
          throw gcnew IllegalStateException("Unknown serializable type.");
        }
      }

      void DataOutput::AdvanceCursor(System::UInt32 offset)
      {
        EnsureCapacity(offset);
        m_cursor += offset;
      }

      void DataOutput::RewindCursor(System::UInt32 offset)
      {
        //first set native one
        WriteBytesToUMDataOutput();
        try
        {
          m_nativeptr->get()->rewindCursor(offset);
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }
        SetBuffer();
      }

      array<Byte>^ DataOutput::GetBuffer()
      {
        try
        {
          WriteBytesToUMDataOutput();
          SetBuffer();
          auto buffLen = static_cast<int>(m_nativeptr->get()->getBufferLength());
          array<Byte>^ buffer = gcnew array<Byte>(buffLen);

          if (buffLen > 0) {
            pin_ptr<Byte> pin_buffer = &buffer[0];
            memcpy((void*)pin_buffer, m_nativeptr->get()->getBuffer(), buffLen);
          }
          return buffer;
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }
      }

      size_t DataOutput::BufferLength::get()
      {
        //first set native one
        WriteBytesToUMDataOutput();
        SetBuffer();

        try
        {
          return m_nativeptr->get()->getBufferLength();
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }
      }

      void DataOutput::Reset()
      {
        WriteBytesToUMDataOutput();
        try
        {
          m_nativeptr->get()->reset();
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }
        SetBuffer();
      }

      void DataOutput::WriteString(String^ value)
      {
        if (value == nullptr)
        {
          this->WriteByte(static_cast<unsigned char>(DSCode::CacheableNullString));
        }
        else
        {
          WriteObject(value);
          /*CacheableString^ cs = gcnew CacheableString(value);

          this->WriteByte( (Byte)(cs->ClassId - 0x80000000));
          cs->ToData(this);*/
        }
      }

      void DataOutput::WriteBytesToUMDataOutput()
      {
        try
        {
          m_nativeptr->get()->advanceCursor(m_cursor);
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }
        m_cursor = 0;
        m_remainingBufferLength = 0;
        m_bytes = nullptr;
      }

      void DataOutput::WriteObject(bool% obj)
      {
        WriteBoolean(obj);
      }

      void DataOutput::WriteObject(Byte% obj)
      {
        WriteByte(obj);
      }

      void DataOutput::WriteObject(Char% obj)
      {
        unsigned short us = (unsigned short)obj;
        EnsureCapacity(2);
        m_bytes[m_cursor++] = us >> 8;
        m_bytes[m_cursor++] = (Byte)us;
      }

      void DataOutput::WriteObject(Double% obj)
      {
        WriteDouble(obj);
      }

      void DataOutput::WriteObject(Single% obj)
      {
        WriteFloat(obj);
      }

      void DataOutput::WriteObject(System::Int16% obj)
      {
        WriteInt16(obj);
      }

      void DataOutput::WriteObject(System::Int32% obj)
      {
        WriteInt32(obj);
      }

      void DataOutput::WriteObject(System::Int64% obj)
      {
        WriteInt64(obj);
      }

      void DataOutput::WriteObject(UInt16% obj)
      {
        WriteUInt16(obj);
      }

      void DataOutput::WriteObject(UInt32% obj)
      {
        WriteUInt32(obj);
      }

      void DataOutput::WriteObject(UInt64% obj)
      {
        WriteUInt64(obj);
      }

      void DataOutput::WriteBooleanArray(array<bool>^ boolArray)
      {
        WriteObject<bool>(boolArray);
      }

      void DataOutput::WriteShortArray(array<Int16>^ shortArray)
      {
        WriteObject<Int16>(shortArray);
      }

      void DataOutput::WriteUShortArray(array<UInt16>^ shortArray)
      {
        WriteObject<UInt16>(shortArray);
      }

      void DataOutput::WriteIntArray(array<Int32>^ intArray)
      {
        WriteObject<Int32>(intArray);
      }

      void DataOutput::WriteUIntArray(array<UInt32>^ intArray)
      {
        WriteObject<UInt32>(intArray);
      }

      void DataOutput::WriteLongArray(array<Int64>^ longArray)
      {
        WriteObject<Int64>(longArray);
      }

      void DataOutput::WriteULongArray(array<UInt64>^ longArray)
      {
        WriteObject<UInt64>(longArray);
      }

      void DataOutput::WriteFloatArray(array<float>^ floatArray)
      {
        WriteObject<float>(floatArray);
      }

      void DataOutput::WriteDoubleArray(array<double>^ doubleArray)
      {
        WriteObject<double>(doubleArray);
      }

      native::Pool* DataOutput::GetPool()
      {
        try
        {
          return native::DataOutputInternal::getPool(*m_nativeptr);
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }
      }

    }  // namespace Client
  }  // namespace Geode
}  // namespace Apache
