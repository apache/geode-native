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


#include "PdxReaderWithTypeCollector.hpp"
#include "../begin_native.hpp"
#include <PdxTypes.hpp>
#include "../end_native.hpp"

using namespace System;

namespace Apache
{
  namespace Geode
  {
    namespace Client
    {
      namespace Internal
      {
        namespace native = apache::geode::client;

        void PdxReaderWithTypeCollector::checkType(String^ fieldName, PdxFieldTypes typeId, String^ fieldType)
        {
          PdxFieldType^ pft = m_pdxType->GetPdxField(fieldName);

          if (pft != nullptr)
          {
            if (static_cast<int8_t>(typeId) != pft->TypeId)
            {
              throw gcnew IllegalStateException("Expected " + fieldType + " field but found field of type " + pft);
            }
          }
        }

        SByte PdxReaderWithTypeCollector::ReadByte(String^ fieldName)
        {
          checkType(fieldName, PdxFieldTypes::BYTE, "byte");
          m_newPdxType->AddFixedLengthTypeField(fieldName, "byte", PdxFieldTypes::BYTE, native::PdxTypes::kPdxByteSize);
          int position = m_pdxType->GetFieldPosition(fieldName, m_offsetsBuffer, m_offsetSize, m_serializedLength);
          if (position != -1)
          {
            m_dataInput->AdvanceCursorPdx(position);
            SByte retVal = PdxLocalReader::ReadByte(fieldName);
            m_dataInput->RewindCursorPdx(position);

            return retVal;
          }
          return 0;
        }

        SByte PdxReaderWithTypeCollector::ReadSByte(String^ fieldName)
        {
          checkType(fieldName, PdxFieldTypes::BYTE, "byte");
          m_newPdxType->AddFixedLengthTypeField(fieldName, "byte", PdxFieldTypes::BYTE, native::PdxTypes::kPdxByteSize);
          int position = m_pdxType->GetFieldPosition(fieldName, m_offsetsBuffer, m_offsetSize, m_serializedLength);
          if (position != -1)
          {
            m_dataInput->AdvanceCursorPdx(position);
            SByte retVal = PdxLocalReader::ReadSByte(fieldName);
            m_dataInput->RewindCursorPdx(position);
            return retVal;
          }
          return 0;
        }

        Boolean PdxReaderWithTypeCollector::ReadBoolean(String^ fieldName)
        {
          checkType(fieldName, PdxFieldTypes::BOOLEAN, "boolean");
          m_newPdxType->AddFixedLengthTypeField(fieldName, "boolean", PdxFieldTypes::BOOLEAN, native::PdxTypes::kPdxBooleanSize);
          int position = m_pdxType->GetFieldPosition(fieldName, m_offsetsBuffer, m_offsetSize, m_serializedLength);
          m_dataInput->AdvanceCursorPdx(position);
          bool retVal = PdxLocalReader::ReadBoolean(fieldName);
          m_dataInput->RewindCursorPdx(position);
          return retVal;
        }

        Char PdxReaderWithTypeCollector::ReadChar(String^ fieldName)
        {
          checkType(fieldName, PdxFieldTypes::CHAR, "char");
          m_newPdxType->AddFixedLengthTypeField(fieldName, "char", PdxFieldTypes::CHAR, native::PdxTypes::kPdxCharSize);
          int position = m_pdxType->GetFieldPosition(fieldName, m_offsetsBuffer, m_offsetSize, m_serializedLength);
          m_dataInput->AdvanceCursorPdx(position);
          Char retVal = PdxLocalReader::ReadChar(fieldName);
          m_dataInput->RewindCursorPdx(position);
          return retVal;
        }

        System::UInt16 PdxReaderWithTypeCollector::ReadUInt16(String^ fieldName)
        {
          checkType(fieldName, PdxFieldTypes::SHORT, "short");
          m_newPdxType->AddFixedLengthTypeField(fieldName, "short", PdxFieldTypes::SHORT, native::PdxTypes::kPdxShortSize);
          int position = m_pdxType->GetFieldPosition(fieldName, m_offsetsBuffer, m_offsetSize, m_serializedLength);
          if (position != -1)
          {
            m_dataInput->AdvanceCursorPdx(position);
            UInt16 retVal = PdxLocalReader::ReadUInt16(fieldName);
            m_dataInput->RewindCursorPdx(position);
            return retVal;
          }
          return 0;
        }

