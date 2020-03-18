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

#include "PdxInstanceFactoryImpl.hpp"
#include "PdxInstanceImpl.hpp"
#include "DotNetTypes.hpp"

using namespace System::Text;

namespace Apache
{
  namespace Geode
  {
    namespace Client
    {

      namespace Internal
      {

        namespace native = apache::geode::client;

        PdxInstanceFactoryImpl::PdxInstanceFactoryImpl(String^ className, Cache^ cache)
        {
          if (className == nullptr)
            throw gcnew IllegalStateException(
            "Classname should not be null.");
          m_pdxType = gcnew PdxType(className, false);
          m_FieldVsValues = gcnew Dictionary<String^, Object^>();
          m_created = false;
          m_cache = cache;
        }

        IPdxInstance^ PdxInstanceFactoryImpl::Create()
        {
          if (m_created)
          {
            throw gcnew IllegalStateException(
              "The IPdxInstanceFactory.Create() method can only be called once.");
          }
          //need to get typeid;
          IPdxInstance^ pi = gcnew PdxInstanceImpl(m_FieldVsValues, m_pdxType, m_cache);
          m_created = true;
          return pi;
        }

        IPdxInstanceFactory^ PdxInstanceFactoryImpl::WriteChar(String^ fieldName, Char value)
        {
          isFieldAdded(fieldName);
          m_pdxType->AddFixedLengthTypeField(fieldName, "char", PdxFieldTypes::CHAR, native::PdxTypes::CHAR_SIZE);
          m_FieldVsValues->Add(fieldName, value);
          return this;
        }

        IPdxInstanceFactory^ PdxInstanceFactoryImpl::WriteBoolean(String^ fieldName, Boolean value)
        {
          isFieldAdded(fieldName);
          m_pdxType->AddFixedLengthTypeField(fieldName, "boolean", PdxFieldTypes::BOOLEAN, native::PdxTypes::BOOLEAN_SIZE);
          m_FieldVsValues->Add(fieldName, value);
          return this;
        }

        IPdxInstanceFactory^ PdxInstanceFactoryImpl::WriteByte(String^ fieldName, SByte value)
        {
          isFieldAdded(fieldName);
          m_pdxType->AddFixedLengthTypeField(fieldName, "sbyte", PdxFieldTypes::BYTE, native::PdxTypes::BYTE_SIZE);
          m_FieldVsValues->Add(fieldName, value);
          return this;
        }

        IPdxInstanceFactory^ PdxInstanceFactoryImpl::WriteUnsignedByte(String^ fieldName, Byte value)
        {
          isFieldAdded(fieldName);
          m_pdxType->AddFixedLengthTypeField(fieldName, "byte", PdxFieldTypes::BYTE, native::PdxTypes::BYTE_SIZE);
          m_FieldVsValues->Add(fieldName, value);
          return this;
        }

        IPdxInstanceFactory^ PdxInstanceFactoryImpl::WriteShort(String^ fieldName, Int16 value)
        {
          isFieldAdded(fieldName);
          m_pdxType->AddFixedLengthTypeField(fieldName, "short", PdxFieldTypes::SHORT, native::PdxTypes::SHORT_SIZE);
          m_FieldVsValues->Add(fieldName, value);
          return this;
        }

        IPdxInstanceFactory^ PdxInstanceFactoryImpl::WriteUShort(String^ fieldName, UInt16 value)
        {
          isFieldAdded(fieldName);
          m_pdxType->AddFixedLengthTypeField(fieldName, "ushort", PdxFieldTypes::SHORT, native::PdxTypes::SHORT_SIZE);
          m_FieldVsValues->Add(fieldName, value);
          return this;
        }

        IPdxInstanceFactory^ PdxInstanceFactoryImpl::WriteInt(String^ fieldName, Int32 value)
        {
          isFieldAdded(fieldName);
          m_pdxType->AddFixedLengthTypeField(fieldName, "int", PdxFieldTypes::INT, native::PdxTypes::INTEGER_SIZE);
          m_FieldVsValues->Add(fieldName, value);
          return this;
        }

        IPdxInstanceFactory^ PdxInstanceFactoryImpl::WriteUInt(String^ fieldName, UInt32 value)
        {
          isFieldAdded(fieldName);
          m_pdxType->AddFixedLengthTypeField(fieldName, "uint", PdxFieldTypes::INT, native::PdxTypes::INTEGER_SIZE);
          m_FieldVsValues->Add(fieldName, value);
          return this;
        }

        IPdxInstanceFactory^ PdxInstanceFactoryImpl::WriteLong(String^ fieldName, Int64 value)
        {
          isFieldAdded(fieldName);
          m_pdxType->AddFixedLengthTypeField(fieldName, "long", PdxFieldTypes::LONG, native::PdxTypes::LONG_SIZE);
          m_FieldVsValues->Add(fieldName, value);
          return this;
        }

