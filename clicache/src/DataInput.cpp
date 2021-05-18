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
#include <geode/Cache.hpp>
#include "SerializationRegistry.hpp"
#include "CacheRegionHelper.hpp"
#include "CacheImpl.hpp"
#include "DataInputInternal.hpp"
#include "end_native.hpp"


#include "DataInput.hpp"
#include "Cache.hpp"
#include "CacheableString.hpp"
#include "CacheableHashMap.hpp"
#include "CacheableStack.hpp"
#include "CacheableVector.hpp"
#include "CacheableArrayList.hpp"
#include "CacheableIDentityHashMap.hpp"
#include "CacheableDate.hpp"
#include "CacheableObjectArray.hpp"
#include "IDataSerializable.hpp"
#include "Serializable.hpp"
#include "impl/PdxHelper.hpp"
#include "impl/PdxWrapper.hpp"

using namespace System;
using namespace System::IO;
using namespace apache::geode::client;

namespace Apache
{
  namespace Geode
  {
    namespace Client
    {
      using namespace msclr::interop;
      namespace native = apache::geode::client;

      DataInput::DataInput(System::Byte* buffer, size_t size, Apache::Geode::Client::Cache^ cache)
      {
        m_ispdxDesrialization = false;
        m_isRootObjectPdx = false;
        m_cache = cache;
        if (buffer != nullptr && size > 0) {
          _GF_MG_EXCEPTION_TRY2

          m_nativeptr = gcnew native_conditional_unique_ptr<native::DataInput>(
            std::make_unique<native::DataInput>(cache->GetNative()->createDataInput(buffer, size)));
          m_cursor = 0;
          m_isManagedObject = false;
          m_forStringDecode = gcnew array<Char>(100);

          try
          {
            m_buffer = const_cast<System::Byte*>(m_nativeptr->get()->currentBufferPosition());
            m_bufferLength = m_nativeptr->get()->getBytesRemaining();
          }
          finally
          {
            GC::KeepAlive(m_nativeptr);
          }

          _GF_MG_EXCEPTION_CATCH_ALL2
        }
        else {
          throw gcnew IllegalArgumentException("DataInput.ctor(): "
                                               "provided buffer is null or empty");
        }
      }

      DataInput::DataInput(array<Byte>^ buffer, Apache::Geode::Client::Cache^ cache)
      {
        m_ispdxDesrialization = false;
        m_isRootObjectPdx = false;
        m_cache =  cache;
        if (buffer != nullptr && buffer->Length > 0) {
          _GF_MG_EXCEPTION_TRY2

          auto len = buffer->Length;
          m_ownedBuffer = make_native_unique<System::Byte[]>(len);
          m_buffer = m_ownedBuffer->get();
          pin_ptr<const Byte> pin_buffer = &buffer[0];
          memcpy(m_buffer, (void*)pin_buffer, len);
          m_nativeptr = gcnew native_conditional_unique_ptr<native::DataInput>(
            std::make_unique<native::DataInput>(m_cache->GetNative()->createDataInput(m_buffer, len)));
          m_cursor = 0;
          m_isManagedObject = false;
          m_forStringDecode = gcnew array<Char>(100);

          try
          {
            m_buffer = const_cast<System::Byte*>(m_nativeptr->get()->currentBufferPosition());
            m_bufferLength = m_nativeptr->get()->getBytesRemaining();
          }
          finally
          {
            GC::KeepAlive(m_nativeptr);
          }

          _GF_MG_EXCEPTION_CATCH_ALL2
        }
        else {
          throw gcnew IllegalArgumentException("DataInput.ctor(): "
                                               "provided buffer is null or empty");
        }
      }