        System::UInt32 PdxReaderWithTypeCollector::ReadUInt32(String^ fieldName)
        {
          checkType(fieldName, PdxFieldTypes::INT, "int");
          m_newPdxType->AddFixedLengthTypeField(fieldName, "int", PdxFieldTypes::INT, native::PdxTypes::kPdxIntegerSize);
          int position = m_pdxType->GetFieldPosition(fieldName, m_offsetsBuffer, m_offsetSize, m_serializedLength);
          if (position != -1)
          {
            m_dataInput->AdvanceCursorPdx(position);
            UInt32 retVal = PdxLocalReader::ReadUInt32(fieldName);
            m_dataInput->RewindCursorPdx(position);
            return retVal;
          }
          return 0;
        }

        System::UInt64 PdxReaderWithTypeCollector::ReadUInt64(String^ fieldName)
        {
          checkType(fieldName, PdxFieldTypes::LONG, "long");
          m_newPdxType->AddFixedLengthTypeField(fieldName, "long", PdxFieldTypes::LONG, native::PdxTypes::kPdxLongSize);
          int position = m_pdxType->GetFieldPosition(fieldName, m_offsetsBuffer, m_offsetSize, m_serializedLength);
          if (position != -1)
          {
            m_dataInput->AdvanceCursorPdx(position);
            UInt64 retVal = PdxLocalReader::ReadUInt64(fieldName);
            m_dataInput->RewindCursorPdx(position);
            return retVal;
          }
          return 0;
        }

        short PdxReaderWithTypeCollector::ReadShort(String^ fieldName)
        {
          checkType(fieldName, PdxFieldTypes::SHORT, "short");
          m_newPdxType->AddFixedLengthTypeField(fieldName, "short", PdxFieldTypes::SHORT, native::PdxTypes::kPdxShortSize);
          int position = m_pdxType->GetFieldPosition(fieldName, m_offsetsBuffer, m_offsetSize, m_serializedLength);
          if (position != -1)
          {
            m_dataInput->AdvanceCursorPdx(position);
            Int16 retVal = PdxLocalReader::ReadShort(fieldName);
            m_dataInput->RewindCursorPdx(position);
            return retVal;
          }
          return 0;
        }

        Int32 PdxReaderWithTypeCollector::ReadInt(String^ fieldName)
        {
          checkType(fieldName, PdxFieldTypes::INT, "int");
          m_newPdxType->AddFixedLengthTypeField(fieldName, "int", PdxFieldTypes::INT, native::PdxTypes::kPdxIntegerSize);
          int position = m_pdxType->GetFieldPosition(fieldName, m_offsetsBuffer, m_offsetSize, m_serializedLength);
          if (position != -1)
          {
            m_dataInput->AdvanceCursorPdx(position);
            Int32 retVal = PdxLocalReader::ReadInt(fieldName);
            m_dataInput->RewindCursorPdx(position);
            return retVal;
          }
          return 0;
        }

        Int64 PdxReaderWithTypeCollector::ReadLong(String^ fieldName)
        {
          checkType(fieldName, PdxFieldTypes::LONG, "long");
          m_newPdxType->AddFixedLengthTypeField(fieldName, "long", PdxFieldTypes::LONG, native::PdxTypes::kPdxLongSize);
          int position = m_pdxType->GetFieldPosition(fieldName, m_offsetsBuffer, m_offsetSize, m_serializedLength);
          if (position != -1)
          {
            m_dataInput->AdvanceCursorPdx(position);
            Int64 retVal = PdxLocalReader::ReadLong(fieldName);
            m_dataInput->RewindCursorPdx(position);
            return retVal;
          }
          return 0;
        }

