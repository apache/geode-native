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




#include "CacheableStack.hpp"
#include "DataOutput.hpp"
#include "DataInput.hpp"
#include "begin_native.hpp"
#include <GeodeTypeIdsImpl.hpp>
#include "end_native.hpp"
#include "impl/SafeConvert.hpp"
#include "GeodeClassIds.hpp"

using namespace System;
using namespace System::Collections::Generic;

namespace Apache
{
  namespace Geode
  {
    namespace Client
    {

      // Region: IGeodeSerializable Members

      void CacheableStack::ToData(DataOutput^ output)
      {
        if (m_stack != nullptr)
        {
          output->WriteArrayLen((System::Int32)m_stack->Count);
          for each (Object^ obj in m_stack) {
            output->WriteObject(obj);
          }
        }
        else
        {
          output->WriteByte(0xFF);
        }
      }

      void CacheableStack::FromData(DataInput^ input)
      {
        int len = input->ReadArrayLen();
        if (len > 0)
        {
          System::Collections::Generic::Stack<Object^>^ stack = safe_cast<System::Collections::Generic::Stack<Object^>^>(m_stack);
          for (int i = 0; i < len; i++)
          {
            (stack)->Push(input->ReadObject());
          }
        }
      }

      System::UInt32 CacheableStack::ClassId::get()
      {
        return GeodeClassIds::CacheableStack;
      }

      System::UInt64 CacheableStack::ObjectSize::get()
      {
        //TODO:
        /*System::UInt32 size = static_cast<System::UInt32> (sizeof(CacheableStack^));
        for each (IGeodeSerializable^ val in this) {
        if (val != nullptr) {
        size += val->ObjectSize;
        }
        }*/
        return m_stack->Count;
      }  // namespace Client
    }  // namespace Geode
  }  // namespace Apache

} //namespace 

