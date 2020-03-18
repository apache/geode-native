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


#include "PdxRemoteWriter.hpp"
#include "PdxTypeRegistry.hpp"
using namespace System;

namespace Apache
{
  namespace Geode
  {
    namespace Client
    {

      namespace Internal
      {
        /*
        * Purpose of this class to map local type to remote type
        * If remote type will have extra field then we have to merge that field
        * if local type has field then need to put on stream
        */

        void PdxRemoteWriter::writePreserveData()
        {
          m_currentDataIdx++;//it starts from -1

          if (m_preserveData != nullptr)
          {
            while (m_currentDataIdx < m_remoteTolocalMap->Length)
            {
              if (m_remoteTolocalMap[m_currentDataIdx] == -1)//need to add preserve data with offset
              {
                PdxLocalWriter::AddOffset();
                m_dataOutput->WriteBytesOnly(m_preserveData[m_preserveDataIdx++]);
                m_currentDataIdx++;
              }
              else if (m_remoteTolocalMap[m_currentDataIdx] == -2)//need to add preserve data WITHOUT offset
              {
                m_dataOutput->WriteBytesOnly(m_preserveData[m_preserveDataIdx++]);
                m_currentDataIdx++;
              }
              else
              {
                break; //continue writing local data..
              }
            }
          }
        }

        void PdxRemoteWriter::initialize()
        {
          //this is default case
          if (m_preserveData == nullptr)
          {
            //this needs to fix for IPdxTypeMapper
            m_pdxType = GetLocalPdxType(m_pdxClassName);
            m_offsets = gcnew array<int>(m_pdxType->NumberOfVarLenFields);
          }
        }

        bool PdxRemoteWriter::isFieldWritingStarted()
        {
          return m_currentDataIdx != -1;//field writing NOT started. do we need this??
        }

        IPdxWriter^ PdxRemoteWriter::WriteUnreadFields(IPdxUnreadFields^ unread)
        {
          PdxLocalWriter::WriteUnreadFields(unread);
          m_remoteTolocalMap = m_pdxType->GetRemoteToLocalMap(m_dataOutput->Cache);
          return this;
        }

        IPdxWriter^ PdxRemoteWriter::WriteByte(String^ fieldName, SByte value)
        {
          writePreserveData();
          PdxLocalWriter::WriteByte(fieldName, value);
          return this;
        }

        IPdxWriter^ PdxRemoteWriter::WriteUnsignedByte(String^ fieldName, Byte value)
        {
          writePreserveData();
          PdxLocalWriter::WriteUnsignedByte(fieldName, value);
          return this;

        }

        IPdxWriter^ PdxRemoteWriter::WriteBoolean(String^ fieldName, bool value)
        {
          writePreserveData();
          PdxLocalWriter::WriteBoolean(fieldName, value);
          return this;

        }

        IPdxWriter^ PdxRemoteWriter::WriteChar(String^ fieldName, Char value)
        {
          writePreserveData();
          PdxLocalWriter::WriteChar(fieldName, value);
          return this;
        }

        IPdxWriter^ PdxRemoteWriter::WriteUShort(String^ fieldName, System::UInt16 value)
        {
          writePreserveData();
          PdxLocalWriter::WriteUShort(fieldName, value);
          return this;

        }

        IPdxWriter^ PdxRemoteWriter::WriteUInt(String^ fieldName, System::UInt32 value)
        {
          writePreserveData();
          PdxLocalWriter::WriteUInt(fieldName, value);
          return this;

        }

        IPdxWriter^ PdxRemoteWriter::WriteULong(String^ fieldName, System::UInt64 value)
        {
          writePreserveData();
          PdxLocalWriter::WriteULong(fieldName, value);
          return this;

        }

        IPdxWriter^ PdxRemoteWriter::WriteShort(String^ fieldName, System::Int16 value)
        {
          writePreserveData();
          PdxLocalWriter::WriteShort(fieldName, value);
          return this;
        }

        IPdxWriter^ PdxRemoteWriter::WriteInt(String^ fieldName, System::Int32 value)
        {
          writePreserveData();
          PdxLocalWriter::WriteInt(fieldName, value);
          return this;
        }

        IPdxWriter^ PdxRemoteWriter::WriteLong(String^ fieldName, Int64 value)
        {
          writePreserveData();
          PdxLocalWriter::WriteLong(fieldName, value);
          return this;
        }

        IPdxWriter^ PdxRemoteWriter::WriteFloat(String^ fieldName, float value)
        {
          writePreserveData();
          PdxLocalWriter::WriteFloat(fieldName, value);
          return this;
        }

        IPdxWriter^ PdxRemoteWriter::WriteDouble(String^ fieldName, double value)
        {
          writePreserveData();
          PdxLocalWriter::WriteDouble(fieldName, value);
          return this;
        }

        IPdxWriter^ PdxRemoteWriter::WriteString(String^ fieldName, String^ value)
        {
          writePreserveData();
          PdxLocalWriter::WriteString(fieldName, value);
          return this;
        }

        IPdxWriter^ PdxRemoteWriter::WriteUTFHuge(String^ fieldName, String^ value)
        {
          writePreserveData();
          PdxLocalWriter::WriteUTFHuge(fieldName, value);
          return this;
        }