        float PdxReaderWithTypeCollector::ReadFloat(String^ fieldName)
        {
          checkType(fieldName, PdxFieldTypes::FLOAT, "float");
          m_newPdxType->AddFixedLengthTypeField(fieldName, "float", PdxFieldTypes::FLOAT, native::PdxTypes::kPdxFloatSize);
          int position = m_pdxType->GetFieldPosition(fieldName, m_offsetsBuffer, m_offsetSize, m_serializedLength);
          if (position != -1)
          {
            m_dataInput->AdvanceCursorPdx(position);
            float retVal = PdxLocalReader::ReadFloat(fieldName);
            m_dataInput->RewindCursorPdx(position);
            return retVal;
          }
          return 0.0;
        }

        double PdxReaderWithTypeCollector::ReadDouble(String^ fieldName)
        {
          checkType(fieldName, PdxFieldTypes::DOUBLE, "double");
          m_newPdxType->AddFixedLengthTypeField(fieldName, "double", PdxFieldTypes::DOUBLE, native::PdxTypes::kPdxDoubleSize);
          int position = m_pdxType->GetFieldPosition(fieldName, m_offsetsBuffer, m_offsetSize, m_serializedLength);
          if (position != -1)
          {
            m_dataInput->AdvanceCursorPdx(position);
            Double retVal = PdxLocalReader::ReadDouble(fieldName);
            m_dataInput->RewindCursorPdx(position);
            return retVal;
          }
          return 0.0;
        }

        String^ PdxReaderWithTypeCollector::ReadString(String^ fieldName)
        {
          checkType(fieldName, PdxFieldTypes::STRING, "String");
          m_newPdxType->AddVariableLengthTypeField(fieldName, "String", PdxFieldTypes::STRING);
          int position = m_pdxType->GetFieldPosition(fieldName, m_offsetsBuffer, m_offsetSize, m_serializedLength);
          if (position != -1)
          {
            m_dataInput->AdvanceCursorPdx(position);
            String^ retVal = PdxLocalReader::ReadString(fieldName);
            m_dataInput->RewindCursorPdx(position);
            return retVal;
          }
          return nullptr;
        }

        String^ PdxReaderWithTypeCollector::ReadUTFHuge(String^ fieldName)
        {
          checkType(fieldName, PdxFieldTypes::STRING, "String");
          m_newPdxType->AddVariableLengthTypeField(fieldName, "String", PdxFieldTypes::STRING);
          int position = m_pdxType->GetFieldPosition(fieldName, m_offsetsBuffer, m_offsetSize, m_serializedLength);
          if (position != -1)
          {
            m_dataInput->AdvanceCursorPdx(position);
            String^ retVal = PdxLocalReader::ReadUTFHuge(fieldName);
            m_dataInput->RewindCursorPdx(position);
            return retVal;
          }
          return nullptr;
        }

        String^ PdxReaderWithTypeCollector::ReadASCIIHuge(String^ fieldName)
        {
          checkType(fieldName, PdxFieldTypes::STRING, "String");
          m_newPdxType->AddVariableLengthTypeField(fieldName, "String", PdxFieldTypes::STRING);
          int position = m_pdxType->GetFieldPosition(fieldName, m_offsetsBuffer, m_offsetSize, m_serializedLength);
          if (position != -1)
          {
            m_dataInput->AdvanceCursorPdx(position);
            String^ retVal = PdxLocalReader::ReadASCIIHuge(fieldName);
            m_dataInput->RewindCursorPdx(position);
            return retVal;
          }
          return nullptr;
        }

        Object^ PdxReaderWithTypeCollector::ReadObject(String^ fieldName)
        {
          //field is collected after reading
          checkType(fieldName, PdxFieldTypes::OBJECT, "Object");
          m_newPdxType->AddVariableLengthTypeField(fieldName, "Object"/*retVal->GetType()->FullName*/, PdxFieldTypes::OBJECT);
          int position = m_pdxType->GetFieldPosition(fieldName, m_offsetsBuffer, m_offsetSize, m_serializedLength);
          if (position != -1)
          {
            m_dataInput->AdvanceCursorPdx(position);
            Object^ retVal = PdxLocalReader::ReadObject(fieldName);
            m_dataInput->ResetPdx(m_startPosition);//force native as well
            m_dataInput->RewindCursorPdx(position);
            return retVal;
          }
          return nullptr;
        }

