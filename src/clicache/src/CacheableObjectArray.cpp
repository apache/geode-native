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
#include <GeodeTypeIdsImpl.hpp>
#include "end_native.hpp"
#include "CacheableObjectArray.hpp"
#include "DataOutput.hpp"
#include "DataInput.hpp"
#include "ExceptionTypes.hpp"
#include "impl/SafeConvert.hpp"
#include "impl/PdxTypeRegistry.hpp"

using namespace System;
using namespace System::Collections::Generic;


namespace Apache
{
  namespace Geode
  {
    namespace Client
    {

      // Region: IGeodeSerializable Members

      void CacheableObjectArray::ToData(DataOutput^ output)
      {
        output->WriteArrayLen((System::Int32)Count);
        output->WriteByte((int8_t)apache::geode::client::GeodeTypeIdsImpl::Class);
        output->WriteByte((int8_t)apache::geode::client::GeodeTypeIds::CacheableASCIIString);
        output->WriteUTF("java.lang.Object");

        for each (Object^ obj in this) {
					//TODO::split
          output->WriteObject(obj);
        }
      }

      IGeodeSerializable^ CacheableObjectArray::FromData(DataInput^ input)
      {
        int len = input->ReadArrayLen();
        if (len >= 0) {
          //int8_t typeCode;
          input->ReadByte(); // ignore CLASS typeid
          input->ReadByte(); // ignore string typeid
          unsigned short classLen;
          classLen = input->ReadInt16();
          input->AdvanceCursor(classLen);
          //nativeInput.readInt(&classLen);
          //nativeInput.advanceCursor(classLen);
        }
        for (System::Int32 index = 0; index < len; ++index) {
          Add(input->ReadObject());
        }
        return this;
        /*_GF_MG_EXCEPTION_TRY

          apache::geode::client::DataInput& nativeInput = *(input->_NativePtr);
          System::Int32 len;
          nativeInput.readArrayLen(&len);
          if (len >= 0) {
            int8_t typeCode;
            nativeInput.read(&typeCode); // ignore CLASS typeid
            nativeInput.read(&typeCode); // ignore string typeid
            System::UInt16 classLen;
            nativeInput.readInt(&classLen);
            nativeInput.advanceCursor(classLen);
          }
          apache::geode::client::CacheablePtr value;
          for (System::Int32 index = 0; index < len; ++index) {
            nativeInput.readObject(value);
            Add(SafeUMSerializableConvert(value.get()));
          }

        _GF_MG_EXCEPTION_CATCH_ALL
        return this;*/
      }

      System::UInt32 CacheableObjectArray::ObjectSize::get()
      { 
       /* System::UInt32 size = static_cast<System::UInt32> (sizeof(CacheableObjectArray^));
        for each (IGeodeSerializable^ val in this) {
          if (val != nullptr) {
            size += val->ObjectSize;
          }
        }*/
        return Count;
    }  // namespace Client
  }  // namespace Geode
}  // namespace Apache

 } //namespace 
