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

#include "../begin_native.hpp"
#include <PdxTypes.hpp>
#include "../end_native.hpp"

#include "PdxWriterWithTypeCollector.hpp"
#include "../DataOutput.hpp"
#include "PdxHelper.hpp"

using namespace System;


namespace Apache
{
  namespace Geode
  {
    namespace Client
    {

      namespace Internal
      {
        void PdxWriterWithTypeCollector::initialize()
        {
          m_offsets = gcnew System::Collections::Generic::List<Int32>();
          m_pdxType = gcnew PdxType(m_pdxClassName, true);
        }

        Int32 PdxWriterWithTypeCollector::calculateLenWithOffsets()
        {
          int bufferLen = static_cast<int>(m_dataOutput->BufferLength - m_startPositionOffset);
          Int32 totalOffsets = 0;
          if (m_offsets->Count > 0)
            totalOffsets = m_offsets->Count - 1;//for first var len no need to append offset
          Int32 totalLen = bufferLen - PdxHelper::PdxHeader + totalOffsets;


          if (totalLen <= 0xff)
            return totalLen;
          else if (totalLen + totalOffsets <= 0xffff)
            return totalLen + totalOffsets;
          else
            return totalLen + totalOffsets * 3;
        }

        void PdxWriterWithTypeCollector::AddOffset()
        {
          int bufferLen = static_cast<int>(m_dataOutput->BufferLength - m_startPositionOffset);
          int offset = bufferLen - PdxHelper::PdxHeader;

          m_offsets->Add(offset);
        }

        void PdxWriterWithTypeCollector::EndObjectWriting()
        {//write header 
          PdxLocalWriter::WritePdxHeader();
        }

        void PdxWriterWithTypeCollector::WriteOffsets(Int32 len)
        {
          if (len <= 0xff)
          {
            for (int i = m_offsets->Count - 1; i > 0; i--)
              m_dataOutput->WriteByte((Byte)m_offsets[i]);
          }
          else if (len <= 0xffff)
          {
            for (int i = m_offsets->Count - 1; i > 0; i--)
              m_dataOutput->WriteUInt16((UInt16)m_offsets[i]);
          }
          else
          {
            for (int i = m_offsets->Count - 1; i > 0; i--)
              m_dataOutput->WriteUInt32((UInt32)m_offsets[i]);
          }
        }

        bool PdxWriterWithTypeCollector::isFieldWritingStarted()
        {
          return m_pdxType->Totalfields > 0;
        }

        IPdxWriter^ PdxWriterWithTypeCollector::WriteUnreadFields(IPdxUnreadFields^ unread)
        {
          PdxLocalWriter::WriteUnreadFields(unread);
          return this;
        }

        IPdxWriter^ PdxWriterWithTypeCollector::WriteByte(String^ fieldName, SByte value)
        {
          m_pdxType->AddFixedLengthTypeField(fieldName, "sbyte", PdxFieldTypes::BYTE,  native::PdxTypes::BYTE_SIZE);
          PdxLocalWriter::WriteByte(fieldName, value);
          return this;
        }

        IPdxWriter^ PdxWriterWithTypeCollector::WriteUnsignedByte(String^ fieldName, Byte value)
        {
          m_pdxType->AddFixedLengthTypeField(fieldName, "byte", PdxFieldTypes::BYTE,  native::PdxTypes::BYTE_SIZE);
          PdxLocalWriter::WriteUnsignedByte(fieldName, value);
          return this;
        }

        IPdxWriter^ PdxWriterWithTypeCollector::WriteBoolean(String^ fieldName, bool value)
        {
          m_pdxType->AddFixedLengthTypeField(fieldName, "boolean", PdxFieldTypes::BOOLEAN,  native::PdxTypes::BOOLEAN_SIZE);
          PdxLocalWriter::WriteBoolean(fieldName, value);
          return this;
        }

