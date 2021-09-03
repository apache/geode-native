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
#include "../DataOutput.hpp"
#include "../ExceptionTypes.hpp"

using namespace System;
using namespace System::IO;

namespace Apache
{
  namespace Geode
  {
    namespace Client
    {

      ref class GeodeDataOutputStream : public Stream
      {
      public:

        GeodeDataOutputStream(DataOutput^ output)
        {
          m_buffer = output;
        }

        virtual property bool CanSeek { bool get() override { return false; } }
        virtual property bool CanRead { bool get() override { return false; } }
        virtual property bool CanWrite { bool get() override { return true; } }

        virtual void Close() override { Stream::Close(); }

        virtual property System::Int64 Length
        {
          System::Int64 get() override
          {
            return (System::Int64) m_buffer->BufferLength;
          }
        }

        virtual property System::Int64 Position
        {
          System::Int64 get() override
          {
            return (System::Int64) m_position;
          }

          void set(System::Int64 value) override
          {
            m_position = (int) value;
          }
        }

        virtual System::Int64 Seek(System::Int64 offset, SeekOrigin origin) override
        {
          throw gcnew System::NotSupportedException("Seek not supported by GeodeDataOutputStream");
        }

        virtual void SetLength(System::Int64 value) override
        { 
          //TODO: overflow check
          //m_buffer->NativePtr->ensureCapacity((System::UInt32)value);
        }

        virtual void Write(array<Byte> ^ buffer, int offset, int count) override
        {
          try {
            /*
            array<Byte> ^ chunk = gcnew array<Byte>(count);
            array<Byte>::ConstrainedCopy(buffer, offset, chunk, 0, count);
            m_buffer->WriteBytesOnly(chunk, count);
            */
            //pin_ptr<const Byte> pin_bytes = &buffer[offset];
            //m_buffer->NativePtr->writeBytesOnly((const System::Byte*)pin_bytes, count);
            m_buffer->WriteBytesOnly(buffer, count, offset);
            m_position += count;
          }
          catch (const apache::geode::client::Exception& ex) {
            throw Apache::Geode::Client::GeodeException::Get(ex);
          }
          catch (System::AccessViolationException^ ex) {
            throw ex;
          }
        }

        virtual void WriteByte(unsigned char value) override
        {
          try {
            m_buffer->WriteByte(value);
            m_position++;
          }
          catch (const apache::geode::client::Exception& ex) {
            throw Apache::Geode::Client::GeodeException::Get(ex);
          }
          catch (System::AccessViolationException^ ex) {
            throw ex;
          }
        }

        virtual int Read(array<Byte> ^ buffer, int offset, int count) override
        {
          throw gcnew System::NotSupportedException("Read not supported by GeodeDataOutputStream");
        }

        virtual void Flush() override { /* do nothing */ }

      private:
        int m_position;
        DataOutput ^ m_buffer;
      };
    }  // namespace Client
  }  // namespace Geode
}  // namespace Apache