      DataInput::DataInput(array<Byte>^ buffer, size_t len, Apache::Geode::Client::Cache^ cache)
      {
        m_ispdxDesrialization = false;
        m_isRootObjectPdx = false;
        m_cache = cache;
        if (buffer != nullptr) {
          if (len == 0 || (System::Int32)len > buffer->Length) {
            throw gcnew IllegalArgumentException(String::Format(
              "DataInput.ctor(): given length {0} is zero or greater than "
              "size of buffer {1}", len, buffer->Length));
          }
          _GF_MG_EXCEPTION_TRY2

          m_ownedBuffer = make_native_unique<System::Byte[]>(len);
          m_buffer = m_ownedBuffer->get();
          pin_ptr<const Byte> pin_buffer = &buffer[0];
          memcpy(m_buffer, (void*)pin_buffer, len);
          m_nativeptr = gcnew native_conditional_unique_ptr<native::DataInput>(
            std::make_unique<native::DataInput>(m_cache->GetNative()->createDataInput(m_buffer, len)));

          try
          {
            m_buffer = const_cast<System::Byte*>(m_nativeptr->get()->currentBufferPosition());
            m_bufferLength = m_nativeptr->get()->getBytesRemaining();
          }
          finally
          {
            GC::KeepAlive(m_nativeptr);
          }

          _GF_MG_EXCEPTION_CATCH_ALL2
        }
        else {
          throw gcnew IllegalArgumentException("DataInput.ctor(): "
                                               "provided buffer is null");
        }
      }

      void DataInput::CheckBufferSize(int size)
      {
        if ((unsigned int)(m_cursor + size) > m_bufferLength)
        {
          Log::Debug("DataInput::CheckBufferSize m_cursor:" + m_cursor + " size:" + size + " m_bufferLength:" + m_bufferLength);
          throw gcnew OutOfRangeException("DataInput: attempt to read beyond buffer");
        }
      }

      DataInput^ DataInput::GetClone()
      {
        return gcnew DataInput(m_buffer, m_bufferLength, m_cache);
      }

      Byte DataInput::ReadByte()
      {
        CheckBufferSize(1);
        return m_buffer[m_cursor++];
      }

      SByte DataInput::ReadSByte()
      {
        CheckBufferSize(1);
        return m_buffer[m_cursor++];
      }

      bool DataInput::ReadBoolean()
      {
        CheckBufferSize(1);
        Byte val = m_buffer[m_cursor++];
        if (val == 1)
          return true;
        else
          return false;
      }

      Char DataInput::ReadChar()
      {
        CheckBufferSize(2);
        Char data = m_buffer[m_cursor++];
        data = (data << 8) | m_buffer[m_cursor++];
        return data;
      }

      array<Byte>^ DataInput::ReadBytes()
      {
        System::Int32 length;
        length = ReadArrayLen();

        if (length >= 0) {
          if (length == 0)
            return gcnew array<Byte>(0);
          else {
            array<Byte>^ bytes = ReadBytesOnly(length);
            return bytes;
          }
        }
        return nullptr;
      }

      int DataInput::ReadArrayLen()
      {
        int code;
        int len;

        code = Convert::ToInt32(ReadByte());

        if (code == 0xFF) {
          len = -1;
        }
        else {
          unsigned int result = code;
          if (result > 252) {  // 252 is java's ((byte)-4 && 0xFF)
            if (code == 0xFE) {
              result = ReadUInt16();
            }
            else if (code == 0xFD) {
              result = ReadUInt32();
            }
            else {
              throw gcnew IllegalStateException("unexpected array length code");
            }
            //TODO:: illegal length
          }
          len = (int)result;
        }
        return len;
      }

      array<SByte>^ DataInput::ReadSBytes()
      {
        System::Int32 length;
        length = ReadArrayLen();

        if (length > -1) {
          if (length == 0)
            return gcnew array<SByte>(0);
          else {
            array<SByte>^ bytes = ReadSBytesOnly(length);
            return bytes;
          }
        }
        return nullptr;
      }

      array<Byte>^ DataInput::ReadBytesOnly(System::UInt32 len)
      {
        if (len > 0) {
          CheckBufferSize(len);
          array<Byte>^ bytes = gcnew array<Byte>(len);

          for (unsigned int i = 0; i < len; i++)
            bytes[i] = m_buffer[m_cursor++];

          return bytes;
        }
        return nullptr;
      }

      void DataInput::ReadBytesOnly(array<Byte> ^ buffer, int offset, int count)
      {
        if (count > 0) {
          CheckBufferSize((System::UInt32)count);

          for (int i = 0; i < count; i++)
            buffer[offset + i] = m_buffer[m_cursor++];
        }
      }