        IPdxWriter^ PdxWriterWithTypeCollector::WriteChar(String^ fieldName, Char value)
        {
          m_pdxType->AddFixedLengthTypeField(fieldName, "char", PdxFieldTypes::CHAR,  native::PdxTypes::CHAR_SIZE);
          PdxLocalWriter::WriteChar(fieldName, value);
          return this;
        }

        IPdxWriter^ PdxWriterWithTypeCollector::WriteUShort(String^ fieldName, System::UInt16 value)
        {
          m_pdxType->AddFixedLengthTypeField(fieldName, "ushort", PdxFieldTypes::SHORT,  native::PdxTypes::SHORT_SIZE);
          PdxLocalWriter::WriteUShort(fieldName, value);
          return this;
        }

        IPdxWriter^ PdxWriterWithTypeCollector::WriteUInt(String^ fieldName, System::UInt32 value)
        {
          m_pdxType->AddFixedLengthTypeField(fieldName, "uint", PdxFieldTypes::INT,  native::PdxTypes::INTEGER_SIZE);
          PdxLocalWriter::WriteUInt(fieldName, value);
          return this;
        }

        IPdxWriter^ PdxWriterWithTypeCollector::WriteULong(String^ fieldName, System::UInt64 value)
        {
          m_pdxType->AddFixedLengthTypeField(fieldName, "ulong", PdxFieldTypes::LONG,  native::PdxTypes::LONG_SIZE);
          PdxLocalWriter::WriteULong(fieldName, value);
          return this;
        }

        IPdxWriter^ PdxWriterWithTypeCollector::WriteShort(String^ fieldName, System::Int16 value)
        {
          m_pdxType->AddFixedLengthTypeField(fieldName, "short", PdxFieldTypes::SHORT,  native::PdxTypes::SHORT_SIZE);
          PdxLocalWriter::WriteShort(fieldName, value);
          return this;
        }

        IPdxWriter^ PdxWriterWithTypeCollector::WriteInt(String^ fieldName, System::Int32 value)
        {
          m_pdxType->AddFixedLengthTypeField(fieldName, "int", PdxFieldTypes::INT,  native::PdxTypes::INTEGER_SIZE);
          PdxLocalWriter::WriteInt(fieldName, value);
          return this;
        }

        IPdxWriter^ PdxWriterWithTypeCollector::WriteLong(String^ fieldName, Int64 value)
        {
          m_pdxType->AddFixedLengthTypeField(fieldName, "long", PdxFieldTypes::LONG,  native::PdxTypes::LONG_SIZE);
          PdxLocalWriter::WriteLong(fieldName, value);
          return this;
        }

        IPdxWriter^ PdxWriterWithTypeCollector::WriteFloat(String^ fieldName, float value)
        {
          m_pdxType->AddFixedLengthTypeField(fieldName, "float", PdxFieldTypes::FLOAT,  native::PdxTypes::FLOAT_SIZE);
          PdxLocalWriter::WriteFloat(fieldName, value);
          return this;
        }

        IPdxWriter^ PdxWriterWithTypeCollector::WriteDouble(String^ fieldName, double value)
        {
          m_pdxType->AddFixedLengthTypeField(fieldName, "double", PdxFieldTypes::DOUBLE,  native::PdxTypes::DOUBLE_SIZE);
          PdxLocalWriter::WriteDouble(fieldName, value);
          return this;
        }

        IPdxWriter^ PdxWriterWithTypeCollector::WriteString(String^ fieldName, String^ value)
        {
          m_pdxType->AddVariableLengthTypeField(fieldName, "String", PdxFieldTypes::STRING);
          PdxLocalWriter::WriteString(fieldName, value);
          return this;
        }

        IPdxWriter^ PdxWriterWithTypeCollector::WriteUTFHuge(String^ fieldName, String^ value)
        {
          m_pdxType->AddVariableLengthTypeField(fieldName, "stringUTFHuge", PdxFieldTypes::STRING);
          PdxLocalWriter::WriteUTFHuge(fieldName, value);
          return this;
        }

