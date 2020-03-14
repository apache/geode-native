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

#include "../geode_defs.hpp"
#include "PdxLocalWriter.hpp"
#include "PdxType.hpp"

using namespace System;

namespace Apache
{
  namespace Geode
  {
    namespace Client
    {

      ref class DataOutput;

      namespace Internal
      {
        ref class PdxWriterWithTypeCollector : public PdxLocalWriter
        {
        private:

          System::Collections::Generic::List<Int32>^ m_offsets;
          void initialize();

        public:

          PdxWriterWithTypeCollector(DataOutput^ dataOutput, String^ pdxClassName)
            :PdxLocalWriter(dataOutput, nullptr)
          {
            m_pdxClassName = pdxClassName;

            initialize();
          }

          virtual void AddOffset() override;

          virtual void EndObjectWriting() override;

          virtual Int32 calculateLenWithOffsets() override;

          virtual void WriteOffsets(Int32 len) override;

          virtual bool isFieldWritingStarted() override;

          property PdxType^ PdxLocalType
          {
            PdxType^ get() { return m_pdxType; }
          }

          /// <summary>
          /// Write a byte to the <c>IPdxWriter</c>.
          /// </summary>
          /// <param name="fieldName">The name of the field associated with the value.</param>
          /// <param name="value">The byte to write.</param>
          virtual IPdxWriter^ WriteByte(String^ fieldName, Byte value) override;

          /// <summary>
          /// Write a signed byte to the <c>IPdxWriter</c>.
          /// </summary>
          /// <param name="fieldName">The name of the field associated with the value.</param>
          /// <param name="value">The signed byte to write.</param>
          virtual IPdxWriter^ WriteSByte(String^ fieldName, SByte value) override;

          /// <summary>
          /// Write a boolean value to the <c>IPdxWriter</c>.
          /// </summary>
          /// <param name="fieldName">The name of the field associated with the value.</param>
          /// <param name="value">The boolean value to write.</param>
          virtual IPdxWriter^ WriteBoolean(String^ fieldName, Boolean value) override;

          /// <summary>
          /// Write a char value to the <c>IPdxWriter</c>.
          /// </summary>
          /// <param name="fieldName">The name of the field associated with the value.</param>
          /// <param name="value">The char value to write.</param>
          virtual IPdxWriter^ WriteChar(String^ fieldName, Char value) override;

          /// <summary>
          /// Write an unsigned short integer (System::Int16) to the <c>IPdxWriter</c>.
          /// </summary>
          /// <param name="fieldName">The name of the field associated with the value.</param>
          /// <param name="value">The unsigned 16-bit integer to write.</param>
          virtual IPdxWriter^ WriteUShort(String^ fieldName, System::UInt16 value) override;

          /// <summary>
          /// Write an unsigned 32-bit integer to the <c>IPdxWriter</c>.
          /// </summary>
          /// <param name="fieldName">The name of the field associated with the value.</param>
          /// <param name="value">The unsigned 32-bit integer to write.</param>
          virtual IPdxWriter^ WriteUInt(String^ fieldName, System::UInt32 value) override;

          /// <summary>
          /// Write an unsigned 64-bit integer to the <c>IPdxWriter</c>.
          /// </summary>
          /// <param name="fieldName">The name of the field associated with the value.</param>
          /// <param name="value">The unsigned 64-bit integer to write.</param>
          virtual IPdxWriter^ WriteULong(String^ fieldName, System::UInt64 value) override;

          /// <summary>
          /// Write a 16-bit integer to the <c>IPdxWriter</c>.
          /// </summary>
          /// <param name="fieldName">The name of the field associated with the value.</param>
          /// <param name="value">The 16-bit integer to write.</param>
          virtual IPdxWriter^ WriteShort(String^ fieldName, short value) override;

          /// <summary>
          /// Write a 32-bit integer to the <c>IPdxWriter</c>.
          /// </summary>
          /// <param name="fieldName">The name of the field associated with the value.</param>
          /// <param name="value">The 32-bit integer to write.</param>
          virtual IPdxWriter^ WriteInt(String^ fieldName, Int32 value) override;

          /// <summary>
          /// Write a 64-bit integer to the <c>IPdxWriter</c>.
          /// </summary>
          /// <param name="fieldName">The name of the field associated with the value.</param>
          /// <param name="value">The 64-bit integer to write.</param>
          virtual IPdxWriter^ WriteLong(String^ fieldName, Int64 value) override;

          /// <summary>
          /// Write a float to the <c>IPdxWriter</c>.
          /// </summary>
          /// <param name="fieldName">The name of the field associated with the value.</param>
          /// <param name="value">The float value to write.</param>
          virtual IPdxWriter^ WriteFloat(String^ fieldName, float value) override;

          /// <summary>
          /// Write a double precision real number to the <c>IPdxWriter</c>.
          /// </summary>
          /// <param name="fieldName">The name of the field associated with the value.</param>
          /// <param name="value">
          /// The double precision real number to write.
          /// </param>
          virtual IPdxWriter^ WriteDouble(String^ fieldName, double value) override;

          /// <summary>
          /// Write a string using java-modified UTF-8 encoding to
          /// <c>IPdxWriter</c>.
          /// The maximum length supported is 2^16-1 beyond which the string
          /// shall be truncated.
          /// </summary>
          /// <param name="fieldName">The name of the field associated with the value.</param>
          /// <param name="value">The UTF encoded string to write.</param>
          virtual IPdxWriter^ WriteString(String^ fieldName, String^ value) override;

          /// <summary>
          /// Write a string using java-modified UTF-8 encoding to
          /// <c>IPdxWriter</c>.
          /// Length should be more than 2^16 -1. 
          /// </summary>
          /// <param name="fieldName">The name of the field associated with the value.</param>
          /// <param name="value">The UTF encoded string to write.</param>
          virtual IPdxWriter^ WriteUTFHuge(String^ fieldName, String^ value) override;

          /// <summary>
          /// Write a string(only ASCII char) to
          /// <c>IPdxWriter</c>.
          /// Length should be more than 2^16 -1.
          /// </summary>
          /// <param name="fieldName">The name of the field associated with the value.</param>
          /// <param name="value">The UTF encoded string to write.</param>
          virtual IPdxWriter^ WriteASCIIHuge(String^ fieldName, String^ value) override;

          /// <summary>
          /// Write an <c>Object</c> object to the <c>IPdxWriter</c>.
          /// </summary>
          /// <param name="fieldName">The name of the field associated with the value.</param>
          /// <param name="obj">The object to write.</param>
          virtual IPdxWriter^ WriteObject(String^ fieldName, Object^ obj) override;

          /// <summary>
          /// Write a date to the <c>IPdxWriter</c>.
          /// </summary>
          /// <param name="fieldName">The name of the field associated with the value.</param>
          /// <param name="date">The date to write.</param>
          virtual IPdxWriter^ WriteDate(String^ fieldName, System::DateTime date) override;

          /// <summary>
          /// Write a boolean array to the <c>IPdxWriter</c>.
          /// </summary>
          /// <param name="fieldName">The name of the field associated with the value.</param>
          /// <param name="boolArray">The boolArray to write.</param>
          virtual IPdxWriter^ WriteBooleanArray(String^ fieldName, array<Boolean>^ boolArray) override;

          /// <summary>
          /// Write a char array to the <c>IPdxWriter</c>.
          /// </summary>
          /// <param name="fieldName">The name of the field associated with the value.</param>
          /// <param name="charArray">The charArray to write.</param>
          virtual IPdxWriter^ WriteCharArray(String^ fieldName, array<Char>^ charArray)  override;

          /// <summary>
          /// Write an unsigned byte array to the <c>IPdxWriter</c>.
          /// </summary>
          /// <param name="fieldName">The name of the field associated with the value.</param>
          /// <param name="byteArray">The byteArray to write.</param>
          virtual IPdxWriter^ WriteByteArray(String^ fieldName, array<Byte>^ byteArray)  override;

          /// <summary>
          /// Write a signed byte arry to the <c>IPdxWriter</c>.
          /// </summary>
          /// <param name="fieldName">The name of the field associated with the value.</param>
          /// <param name="sbyteArray">The sbyteArray to write.</param>
          virtual IPdxWriter^ WriteSByteArray(String^ fieldName, array<SByte>^ sbyteArray) override;

          /// <summary>
          /// Write a 16 bit integer array to the <c>IPdxWriter</c>.
          /// </summary>
          /// <param name="fieldName">The name of the field associated with the value.</param>
          /// <param name="shortArray">The shortArray to write.</param>
          virtual IPdxWriter^ WriteShortArray(String^ fieldName, array<short>^ shortArray) override;

          /// <summary>
          /// Write an unsigned 16 bit integer array to the <c>IPdxWriter</c>.
          /// </summary>
          /// <param name="fieldName">The name of the field associated with the value.</param>
          /// <param name="ushortArray">The ushortArray to write.</param>
          virtual IPdxWriter^ WriteUShortArray(String^ fieldName, array<System::UInt16>^ ushortArray) override;

          /// <summary>
          /// Write an integer array to the <c>IPdxWriter</c>.
          /// </summary>
          /// <param name="fieldName">The name of the field associated with the value.</param>
          /// <param name="intArray">The intArray to write.</param>
          virtual IPdxWriter^ WriteIntArray(String^ fieldName, array<System::Int32>^ intArray) override;

          /// <summary>
          /// Write an unsigned integer array to the <c>IPdxWriter</c>.
          /// </summary>
          /// <param name="fieldName">The name of the field associated with the value.</param>
          /// <param name="uintArray">The uintArray to write.</param>
          virtual IPdxWriter^ WriteUIntArray(String^ fieldName, array<System::UInt32>^ uintArray) override;

          /// <summary>
          /// Write a 64 bit integer array to the <c>IPdxWriter</c>.
          /// </summary>
          /// <param name="fieldName">The name of the field associated with the value.</param>
          /// <param name="longArray">The longArray to write.</param>
          virtual IPdxWriter^ WriteLongArray(String^ fieldName, array<Int64>^ longArray) override;

          /// <summary>
          /// Write an unsigned 64 bit integer array to the <c>IPdxWriter</c>.
          /// </summary>
          /// <param name="fieldName">The name of the field associated with the value.</param>
          /// <param name="ulongArray">The ulongArray to write.</param>
          virtual IPdxWriter^ WriteULongArray(String^ fieldName, array<System::UInt64>^ ulongArray) override;

          /// <summary>
          /// Write an collection to the <c>IPdxWriter</c>.
          /// </summary>
          /// <param name="fieldName">The name of the field associated with the value.</param>
          /// <param name="floatArray">The floatArray to write.</param>
          virtual IPdxWriter^ WriteFloatArray(String^ fieldName, array<float>^ floatArray) override;

          /// <summary>
          /// Write an collection to the <c>IPdxWriter</c>.
          /// </summary>
          /// <param name="fieldName">The name of the field associated with the value.</param>
          /// <param name="doubleArray">The doubleArray to write.</param>
          virtual IPdxWriter^ WriteDoubleArray(String^ fieldName, array<double>^ doubleArray) override;

          /// <summary>
          /// Write an collection to the <c>IPdxWriter</c>.
          /// </summary>
          /// <param name="fieldName">The name of the field associated with the value.</param>
          /// <param name="stringArray">The stringArray to write.</param>
          virtual IPdxWriter^ WriteStringArray(String^ fieldName, array<String^>^ stringArray) override;

          /// <summary>
          /// Write an collection to the <c>IPdxWriter</c>.
          /// </summary>
          /// <param name="fieldName">The name of the field associated with the value.</param>
          /// <param name="objectArray">The objectArray to write.</param>
          virtual IPdxWriter^ WriteObjectArray(String^ fieldName, List<Object^>^ objectArray) override;

          /// <summary>
          /// Write an array of byte arrays to the <c>IPdxWriter</c>.
          /// </summary>
          /// <param name="fieldName">The name of the field associated with the value.</param>
          /// <param name="byteArrays">The array of byteArrays to write.</param>
          virtual IPdxWriter^ WriteArrayOfByteArrays(String^ fieldName, array<array<Byte>^>^ byteArrays) override;

          /// <summary>
          /// Write an array of signed byte arrays to the <c>IPdxWriter</c>.
          /// </summary>
          /// <param name="fieldName">The name of the field associated with the value.</param>
          /// <param name="byteArrays">The array of signedbyteArrays to write.</param>
          virtual IPdxWriter^ WriteArrayOfSByteArrays(String^ fieldName, array<array<SByte>^>^ byteArrays) override;

          //TODO:
          //virtual IPdxWriter^ WriteEnum(String^ fieldName, Enum e) ;
          //virtual IPdxWriter^ WriteInetAddress(String^ fieldName, InetAddress address);


          virtual IPdxWriter^ MarkIdentityField(String^ fieldName) override;

          virtual IPdxWriter^ WriteUnreadFields(IPdxUnreadFields^ unread) override;
        };

      }  // namespace Internal
    }  // namespace Client
  }  // namespace Geode
}  // namespace Apache