      array<SByte>^ DataInput::ReadSBytesOnly(System::UInt32 len)
      {
        if (len > 0) {
          CheckBufferSize(len);
          array<SByte>^ bytes = gcnew array<SByte>(len);

          for (unsigned int i = 0; i < len; i++)
            bytes[i] = (SByte)m_buffer[m_cursor++];

          return bytes;
        }
        return nullptr;
      }

      System::UInt16 DataInput::ReadUInt16()
      {
        CheckBufferSize(2);
        System::UInt16 data = m_buffer[m_cursor++];
        data = (data << 8) | m_buffer[m_cursor++];
        return data;
      }

      System::UInt32 DataInput::ReadUInt32()
      {
        CheckBufferSize(4);
        System::UInt32 data = m_buffer[m_cursor++];
        data = (data << 8) | m_buffer[m_cursor++];
        data = (data << 8) | m_buffer[m_cursor++];
        data = (data << 8) | m_buffer[m_cursor++];

        return data;
      }

      System::UInt64 DataInput::ReadUInt64()
      {
        System::UInt64 data;

        CheckBufferSize(8);

        data = m_buffer[m_cursor++];
        data = (data << 8) | m_buffer[m_cursor++];
        data = (data << 8) | m_buffer[m_cursor++];
        data = (data << 8) | m_buffer[m_cursor++];
        data = (data << 8) | m_buffer[m_cursor++];
        data = (data << 8) | m_buffer[m_cursor++];
        data = (data << 8) | m_buffer[m_cursor++];
        data = (data << 8) | m_buffer[m_cursor++];

        return data;
      }

      System::Int16 DataInput::ReadInt16()
      {
        return ReadUInt16();
      }

      System::Int32 DataInput::ReadInt32()
      {
        return ReadUInt32();
      }

      System::Int64 DataInput::ReadInt64()
      {
        return ReadUInt64();
      }

      array<Byte>^ DataInput::ReadReverseBytesOnly(int len)
      {
        CheckBufferSize(len);

        int i = 0;
        auto j = m_cursor + len - 1;
        array<Byte>^ bytes = gcnew array<Byte>(len);

        while (i < len)
        {
          bytes[i++] = m_buffer[j--];
        }
        m_cursor += len;
        return bytes;
      }

      float DataInput::ReadFloat()
      {
        float data;

        array<Byte>^ bytes = nullptr;
        if (BitConverter::IsLittleEndian)
          bytes = ReadReverseBytesOnly(4);
        else
          bytes = ReadBytesOnly(4);

        data = BitConverter::ToSingle(bytes, 0);

        return data;
      }

      double DataInput::ReadDouble()
      {
        double data;

        array<Byte>^ bytes = nullptr;
        if (BitConverter::IsLittleEndian)
          bytes = ReadReverseBytesOnly(8);
        else
          bytes = ReadBytesOnly(8);

        data = BitConverter::ToDouble(bytes, 0);

        return data;
      }

      String^ DataInput::ReadUTF()
      {
        int length = ReadUInt16();
        CheckBufferSize(length);
        String^ str = DecodeBytes(length);
        return str;
      }

      String^ DataInput::ReadUTFHuge()
      {
        int length = ReadUInt32();
        CheckBufferSize(length);

        array<Char>^ chArray = gcnew array<Char>(length);

        for (int i = 0; i < length; i++)
        {
          Char ch = ReadByte();
          ch = ((ch << 8) | ReadByte());
          chArray[i] = ch;
        }

        String^ str = gcnew String(chArray);

        return str;
      }

      String^ DataInput::ReadASCIIHuge()
      {
        int length = ReadInt32();
        CheckBufferSize(length);
        String^ str = DecodeBytes(length);
        return str;
      }

      Object^ DataInput::ReadObject()
      {
        return ReadInternalObject();
      }

      /*	Object^ DataInput::ReadGenericObject( )
        {
        return ReadInternalGenericObject();
        }*/