        IPdxWriter^ PdxWriterWithTypeCollector::WriteASCIIHuge(String^ fieldName, String^ value)
        {
          m_pdxType->AddVariableLengthTypeField(fieldName, "stringASCIIHuge", PdxFieldTypes::STRING);
          PdxLocalWriter::WriteASCIIHuge(fieldName, value);
          return this;
        }

        IPdxWriter^ PdxWriterWithTypeCollector::WriteObject(String^ fieldName, Object^ obj)
        {
          m_pdxType->AddVariableLengthTypeField(fieldName, /*obj->GetType()->FullName*/"Object", PdxFieldTypes::OBJECT);
          PdxLocalWriter::WriteObject(fieldName, obj);
          return this;
        }

        IPdxWriter^ PdxWriterWithTypeCollector::WriteDate(String^ fieldName, System::DateTime date)
        {
          m_pdxType->AddFixedLengthTypeField(fieldName, "Date", PdxFieldTypes::DATE,  native::PdxTypes::DATE_SIZE);
          PdxLocalWriter::WriteDate(fieldName, date);
          return this;
        }

        IPdxWriter^ PdxWriterWithTypeCollector::WriteBooleanArray(String^ fieldName, array<bool>^ boolArray)
        {
          m_pdxType->AddVariableLengthTypeField(fieldName, "bool[]", PdxFieldTypes::BOOLEAN_ARRAY);
          PdxLocalWriter::WriteBooleanArray(fieldName, boolArray);
          return this;
        }

        IPdxWriter^ PdxWriterWithTypeCollector::WriteCharArray(String^ fieldName, array<Char>^ charArray)
        {
          m_pdxType->AddVariableLengthTypeField(fieldName, "char[]", PdxFieldTypes::CHAR_ARRAY);
          PdxLocalWriter::WriteCharArray(fieldName, charArray);
          return this;
        }

        IPdxWriter^ PdxWriterWithTypeCollector::WriteByteArray(String^ fieldName, array<Byte>^ byteArray)
        {
          m_pdxType->AddVariableLengthTypeField(fieldName, "byte[]", PdxFieldTypes::BYTE_ARRAY);
          PdxLocalWriter::WriteByteArray(fieldName, byteArray);
          return this;
        }

        IPdxWriter^ PdxWriterWithTypeCollector::WriteSByteArray(String^ fieldName, array<SByte>^ sbyteArray)
        {
          m_pdxType->AddVariableLengthTypeField(fieldName, "sbyte[]", PdxFieldTypes::BYTE_ARRAY);
          PdxLocalWriter::WriteSByteArray(fieldName, sbyteArray);
          return this;
        }

        IPdxWriter^ PdxWriterWithTypeCollector::WriteShortArray(String^ fieldName, array<System::Int16>^ shortArray)
        {
          m_pdxType->AddVariableLengthTypeField(fieldName, "short[]", PdxFieldTypes::SHORT_ARRAY);
          PdxLocalWriter::WriteShortArray(fieldName, shortArray);
          return this;
        }

        IPdxWriter^ PdxWriterWithTypeCollector::WriteUShortArray(String^ fieldName, array<System::UInt16>^ ushortArray)
        {
          m_pdxType->AddVariableLengthTypeField(fieldName, "ushort[]", PdxFieldTypes::SHORT_ARRAY);
          PdxLocalWriter::WriteUShortArray(fieldName, ushortArray);
          return this;
        }

        IPdxWriter^ PdxWriterWithTypeCollector::WriteIntArray(String^ fieldName, array<System::Int32>^ intArray)
        {
          m_pdxType->AddVariableLengthTypeField(fieldName, "int[]", PdxFieldTypes::INT_ARRAY);
          PdxLocalWriter::WriteIntArray(fieldName, intArray);
          return this;
        }

        IPdxWriter^ PdxWriterWithTypeCollector::WriteUIntArray(String^ fieldName, array<System::UInt32>^ uintArray)
        {
          m_pdxType->AddVariableLengthTypeField(fieldName, "uint[]", PdxFieldTypes::INT_ARRAY);
          PdxLocalWriter::WriteUIntArray(fieldName, uintArray);
          return this;
        }

