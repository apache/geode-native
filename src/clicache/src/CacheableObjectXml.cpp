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



//#include "geode_includes.hpp"
#include "CacheableObjectXml.hpp"
#include "DataInput.hpp"
#include "DataOutput.hpp"
#include "Log.hpp"
#include "impl/GeodeNullStream.hpp"
#include "impl/GeodeDataInputStream.hpp"
#include "impl/GeodeDataOutputStream.hpp"

using namespace System;
using namespace System::IO;
using namespace System::Xml::Serialization;

namespace Apache
{
  namespace Geode
  {
    namespace Client
    {

      void CacheableObjectXml::ToData(DataOutput^ output)
      {
        if (m_obj == nullptr) {
          output->WriteByte((Byte)1);
        }
        else
        {
          output->WriteByte((Byte)0);
          Type^ objType = m_obj->GetType();

          output->WriteUTF(objType->AssemblyQualifiedName);

          output->AdvanceCursor(4); // placeholder for object size bytes needed while reading back.

          XmlSerializer xs(objType);
          GeodeDataOutputStream dos(output);
          System::Int64 checkpoint = dos.Length;
          xs.Serialize(%dos, m_obj);
          m_objectSize = (System::UInt32) (dos.Length - checkpoint);

          output->RewindCursor(m_objectSize + 4);
          output->WriteInt32(m_objectSize);
          output->AdvanceCursor(m_objectSize);
        }
      }

      IGeodeSerializable^ CacheableObjectXml::FromData(DataInput^ input)
      {
        Byte isNull = input->ReadByte();
        if (isNull) {
          m_obj = nullptr;
        }
        else {
          String^ typeName = input->ReadUTF();
          Type^ objType = Type::GetType(typeName);
          if (objType == nullptr)
          {
            Log::Error("CacheableObjectXml.FromData(): Cannot find type '" +
              typeName + "'.");
            m_obj = nullptr;
          }
          else {
            int maxSize = input->ReadInt32();
            GeodeDataInputStream dis(input, maxSize);
            XmlSerializer xs(objType);
            System::UInt32 checkpoint = dis.BytesRead;
            m_obj = xs.Deserialize(%dis);
            m_objectSize = dis.BytesRead - checkpoint;
          }
        }
        return this;
      }

      System::UInt32 CacheableObjectXml::ObjectSize::get()
      { 
        if (m_objectSize == 0) {
          if (m_obj != nullptr) {
            Type^ objType = m_obj->GetType();
            XmlSerializer xs(objType);
            GeodeNullStream ns;
            xs.Serialize(%ns, m_obj);

            m_objectSize = (System::UInt32)sizeof(CacheableObjectXml^) + (System::UInt32)ns.Length;
          }
        }
        return m_objectSize;
    }  // namespace Client
  }  // namespace Geode
}  // namespace Apache

 } //namespace 