      Object^ DataInput::ReadDotNetTypes(int8_t typeId)
      {
        switch (static_cast<apache::geode::client::internal::DSCode>(typeId))
        {
        case apache::geode::client::internal::DSCode::CacheableByte:
        {
          return ReadByte();
        }
        case apache::geode::client::internal::DSCode::CacheableBoolean:
        {
          bool obj;
          ReadObject(obj);
          return obj;
        }
        case apache::geode::client::internal::DSCode::CacheableCharacter:
        {
          Char obj;
          ReadObject(obj);
          return obj;
        }
        case apache::geode::client::internal::DSCode::CacheableDouble:
        {
          Double obj;
          ReadObject(obj);
          return obj;
        }
        case apache::geode::client::internal::DSCode::CacheableASCIIString:
        {
          /*	CacheableString^ cs = static_cast<CacheableString^>(CacheableString::CreateDeserializable());
            cs->FromData(this);
            return cs->Value;*/
          return ReadUTF();
        }
        case apache::geode::client::internal::DSCode::CacheableASCIIStringHuge:
        {
          /*CacheableString^ cs = static_cast<CacheableString^>(CacheableString::createDeserializableHuge());
          cs->FromData(this);
          return cs->Value;*/
          return ReadASCIIHuge();
        }
        case apache::geode::client::internal::DSCode::CacheableString:
        {
          /*CacheableString^ cs = static_cast<CacheableString^>(CacheableString::createUTFDeserializable());
          cs->FromData(this);
          return cs->Value;*/
          return ReadUTF();
        }
        case apache::geode::client::internal::DSCode::CacheableStringHuge:
        {
          //TODO: need to look all strings types
          /*CacheableString^ cs = static_cast<CacheableString^>(CacheableString::createUTFDeserializableHuge());
          cs->FromData(this);
          return cs->Value;*/
          return ReadUTFHuge();
        }
        case apache::geode::client::internal::DSCode::CacheableFloat:
        {
          float obj;
          ReadObject(obj);
          return obj;
        }
        case apache::geode::client::internal::DSCode::CacheableInt16:
        {
          Int16 obj;
          ReadObject(obj);
          return obj;
        }
        case apache::geode::client::internal::DSCode::CacheableInt32:
        {
          Int32 obj;
          ReadObject(obj);
          return obj;
        }
        case apache::geode::client::internal::DSCode::CacheableInt64:
        {
          Int64 obj;
          ReadObject(obj);
          return obj;
        }
        case apache::geode::client::internal::DSCode::CacheableDate:
        {
          CacheableDate^ cd = CacheableDate::Create();
          cd->FromData(this);
          return cd->Value;
        }
        case apache::geode::client::internal::DSCode::CacheableBytes:
        {
          return ReadBytes();
        }
        case apache::geode::client::internal::DSCode::CacheableDoubleArray:
        {
          array<Double>^ obj;
          ReadObject(obj);
          return obj;
        }
        case apache::geode::client::internal::DSCode::CacheableFloatArray:
        {
          array<float>^ obj;
          ReadObject(obj);
          return obj;
        }
        case apache::geode::client::internal::DSCode::CacheableInt16Array:
        {
          array<Int16>^ obj;
          ReadObject(obj);
          return obj;
        }
        case apache::geode::client::internal::DSCode::CacheableInt32Array:
        {
          array<Int32>^ obj;
          ReadObject(obj);
          return obj;
        }
        case apache::geode::client::internal::DSCode::BooleanArray:
        {
          array<bool>^ obj;
          ReadObject(obj);
          return obj;
        }
        case apache::geode::client::internal::DSCode::CharArray:
        {
          array<Char>^ obj;
          ReadObject(obj);
          return obj;
        }
        case apache::geode::client::internal::DSCode::CacheableInt64Array:
        {
          array<Int64>^ obj;
          ReadObject(obj);
          return obj;
        }
        case apache::geode::client::internal::DSCode::CacheableStringArray:
        {
          return ReadStringArray();
        }
        case apache::geode::client::internal::DSCode::CacheableHashTable:
        {
          return ReadHashtable();
        }
        case apache::geode::client::internal::DSCode::CacheableHashMap:
        {
          CacheableHashMap^ chm = static_cast<CacheableHashMap^>(CacheableHashMap::CreateDeserializable());
          chm->FromData(this);
          return chm->Value;
        }
        case apache::geode::client::internal::DSCode::CacheableIdentityHashMap:
        {
          CacheableIdentityHashMap^ chm = static_cast<CacheableIdentityHashMap^>(CacheableIdentityHashMap::CreateDeserializable());
          chm->FromData(this);
          return chm->Value;
        }
        case apache::geode::client::internal::DSCode::CacheableVector:
        {
          /*CacheableVector^ cv = static_cast<CacheableVector^>(CacheableVector::CreateDeserializable());
          cv->FromData(this);
          return cv->Value;*/
          int len = ReadArrayLen();
          System::Collections::ArrayList^ retA = gcnew System::Collections::ArrayList(len);

          for (int i = 0; i < len; i++)
          {
            retA->Add(this->ReadObject());
          }
          return retA;
        }
        case apache::geode::client::internal::DSCode::CacheableArrayList:
        {
          /*CacheableArrayList^ cv = static_cast<CacheableArrayList^>(CacheableArrayList::CreateDeserializable());
          cv->FromData(this);
          return cv->Value;*/
          int len = ReadArrayLen();
          System::Collections::Generic::List<Object^>^ retA = gcnew System::Collections::Generic::List<Object^>(len);
          for (int i = 0; i < len; i++)
          {
            retA->Add(this->ReadObject());
          }
          return retA;

        }
        case apache::geode::client::internal::DSCode::CacheableLinkedList:
        {
          /*CacheableArrayList^ cv = static_cast<CacheableArrayList^>(CacheableArrayList::CreateDeserializable());
          cv->FromData(this);
          return cv->Value;*/
          int len = ReadArrayLen();
          System::Collections::Generic::LinkedList<Object^>^ retA = gcnew System::Collections::Generic::LinkedList<Object^>();
          for (int i = 0; i < len; i++)
          {
            retA->AddLast(this->ReadObject());
          }
          return retA;

        }
        case apache::geode::client::internal::DSCode::CacheableStack:
        {
          CacheableStack^ cv = static_cast<CacheableStack^>(CacheableStack::CreateDeserializable());
          cv->FromData(this);
          return cv->Value;
        }
        default:
          return nullptr;
        }
      }