        IPdxWriter^ PdxWriterWithTypeCollector::WriteLongArray(String^ fieldName, array<Int64>^ longArray)
        {
          m_pdxType->AddVariableLengthTypeField(fieldName, "long[]", PdxFieldTypes::LONG_ARRAY);
          PdxLocalWriter::WriteLongArray(fieldName, longArray);
          return this;
        }

        IPdxWriter^ PdxWriterWithTypeCollector::WriteULongArray(String^ fieldName, array<System::UInt64>^ ulongArray)
        {
          m_pdxType->AddVariableLengthTypeField(fieldName, "ulong[]", PdxFieldTypes::LONG_ARRAY);
          PdxLocalWriter::WriteULongArray(fieldName, ulongArray);
          return this;
        }

        IPdxWriter^ PdxWriterWithTypeCollector::WriteFloatArray(String^ fieldName, array<float>^ floatArray)
        {
          m_pdxType->AddVariableLengthTypeField(fieldName, "float[]", PdxFieldTypes::FLOAT_ARRAY);
          PdxLocalWriter::WriteFloatArray(fieldName, floatArray);
          return this;
        }

        IPdxWriter^ PdxWriterWithTypeCollector::WriteDoubleArray(String^ fieldName, array<double>^ doubleArray)
        {
          m_pdxType->AddVariableLengthTypeField(fieldName, "double[]", PdxFieldTypes::DOUBLE_ARRAY);
          PdxLocalWriter::WriteDoubleArray(fieldName, doubleArray);
          return this;
        }

        IPdxWriter^ PdxWriterWithTypeCollector::WriteStringArray(String^ fieldName, array<String^>^ stringArray)
        {
          m_pdxType->AddVariableLengthTypeField(fieldName, "String[]", PdxFieldTypes::STRING_ARRAY);
          PdxLocalWriter::WriteStringArray(fieldName, stringArray);
          return this;
        }

        IPdxWriter^ PdxWriterWithTypeCollector::WriteObjectArray(String^ fieldName, List<Object^>^ objectArray)
        {
          m_pdxType->AddVariableLengthTypeField(fieldName, "Object[]", PdxFieldTypes::OBJECT_ARRAY);
          PdxLocalWriter::WriteObjectArray(fieldName, objectArray);
          return this;
        }

        IPdxWriter^ PdxWriterWithTypeCollector::WriteArrayOfByteArrays(String^ fieldName, array<array<Byte>^>^ byteArrays)
        {
          m_pdxType->AddVariableLengthTypeField(fieldName, "byte[][]", PdxFieldTypes::ARRAY_OF_BYTE_ARRAYS);
          PdxLocalWriter::WriteArrayOfByteArrays(fieldName, byteArrays);
          return this;
        }

        IPdxWriter^ PdxWriterWithTypeCollector::WriteArrayOfSByteArrays(String^ fieldName, array<array<SByte>^>^ byteArrays)
        {
          m_pdxType->AddVariableLengthTypeField(fieldName, "sbyte[][]", PdxFieldTypes::ARRAY_OF_BYTE_ARRAYS);
          PdxLocalWriter::WriteArrayOfSByteArrays(fieldName, byteArrays);
          return this;
        }

        //TODO:
        //IPdxWriter^ PdxWriterWithTypeCollector::WriteEnum(String^ fieldName, Enum e) ;
        //IPdxWriter^ PdxWriterWithTypeCollector::WriteInetAddress(String^ fieldName, InetAddress address);


        IPdxWriter^ PdxWriterWithTypeCollector::MarkIdentityField(String^ fieldName)
        {
          PdxFieldType^ pft = m_pdxType->GetPdxField(fieldName);

          if (pft == nullptr)
          {
            throw gcnew IllegalStateException(
              "Field must be written to IPdxWriter before calling MarkIdentityField ");
          }

          pft->IdentityField = true;
          return this;
        }  // namespace Client
      }  // namespace Geode
    }  // namespace Apache

  }
}