        IPdxInstanceFactory^ PdxInstanceFactoryImpl::WriteULong(String^ fieldName, UInt64 value)
        {
          isFieldAdded(fieldName);
          m_pdxType->AddFixedLengthTypeField(fieldName, "ulong", PdxFieldTypes::LONG, native::PdxTypes::LONG_SIZE);
          m_FieldVsValues->Add(fieldName, value);
          return this;
        }

        IPdxInstanceFactory^ PdxInstanceFactoryImpl::WriteFloat(String^ fieldName, float value)
        {
          isFieldAdded(fieldName);
          m_pdxType->AddFixedLengthTypeField(fieldName, "float", PdxFieldTypes::FLOAT, native::PdxTypes::FLOAT_SIZE);
          m_FieldVsValues->Add(fieldName, value);
          return this;
        }

        IPdxInstanceFactory^ PdxInstanceFactoryImpl::WriteDouble(String^ fieldName, double value)
        {
          isFieldAdded(fieldName);
          m_pdxType->AddFixedLengthTypeField(fieldName, "double", PdxFieldTypes::DOUBLE, native::PdxTypes::DOUBLE_SIZE);
          m_FieldVsValues->Add(fieldName, value);
          return this;
        }

        IPdxInstanceFactory^ PdxInstanceFactoryImpl::WriteDate(String^ fieldName, System::DateTime value)
        {
          isFieldAdded(fieldName);
          m_pdxType->AddFixedLengthTypeField(fieldName, "Date", PdxFieldTypes::DATE, native::PdxTypes::DATE_SIZE);
          m_FieldVsValues->Add(fieldName, value);
          return this;
        }

        IPdxInstanceFactory^ PdxInstanceFactoryImpl::WriteString(String^ fieldName, String^ value)
        {
          isFieldAdded(fieldName);
          m_pdxType->AddVariableLengthTypeField(fieldName, "String", PdxFieldTypes::STRING);
          m_FieldVsValues->Add(fieldName, value);
          return this;
        }

        IPdxInstanceFactory^ PdxInstanceFactoryImpl::WriteObject(String^ fieldName, Object^ value)
        {
          isFieldAdded(fieldName);
          m_pdxType->AddVariableLengthTypeField(fieldName, /*obj->GetType()->FullName*/"Object", PdxFieldTypes::OBJECT);
          m_FieldVsValues->Add(fieldName, value);
          return this;
        }

        IPdxInstanceFactory^ PdxInstanceFactoryImpl::WriteBooleanArray(String^ fieldName, array<Boolean>^ value)
        {
          isFieldAdded(fieldName);
          m_pdxType->AddVariableLengthTypeField(fieldName, "bool[]", PdxFieldTypes::BOOLEAN_ARRAY);
          m_FieldVsValues->Add(fieldName, value);
          return this;
        }

        IPdxInstanceFactory^ PdxInstanceFactoryImpl::WriteCharArray(String^ fieldName, array<Char>^ value)
        {
          isFieldAdded(fieldName);
          m_pdxType->AddVariableLengthTypeField(fieldName, "char[]", PdxFieldTypes::CHAR_ARRAY);
          m_FieldVsValues->Add(fieldName, value);
          return this;
        }

        IPdxInstanceFactory^ PdxInstanceFactoryImpl::WriteByteArray(String^ fieldName, array<Byte>^ value)
        {
          isFieldAdded(fieldName);
          m_pdxType->AddVariableLengthTypeField(fieldName, "byte[]", PdxFieldTypes::BYTE_ARRAY);
          m_FieldVsValues->Add(fieldName, value);
          return this;
        }

        IPdxInstanceFactory^ PdxInstanceFactoryImpl::WriteSByteArray(String^ fieldName, array<SByte>^ value)
        {
          isFieldAdded(fieldName);
          m_pdxType->AddVariableLengthTypeField(fieldName, "sbyte[]", PdxFieldTypes::BYTE_ARRAY);
          m_FieldVsValues->Add(fieldName, value);
          return this;
        }

        IPdxInstanceFactory^ PdxInstanceFactoryImpl::WriteShortArray(String^ fieldName, array<Int16>^ value)
        {
          isFieldAdded(fieldName);
          m_pdxType->AddVariableLengthTypeField(fieldName, "short[]", PdxFieldTypes::SHORT_ARRAY);
          m_FieldVsValues->Add(fieldName, value);
          return this;
        }

        IPdxInstanceFactory^ PdxInstanceFactoryImpl::WriteUShortArray(String^ fieldName, array<UInt16>^ value)
        {
          isFieldAdded(fieldName);
          m_pdxType->AddVariableLengthTypeField(fieldName, "ushort[]", PdxFieldTypes::SHORT_ARRAY);
          m_FieldVsValues->Add(fieldName, value);
          return this;
        }