      Object^ DataInput::ReadInternalObject()
      {
        try
        {
          //Log::Debug("DataInput::ReadInternalObject m_cursor " + m_cursor);
          bool findinternal = false;
          int8_t typeId = ReadByte();
          System::Int64 compId = 0;
          TypeFactoryMethod^ createType = nullptr;

          switch (static_cast<DSCode>(typeId)) 
          {
            case DSCode::NullObj:
              return nullptr;
            case DSCode::PDX: 
            {
              //cache current state and reset after reading pdx object
              auto cacheCursor = m_cursor;
              System::Byte* cacheBuffer = m_buffer;
              auto cacheBufferLength = m_bufferLength;
              Object^ ret = Internal::PdxHelper::DeserializePdx(this, false, CacheRegionHelper::getCacheImpl(m_cache->GetNative().get())->getSerializationRegistry().get());
              auto tmp = m_nativeptr->get()->getBytesRemaining();
              m_cursor = cacheBufferLength - tmp;
              m_buffer = cacheBuffer;
              m_bufferLength = cacheBufferLength;
              m_nativeptr->get()->rewindCursor(m_cursor);

              if (ret != nullptr)
              {
                if (auto pdxWrapper = dynamic_cast<Apache::Geode::Client::PdxWrapper^>(ret))
                {
                  return pdxWrapper->GetObject();
                }
              }
              return ret;
            }
            case DSCode::PDX_ENUM: 
            {
              int8_t dsId = ReadByte();
              int tmp = ReadArrayLen();
              int enumId = (dsId << 24) | (tmp & 0xFFFFFF);

              Object^ enumVal = Internal::PdxHelper::GetEnum(enumId, m_cache);
              return enumVal;
            }
            case DSCode::CacheableNullString:
              return nullptr;
            case DSCode::CacheableUserData:
              compId = ReadByte();
              break;
            case DSCode::CacheableUserData2:
              compId = ReadInt16();
              break;
            case DSCode::CacheableUserData4:
              compId = ReadInt32();
              break;
            case DSCode::FixedIDByte:
              compId = ReadByte();
              findinternal = true;
              break;
            case DSCode::FixedIDShort:
              compId = ReadInt16();
              findinternal = true;
              break;
            case DSCode::FixedIDInt:
              compId = ReadInt32();
              findinternal = true;
              break;
          }

          if (findinternal) 
          {
            createType = m_cache->TypeRegistry->GetDataSerializableFixedTypeFactoryMethodForFixedId(static_cast<Int32>(compId));
          }
          else 
          {
            createType = m_cache->TypeRegistry->GetManagedObjectFactory(compId);
            if (createType == nullptr)
            {
              Object^ retVal = ReadDotNetTypes(typeId);

              if (retVal != nullptr)
                return retVal;

              if (m_ispdxDesrialization && static_cast<DSCode>(typeId) == DSCode::CacheableObjectArray)
              {//object array and pdxSerialization
                return readDotNetObjectArray();
              }
              
              createType = m_cache->TypeRegistry->GetDataSerializablePrimitiveTypeFactoryMethodForDsCode(typeId);
            }
          }

          if (createType == nullptr) 
          {
            throw gcnew IllegalStateException("Unregistered typeId " + typeId + " in deserialization, aborting.");
          }

          bool isPdxDeserialization = m_ispdxDesrialization;
          m_ispdxDesrialization = false;//for nested objects
          ISerializable^ newObj = createType();

          if (auto dataSerializable = dynamic_cast<IDataSerializablePrimitive^>(newObj))
          {
            dataSerializable->FromData(this);
          }
          else if (auto dataSerializable = dynamic_cast<IDataSerializable^>(newObj))
          {
            dataSerializable->FromData(this);
          }
          else if (auto dataSerializableFixedId = dynamic_cast<IDataSerializableFixedId^>(newObj))
          {
            dataSerializableFixedId->FromData(this);
          }
          else
          {
            throw gcnew IllegalStateException("Unknown serialization type.");
          }

          m_ispdxDesrialization = isPdxDeserialization;
          return newObj;
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }
      }