        System::DateTime PdxReaderWithTypeCollector::ReadDate(String^ fieldName)
        {
          checkType(fieldName, PdxFieldTypes::DATE, "Date");
          m_newPdxType->AddFixedLengthTypeField(fieldName, "Date", PdxFieldTypes::DATE, native::PdxTypes::kPdxDateSize);
          int position = m_pdxType->GetFieldPosition(fieldName, m_offsetsBuffer, m_offsetSize, m_serializedLength);
          if (position != -1)
          {
            m_dataInput->AdvanceCursorPdx(position);
            System::DateTime retVal = PdxLocalReader::ReadDate(fieldName);
            m_dataInput->RewindCursorPdx(position);
            return retVal;
          }
          DateTime dt(0);
          return dt;
        }

        array<bool>^ PdxReaderWithTypeCollector::ReadBooleanArray(String^ fieldName)
        {
          checkType(fieldName, PdxFieldTypes::BOOLEAN_ARRAY, "boolean[]");
          m_newPdxType->AddVariableLengthTypeField(fieldName, "boolean[]", PdxFieldTypes::BOOLEAN_ARRAY);
          int position = m_pdxType->GetFieldPosition(fieldName, m_offsetsBuffer, m_offsetSize, m_serializedLength);
          if (position != -1)
          {
            m_dataInput->AdvanceCursorPdx(position);
            array<bool>^ retVal = PdxLocalReader::ReadBooleanArray(fieldName);
            m_dataInput->RewindCursorPdx(position);
            return retVal;
          }
          return nullptr;
        }

        array<Char>^ PdxReaderWithTypeCollector::ReadCharArray(String^ fieldName)
        {
          checkType(fieldName, PdxFieldTypes::CHAR_ARRAY, "char[]");
          m_newPdxType->AddVariableLengthTypeField(fieldName, "char[]", PdxFieldTypes::CHAR_ARRAY);
          int position = m_pdxType->GetFieldPosition(fieldName, m_offsetsBuffer, m_offsetSize, m_serializedLength);
          if (position != -1)
          {
            m_dataInput->AdvanceCursorPdx(position);
            array<Char>^ retVal = PdxLocalReader::ReadCharArray(fieldName);
            m_dataInput->RewindCursorPdx(position);
            return retVal;
          }
          return nullptr;
        }

        array<Byte>^ PdxReaderWithTypeCollector::ReadByteArray(String^ fieldName)
        {
          checkType(fieldName, PdxFieldTypes::BYTE_ARRAY, "byte[]");
          m_newPdxType->AddVariableLengthTypeField(fieldName, "byte[]", PdxFieldTypes::BYTE_ARRAY);
          int position = m_pdxType->GetFieldPosition(fieldName, m_offsetsBuffer, m_offsetSize, m_serializedLength);
          if (position != -1)
          {
            m_dataInput->AdvanceCursorPdx(position);
            array<Byte>^ retVal = PdxLocalReader::ReadByteArray(fieldName);
            m_dataInput->RewindCursorPdx(position);
            return retVal;
          }
          return nullptr;
        }

        array<SByte>^ PdxReaderWithTypeCollector::ReadSByteArray(String^ fieldName)
        {
          checkType(fieldName, PdxFieldTypes::BYTE_ARRAY, "byte[]");
          m_newPdxType->AddVariableLengthTypeField(fieldName, "byte[]", PdxFieldTypes::BYTE_ARRAY);
          int position = m_pdxType->GetFieldPosition(fieldName, m_offsetsBuffer, m_offsetSize, m_serializedLength);
          if (position != -1)
          {
            m_dataInput->AdvanceCursorPdx(position);
            array<SByte>^ retVal = PdxLocalReader::ReadSByteArray(fieldName);
            m_dataInput->RewindCursorPdx(position);
            return retVal;
          }
          return nullptr;
        }

        array<short>^ PdxReaderWithTypeCollector::ReadShortArray(String^ fieldName)
        {
          checkType(fieldName, PdxFieldTypes::SHORT_ARRAY, "short[]");
          m_newPdxType->AddVariableLengthTypeField(fieldName, "short[]", PdxFieldTypes::SHORT_ARRAY);
          int position = m_pdxType->GetFieldPosition(fieldName, m_offsetsBuffer, m_offsetSize, m_serializedLength);
          if (position != -1)
          {
            m_dataInput->AdvanceCursorPdx(position);
            array<Int16>^ retVal = PdxLocalReader::ReadShortArray(fieldName);
            m_dataInput->RewindCursorPdx(position);
            return retVal;
          }
          return nullptr;
        }