        IPdxInstanceFactory^ PdxInstanceFactoryImpl::WriteIntArray(String^ fieldName, array<Int32>^ value)
        {
          isFieldAdded(fieldName);
          m_pdxType->AddVariableLengthTypeField(fieldName, "int[]", PdxFieldTypes::INT_ARRAY);
          m_FieldVsValues->Add(fieldName, value);
          return this;
        }

        IPdxInstanceFactory^ PdxInstanceFactoryImpl::WriteUIntArray(String^ fieldName, array<UInt32>^ value)
        {
          isFieldAdded(fieldName);
          m_pdxType->AddVariableLengthTypeField(fieldName, "uint[]", PdxFieldTypes::INT_ARRAY);
          m_FieldVsValues->Add(fieldName, value);
          return this;
        }

        IPdxInstanceFactory^ PdxInstanceFactoryImpl::WriteLongArray(String^ fieldName, array<Int64>^ value)
        {
          isFieldAdded(fieldName);
          m_pdxType->AddVariableLengthTypeField(fieldName, "long[]", PdxFieldTypes::LONG_ARRAY);
          m_FieldVsValues->Add(fieldName, value);
          return this;
        }

        IPdxInstanceFactory^ PdxInstanceFactoryImpl::WriteULongArray(String^ fieldName, array<UInt64>^ value)
        {
          isFieldAdded(fieldName);
          m_pdxType->AddVariableLengthTypeField(fieldName, "ulong[]", PdxFieldTypes::LONG_ARRAY);
          m_FieldVsValues->Add(fieldName, value);
          return this;
        }

        IPdxInstanceFactory^ PdxInstanceFactoryImpl::WriteFloatArray(String^ fieldName, array<float>^ value)
        {
          isFieldAdded(fieldName);
          m_pdxType->AddVariableLengthTypeField(fieldName, "float[]", PdxFieldTypes::FLOAT_ARRAY);
          m_FieldVsValues->Add(fieldName, value);
          return this;
        }

        IPdxInstanceFactory^ PdxInstanceFactoryImpl::WriteDoubleArray(String^ fieldName, array<double>^ value)
        {
          isFieldAdded(fieldName);
          m_pdxType->AddVariableLengthTypeField(fieldName, "double[]", PdxFieldTypes::DOUBLE_ARRAY);
          m_FieldVsValues->Add(fieldName, value);
          return this;
        }

        IPdxInstanceFactory^ PdxInstanceFactoryImpl::WriteStringArray(String^ fieldName, array<String^>^ value)
        {
          isFieldAdded(fieldName);
          m_pdxType->AddVariableLengthTypeField(fieldName, "String[]", PdxFieldTypes::STRING_ARRAY);
          m_FieldVsValues->Add(fieldName, value);
          return this;
        }

        IPdxInstanceFactory^ PdxInstanceFactoryImpl::WriteObjectArray(String^ fieldName, System::Collections::Generic::List<Object^>^ value)
        {
          isFieldAdded(fieldName);
          m_pdxType->AddVariableLengthTypeField(fieldName, "Object[]", PdxFieldTypes::OBJECT_ARRAY);
          m_FieldVsValues->Add(fieldName, value);
          return this;
        }

        IPdxInstanceFactory^ PdxInstanceFactoryImpl::WriteArrayOfByteArrays(String^ fieldName, array<array<Byte>^>^ value)
        {
          isFieldAdded(fieldName);
          m_pdxType->AddVariableLengthTypeField(fieldName, "byte[][]", PdxFieldTypes::ARRAY_OF_BYTE_ARRAYS);
          m_FieldVsValues->Add(fieldName, value);
          return this;
        }

        IPdxInstanceFactory^ PdxInstanceFactoryImpl::WriteArrayOfSByteArrays(String^ fieldName, array<array<SByte>^>^ value)
        {
          isFieldAdded(fieldName);
          m_pdxType->AddVariableLengthTypeField(fieldName, "sbyte[][]", PdxFieldTypes::ARRAY_OF_BYTE_ARRAYS);
          m_FieldVsValues->Add(fieldName, value);
          return this;
        }