      Object^ DataInput::readDotNetObjectArray()
      {
        int len = ReadArrayLen();
        String^ className = nullptr;
        if (len >= 0)
        {
          ReadByte(); // ignore CLASS typeid
          className = (String^)ReadObject();
          className = m_cache->TypeRegistry->GetLocalTypeName(className);
          System::Collections::IList^ list = nullptr;
          if (len == 0)
          {
            list = (System::Collections::IList^)m_cache->TypeRegistry->GetArrayObject(className, len);
            return list;
          }
          //read first object

          Object^ ret = ReadObject();//in case it returns pdxinstance or java.lang.object

          list = (System::Collections::IList^)m_cache->TypeRegistry->GetArrayObject(ret->GetType()->FullName, len);

          list[0] = ret;
          for (System::Int32 index = 1; index < list->Count; ++index)
          {
            list[index] = ReadObject();
          }
          return list;
        }
        return nullptr;
      }

      size_t DataInput::BytesRead::get()
      {
        AdvanceUMCursor();
        SetBuffer();

        try
        {
          return m_nativeptr->get()->getBytesRead();
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }
      }

      size_t DataInput::BytesReadInternally::get()
      {
        return m_cursor;
      }

      size_t DataInput::BytesRemaining::get()
      {
        AdvanceUMCursor();
        SetBuffer();
        try
        {
          return m_nativeptr->get()->getBytesRemaining();
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }
      }

      void DataInput::AdvanceCursor(size_t offset)
      {
        m_cursor += offset;
      }