        array<System::UInt16>^ PdxReaderWithTypeCollector::ReadUnsignedShortArray(String^ fieldName)
        {
          checkType(fieldName, PdxFieldTypes::SHORT_ARRAY, "short[]");
          m_newPdxType->AddVariableLengthTypeField(fieldName, "short[]", PdxFieldTypes::SHORT_ARRAY);
          int position = m_pdxType->GetFieldPosition(fieldName, m_offsetsBuffer, m_offsetSize, m_serializedLength);
          if (position != -1)
          {
            m_dataInput->AdvanceCursorPdx(position);
            array<UInt16>^ retVal = PdxLocalReader::ReadUnsignedShortArray(fieldName);
            m_dataInput->RewindCursorPdx(position);
            return retVal;
          }
          return nullptr;
        }

        array<System::Int32>^ PdxReaderWithTypeCollector::ReadIntArray(String^ fieldName)
        {
          checkType(fieldName, PdxFieldTypes::INT_ARRAY, "int[]");
          m_newPdxType->AddVariableLengthTypeField(fieldName, "int[]", PdxFieldTypes::INT_ARRAY);
          int position = m_pdxType->GetFieldPosition(fieldName, m_offsetsBuffer, m_offsetSize, m_serializedLength);
          if (position != -1)
          {
            m_dataInput->AdvanceCursorPdx(position);
            array<Int32>^ retVal = PdxLocalReader::ReadIntArray(fieldName);
            m_dataInput->RewindCursorPdx(position);
            return retVal;
          }
          return nullptr;
        }

        array<System::UInt32>^ PdxReaderWithTypeCollector::ReadUnsignedIntArray(String^ fieldName)
        {
          checkType(fieldName, PdxFieldTypes::INT_ARRAY, "int[]");
          m_newPdxType->AddVariableLengthTypeField(fieldName, "int[]", PdxFieldTypes::INT_ARRAY);
          int position = m_pdxType->GetFieldPosition(fieldName, m_offsetsBuffer, m_offsetSize, m_serializedLength);
          if (position != -1)
          {
            m_dataInput->AdvanceCursorPdx(position);
            array<UInt32>^ retVal = PdxLocalReader::ReadUnsignedIntArray(fieldName);
            m_dataInput->RewindCursorPdx(position);
            return retVal;
          }
          return nullptr;
        }

        array<Int64>^ PdxReaderWithTypeCollector::ReadLongArray(String^ fieldName)
        {
          checkType(fieldName, PdxFieldTypes::LONG_ARRAY, "long[]");
          m_newPdxType->AddVariableLengthTypeField(fieldName, "long[]", PdxFieldTypes::LONG_ARRAY);
          int position = m_pdxType->GetFieldPosition(fieldName, m_offsetsBuffer, m_offsetSize, m_serializedLength);
          if (position != -1)
          {
            m_dataInput->AdvanceCursorPdx(position);
            array<Int64>^ retVal = PdxLocalReader::ReadLongArray(fieldName);
            m_dataInput->RewindCursorPdx(position);
            return retVal;
          }
          return nullptr;
        }

        array<System::UInt64>^ PdxReaderWithTypeCollector::ReadUnsignedLongArray(String^ fieldName)
        {
          checkType(fieldName, PdxFieldTypes::LONG_ARRAY, "long[]");
          m_newPdxType->AddVariableLengthTypeField(fieldName, "long[]", PdxFieldTypes::LONG_ARRAY);
          int position = m_pdxType->GetFieldPosition(fieldName, m_offsetsBuffer, m_offsetSize, m_serializedLength);
          if (position != -1)
          {
            m_dataInput->AdvanceCursorPdx(position);
            array<UInt64>^ retVal = PdxLocalReader::ReadUnsignedLongArray(fieldName);
            m_dataInput->RewindCursorPdx(position);
            return retVal;
          }
          return nullptr;
        }

