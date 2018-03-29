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


#include "CacheableObject.hpp"
#include "DataInput.hpp"
#include "DataOutput.hpp"
#include "impl/GeodeNullStream.hpp"
#include "impl/GeodeDataInputStream.hpp"
#include "impl/GeodeDataOutputStream.hpp"

using namespace System;
using namespace System::IO;
using namespace System::Runtime::Serialization::Formatters::Binary;

namespace Apache
{
  namespace Geode
  {
    namespace Client
    {

      void CacheableObject::ToData(DataOutput^ output)
      {
        if(m_obj != nullptr)
        {
          output->AdvanceCursor(4); // placeholder for object size bytes needed while reading back.

          GeodeDataOutputStream dos(output);
          BinaryFormatter bf;
          auto checkpoint = dos.Length;
          bf.Serialize(%dos, m_obj);
          m_objectSize = dos.Length - checkpoint;

          auto size = static_cast<uint32_t>(m_objectSize);
          output->RewindCursor(size + 4);
          output->WriteInt32(size);
          output->AdvanceCursor(size);
        }
      }

      void CacheableObject::FromData(DataInput^ input)
      {
        int maxSize = input->ReadInt32();
        GeodeDataInputStream dis(input, maxSize);
        auto checkpoint = dis.BytesRead;
        BinaryFormatter bf;
        m_obj = bf.Deserialize(%dis);
        m_objectSize = dis.BytesRead - checkpoint;
      }

      System::UInt64 CacheableObject::ObjectSize::get()
      { 
        if (m_objectSize == 0) {
          GeodeNullStream ns;
          BinaryFormatter bf;
          bf.Serialize(%ns, m_obj);

          m_objectSize = sizeof(CacheableObject^) + ns.Length;
        }
        return m_objectSize;
    }  // namespace Client
  }  // namespace Geode
}  // namespace Apache

 } //namespace 