      void DataInput::RewindCursor(size_t offset)
      {
        AdvanceUMCursor();
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

      void DataInput::Reset()
      {
        AdvanceUMCursor();
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

      void DataInput::ReadDictionary(System::Collections::IDictionary^ dict)
      {
        int len = this->ReadArrayLen();

        if (len > 0)
        {
          for (int i = 0; i < len; i++)
          {
            Object^ key = this->ReadObject();
            Object^ val = this->ReadObject();

            dict->Add(key, val);
          }
        }
      }

      IDictionary<Object^, Object^>^ DataInput::ReadDictionary()
      {
        int len = this->ReadArrayLen();

        if (len == -1)
          return nullptr;
        else
        {
          IDictionary<Object^, Object^>^ dict = gcnew Dictionary<Object^, Object^>();
          for (int i = 0; i < len; i++)
          {
            Object^ key = this->ReadObject();
            Object^ val = this->ReadObject();

            dict->Add(key, val);
          }
          return dict;
        }
      }

      System::DateTime DataInput::ReadDate()
      {
        long ticks = (long)ReadInt64();
        if (ticks != -1L)
        {
          m_cursor -= 8;//for above
          CacheableDate^ cd = CacheableDate::Create();
          cd->FromData(this);
          return cd->Value;
        }
        else
        {
          DateTime dt(0);
          return dt;
        }
      }

      void DataInput::ReadCollection(System::Collections::IList^ coll)
      {
        int len = ReadArrayLen();
        for (int i = 0; i < len; i++)
        {
          coll->Add(ReadObject());
        }
      }

      array<Char>^ DataInput::ReadCharArray()
      {
        array<Char>^ arr;
        this->ReadObject(arr);
        return arr;
      }

      array<bool>^ DataInput::ReadBooleanArray()
      {
        array<bool>^ arr;
        this->ReadObject(arr);
        return arr;
      }

      array<Int16>^ DataInput::ReadShortArray()
      {
        array<Int16>^ arr;
        this->ReadObject(arr);
        return arr;
      }

      array<Int32>^ DataInput::ReadIntArray()
      {
        array<Int32>^ arr;
        this->ReadObject(arr);
        return arr;
      }

      array<Int64>^ DataInput::ReadLongArray()
      {
        array<Int64>^ arr;
        this->ReadObject(arr);
        return arr;
      }

      array<float>^ DataInput::ReadFloatArray()
      {
        array<float>^ arr;
        this->ReadObject(arr);
        return arr;
      }

      array<double>^ DataInput::ReadDoubleArray()
      {
        array<double>^ arr;
        this->ReadObject(arr);
        return arr;
      }

      List<Object^>^ DataInput::ReadObjectArray()
      {
        //this to know whether it is null or it is empty
        auto storeCursor = m_cursor;
        auto len = this->ReadArrayLen();
        if (len == -1)
          return nullptr;
        //this will be read further by fromdata
        m_cursor = m_cursor - (m_cursor - storeCursor);


        CacheableObjectArray^ coa = CacheableObjectArray::Create();
        coa->FromData(this);
        List<Object^>^ retObj = (List<Object^>^)coa;

        if (retObj->Count >= 0)
          return retObj;
        return nullptr;
      }

      array<array<Byte>^>^ DataInput::ReadArrayOfByteArrays()
      {
        int len = ReadArrayLen();
        if (len >= 0)
        {
          array<array<Byte>^>^ retVal = gcnew array<array<Byte>^>(len);
          for (int i = 0; i < len; i++)
          {
            retVal[i] = this->ReadBytes();
          }
          return retVal;
        }
        else
          return nullptr;
      }

      void DataInput::ReadObject(array<UInt16>^% obj)
      {
        int len = ReadArrayLen();
        if (len >= 0)
        {
          obj = gcnew array<UInt16>(len);
          for (int i = 0; i < len; i++)
          {
            obj[i] = this->ReadUInt16();
          }
        }
      }

      void DataInput::ReadObject(array<UInt32>^% obj)
      {
        int len = ReadArrayLen();
        if (len >= 0)
        {
          obj = gcnew array<UInt32>(len);
          for (int i = 0; i < len; i++)
          {
            obj[i] = this->ReadUInt32();
          }
        }
      }

      void DataInput::ReadObject(array<UInt64>^% obj)
      {
        int len = ReadArrayLen();
        if (len >= 0)
        {
          obj = gcnew array<UInt64>(len);
          for (int i = 0; i < len; i++)
          {
            obj[i] = this->ReadUInt64();
          }
        }
      }

      String^ DataInput::ReadString()
      {
        switch (static_cast<DSCode>(ReadByte()))
        {
          case DSCode::CacheableNullString:
            return nullptr;
          case DSCode::CacheableASCIIString:
          case DSCode::CacheableString:
            return ReadUTF();
          case DSCode::CacheableASCIIStringHuge:
            return ReadASCIIHuge();
          default:
            return ReadUTFHuge();
        }
      }

      native::Pool* DataInput::GetPool()
      {
        try
        {
          return native::DataInputInternal::getPool(*m_nativeptr);
        }
        finally {
          GC::KeepAlive(m_nativeptr);
        }
      }

    }  // namespace Client
  }  // namespace Geode
}  // namespace Apache