        IPdxInstanceFactory^ PdxInstanceFactoryImpl::WriteField(String^ fieldName, Object^ fieldValue, Type^ type)
        {
          isFieldAdded(fieldName);
          if (type->Equals(DotNetTypes::IntType) || type->Equals(DotNetTypes::UIntType))
          {
            return this->WriteInt(fieldName, (int)fieldValue);
          }
          else if (type->Equals(DotNetTypes::StringType))
          {
            return this->WriteString(fieldName, (String^)fieldValue);
          }
          else if (type->Equals(DotNetTypes::BooleanType))
          {
            return this->WriteBoolean(fieldName, (bool)fieldValue);
          }
          else if (type->Equals(DotNetTypes::FloatType))
          {
            return this->WriteFloat(fieldName, (float)fieldValue);
          }
          else if (type->Equals(DotNetTypes::DoubleType))
          {
            return this->WriteDouble(fieldName, (double)fieldValue);
          }
          else if (type->Equals(DotNetTypes::CharType))
          {
            return this->WriteChar(fieldName, (Char)fieldValue);
          }
          else if (type->Equals(DotNetTypes::SByteType) || type->Equals(DotNetTypes::ByteType))
          {
            return this->WriteByte(fieldName, (SByte)fieldValue);
          }
          else if (type->Equals(DotNetTypes::ShortType) || type->Equals(DotNetTypes::UShortType))
          {
            return this->WriteShort(fieldName, (short)fieldValue);
          }
          else if (type->Equals(DotNetTypes::LongType) || type->Equals(DotNetTypes::ULongType))
          {
            return this->WriteLong(fieldName, (Int64)fieldValue);
          }
          else if (type->Equals(DotNetTypes::ByteArrayType) || type->Equals(DotNetTypes::SByteArrayType))
          {
            return this->WriteByteArray(fieldName, (array<Byte>^)fieldValue);
          }
          else if (type->Equals(DotNetTypes::DoubleArrayType))
          {
            return this->WriteDoubleArray(fieldName, (array<double>^)fieldValue);
          }
          else if (type->Equals(DotNetTypes::FloatArrayType))
          {
            return this->WriteFloatArray(fieldName, (array<float>^)fieldValue);
          }
          else if (type->Equals(DotNetTypes::ShortArrayType) || type->Equals(DotNetTypes::UShortArrayType))
          {
            return this->WriteShortArray(fieldName, (array<Int16>^)fieldValue);
          }
          else if (type->Equals(DotNetTypes::IntArrayType) || type->Equals(DotNetTypes::UIntArrayType))
          {
            return this->WriteIntArray(fieldName, (array<System::Int32>^)fieldValue);
          }
          else if (type->Equals(DotNetTypes::LongArrayType) || type->Equals(DotNetTypes::ULongArrayType))
          {
            return this->WriteLongArray(fieldName, (array<Int64>^)fieldValue);
          }
          else if (type->Equals(DotNetTypes::BoolArrayType))
          {
            return this->WriteBooleanArray(fieldName, (array<bool>^)fieldValue);
          }
          else if (type->Equals(DotNetTypes::CharArrayType))
          {
            return this->WriteCharArray(fieldName, (array<Char>^)fieldValue);
          }
          else if (type->Equals(DotNetTypes::StringArrayType))
          {
            return this->WriteStringArray(fieldName, (array<String^>^)fieldValue);
          }
          else if (type->Equals(DotNetTypes::DateType))
          {
            return this->WriteDate(fieldName, (DateTime)fieldValue);
          }
          else if (type->Equals(DotNetTypes::ByteArrayOfArrayType) || type->Equals(DotNetTypes::SByteArrayOfArrayType))
          {
            return this->WriteArrayOfByteArrays(fieldName, (array<array<Byte>^>^)fieldValue);
          }
          else if (type->Equals(DotNetTypes::ObjectArrayType))
          {
            return this->WriteObjectArray(fieldName, safe_cast<System::Collections::Generic::List<Object^>^>(fieldValue));
          }
          else if (type->Equals(DotNetTypes::GuidType))
          {
            return this->WriteString(fieldName, fieldValue->ToString());
          }
          else
          {
            return this->WriteObject(fieldName, fieldValue);
            //throw gcnew IllegalStateException("WriteField unable to serialize  " 
            //																	+ fieldName + " of " + type); 
          }
          // return this;
        }

        IPdxInstanceFactory^ PdxInstanceFactoryImpl::MarkIdentityField(String^ fieldName)
        {
          PdxFieldType^ pft = m_pdxType->GetPdxField(fieldName);

          if (pft == nullptr)
          {
            throw gcnew IllegalStateException(
              "Field must be added before calling MarkIdentityField ");
          }

          pft->IdentityField = true;
          return this;
        }

        void PdxInstanceFactoryImpl::isFieldAdded(String^ fieldName)
        {
          if (fieldName == nullptr || fieldName->Length == 0 || m_FieldVsValues->ContainsKey(fieldName))
          {
            throw gcnew IllegalStateException(
              "Field: " + fieldName + " either already added into PdxInstanceFactory or it is null");
          }  // namespace Client
        }  // namespace Geode
      }  // namespace Apache

    }
  }
}