        array<float>^ PdxReaderWithTypeCollector::ReadFloatArray(String^ fieldName)
        {
          checkType(fieldName, PdxFieldTypes::FLOAT_ARRAY, "float[]");
          m_newPdxType->AddVariableLengthTypeField(fieldName, "float[]", PdxFieldTypes::FLOAT_ARRAY);
          int position = m_pdxType->GetFieldPosition(fieldName, m_offsetsBuffer, m_offsetSize, m_serializedLength);
          if (position != -1)
          {
            m_dataInput->AdvanceCursorPdx(position);
            array<float>^ retVal = PdxLocalReader::ReadFloatArray(fieldName);
            m_dataInput->RewindCursorPdx(position);
            return retVal;
          }
          return nullptr;
        }

        array<double>^ PdxReaderWithTypeCollector::ReadDoubleArray(String^ fieldName)
        {
          checkType(fieldName, PdxFieldTypes::DOUBLE_ARRAY, "double[]");
          m_newPdxType->AddVariableLengthTypeField(fieldName, "double[]", PdxFieldTypes::DOUBLE_ARRAY);
          int position = m_pdxType->GetFieldPosition(fieldName, m_offsetsBuffer, m_offsetSize, m_serializedLength);
          if (position != -1)
          {
            m_dataInput->AdvanceCursorPdx(position);
            array<Double>^ retVal = PdxLocalReader::ReadDoubleArray(fieldName);
            m_dataInput->RewindCursorPdx(position);
            return retVal;
          }
          return nullptr;
        }

        array<String^>^ PdxReaderWithTypeCollector::ReadStringArray(String^ fieldName)
        {
          checkType(fieldName, PdxFieldTypes::STRING_ARRAY, "String[]");
          m_newPdxType->AddVariableLengthTypeField(fieldName, "String[]", PdxFieldTypes::STRING_ARRAY);
          int position = m_pdxType->GetFieldPosition(fieldName, m_offsetsBuffer, m_offsetSize, m_serializedLength);
          if (position != -1)
          {
            m_dataInput->AdvanceCursorPdx(position);
            array<String^>^ retVal = PdxLocalReader::ReadStringArray(fieldName);
            m_dataInput->RewindCursorPdx(position);
            return retVal;
          }
          return nullptr;
        }

        List<Object^>^ PdxReaderWithTypeCollector::ReadObjectArray(String^ fieldName)
        {
          checkType(fieldName, PdxFieldTypes::OBJECT_ARRAY, "Object[]");
          m_newPdxType->AddVariableLengthTypeField(fieldName, "Object[]", PdxFieldTypes::OBJECT_ARRAY);
          int position = m_pdxType->GetFieldPosition(fieldName, m_offsetsBuffer, m_offsetSize, m_serializedLength);
          if (position != -1)
          {
            m_dataInput->AdvanceCursorPdx(position);
            List<Object^>^ retVal = PdxLocalReader::ReadObjectArray(fieldName);
            m_dataInput->ResetPdx(m_startPosition);//force native as well
            m_dataInput->RewindCursorPdx(position);
            return retVal;
          }
          return nullptr;
        }

        array<array<Byte>^>^ PdxReaderWithTypeCollector::ReadArrayOfByteArrays(String^ fieldName)
        {
          checkType(fieldName, PdxFieldTypes::ARRAY_OF_BYTE_ARRAYS, "byte[][]");
          m_newPdxType->AddVariableLengthTypeField(fieldName, "byte[][]", PdxFieldTypes::ARRAY_OF_BYTE_ARRAYS);
          int position = m_pdxType->GetFieldPosition(fieldName, m_offsetsBuffer, m_offsetSize, m_serializedLength);
          if (position != -1)
          {
            m_dataInput->AdvanceCursorPdx(position);
            array<array<Byte>^>^ retVal = PdxLocalReader::ReadArrayOfByteArrays(fieldName);
            m_dataInput->RewindCursorPdx(position);
            return retVal;
          }
          return nullptr;
        }

      }  // namepsace Internal
    }  // namespace Client
  }  // namespace Geode
}  // namespace Apache