        IPdxWriter^ PdxRemoteWriter::WriteASCIIHuge(String^ fieldName, String^ value)
        {
          writePreserveData();
          PdxLocalWriter::WriteASCIIHuge(fieldName, value);
          return this;
        }

        IPdxWriter^ PdxRemoteWriter::WriteObject(String^ fieldName, Object^ obj)
        {
          writePreserveData();
          PdxLocalWriter::WriteObject(fieldName, obj);
          return this;
        }

        //TODO:
        //IPdxWriter^ PdxRemoteWriter::WriteMap( String^ fieldName, System::Collections::IDictionary^ map );

        IPdxWriter^ PdxRemoteWriter::WriteCollection(String^ fieldName, System::Collections::IList^ obj)
        {
          writePreserveData();
          PdxLocalWriter::WriteCollection(fieldName, obj);
          return this;
        }

        IPdxWriter^ PdxRemoteWriter::WriteDate(String^ fieldName, System::DateTime date)
        {
          writePreserveData();
          PdxLocalWriter::WriteDate(fieldName, date);
          return this;
        }

        IPdxWriter^ PdxRemoteWriter::WriteBooleanArray(String^ fieldName, array<bool>^ boolArray)
        {
          writePreserveData();
          PdxLocalWriter::WriteBooleanArray(fieldName, boolArray);
          return this;
        }

        IPdxWriter^ PdxRemoteWriter::WriteCharArray(String^ fieldName, array<Char>^ charArray)
        {
          writePreserveData();
          PdxLocalWriter::WriteCharArray(fieldName, charArray);
          return this;
        }

        IPdxWriter^ PdxRemoteWriter::WriteByteArray(String^ fieldName, array<Byte>^ byteArray)
        {
          writePreserveData();
          PdxLocalWriter::WriteByteArray(fieldName, byteArray);
          return this;
        }

        IPdxWriter^ PdxRemoteWriter::WriteSByteArray(String^ fieldName, array<SByte>^ sbyteArray)
        {
          writePreserveData();
          PdxLocalWriter::WriteSByteArray(fieldName, sbyteArray);
          return this;
        }

        IPdxWriter^ PdxRemoteWriter::WriteShortArray(String^ fieldName, array<System::Int16>^ shortArray)
        {
          writePreserveData();
          PdxLocalWriter::WriteShortArray(fieldName, shortArray);
          return this;
        }

        IPdxWriter^ PdxRemoteWriter::WriteUShortArray(String^ fieldName, array<System::UInt16>^ ushortArray)
        {
          writePreserveData();
          PdxLocalWriter::WriteUShortArray(fieldName, ushortArray);
          return this;
        }

        IPdxWriter^ PdxRemoteWriter::WriteIntArray(String^ fieldName, array<System::Int32>^ intArray)
        {
          writePreserveData();
          PdxLocalWriter::WriteIntArray(fieldName, intArray);
          return this;
        }

        IPdxWriter^ PdxRemoteWriter::WriteUIntArray(String^ fieldName, array<System::UInt32>^ uintArray)
        {
          writePreserveData();
          PdxLocalWriter::WriteUIntArray(fieldName, uintArray);
          return this;
        }

        IPdxWriter^ PdxRemoteWriter::WriteLongArray(String^ fieldName, array<Int64>^ longArray)
        {
          writePreserveData();
          PdxLocalWriter::WriteLongArray(fieldName, longArray);
          return this;
        }

        IPdxWriter^ PdxRemoteWriter::WriteULongArray(String^ fieldName, array<System::UInt64>^ ulongArray)
        {
          writePreserveData();
          PdxLocalWriter::WriteULongArray(fieldName, ulongArray);
          return this;
        }

        IPdxWriter^ PdxRemoteWriter::WriteFloatArray(String^ fieldName, array<float>^ floatArray)
        {
          writePreserveData();
          PdxLocalWriter::WriteFloatArray(fieldName, floatArray);
          return this;
        }

        IPdxWriter^ PdxRemoteWriter::WriteDoubleArray(String^ fieldName, array<double>^ doubleArray)
        {
          writePreserveData();
          PdxLocalWriter::WriteDoubleArray(fieldName, doubleArray);
          return this;
        }

        IPdxWriter^ PdxRemoteWriter::WriteStringArray(String^ fieldName, array<String^>^ stringArray)
        {
          writePreserveData();
          PdxLocalWriter::WriteStringArray(fieldName, stringArray);
          return this;
        }

        IPdxWriter^ PdxRemoteWriter::WriteObjectArray(String^ fieldName, List<Object^>^ objectArray)
        {
          writePreserveData();
          PdxLocalWriter::WriteObjectArray(fieldName, objectArray);
          return this;
        }

        IPdxWriter^ PdxRemoteWriter::WriteArrayOfByteArrays(String^ fieldName, array<array<Byte>^>^ byteArrays)
        {
          writePreserveData();
          PdxLocalWriter::WriteArrayOfByteArrays(fieldName, byteArrays);
          return this;
        }

        //TODO:
        //IPdxWriter^ PdxRemoteWriter::WriteEnum(String^ fieldName, Enum e) ;
        //IPdxWriter^ PdxRemoteWriter::WriteInetAddress(String^ fieldName, InetAddress address);

      }  // namespace Internal
    }  // namespace Client
  }  // namespace Geode
}  // namespace Apache

