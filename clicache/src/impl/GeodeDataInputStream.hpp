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
#include "../DataInput.hpp"
#include "../ExceptionTypes.hpp"

using namespace System;
using namespace System::IO;

namespace Apache
{
  namespace Geode
  {
    namespace Client
    {

      ref class GeodeDataInputStream : public Stream
      {
      public:

        GeodeDataInputStream(DataInput^ input)
        {
          m_buffer = input;
          m_maxSize = input->BytesRemaining;
        }

        GeodeDataInputStream(DataInput^ input, int maxSize)
        {
          m_buffer = input;
          m_maxSize = maxSize;
          m_buffer->AdvanceUMCursor();
          m_buffer->SetBuffer();
        }

        virtual property bool CanSeek { bool get() override { return false; } }
        virtual property bool CanRead { bool get() override { return true; } }
        virtual property bool CanWrite { bool get() override { return false; } }

        virtual void Close() override { Stream::Close(); }

        virtual property System::Int64 Length
        {
          System::Int64 get() override
          {
            //return (System::Int64) m_buffer->BytesRead + m_buffer->BytesRemaining;
            return (System::Int64) m_maxSize;
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
          throw gcnew System::NotSupportedException("Seek not supported by GeodeDataInputStream");
        }

        virtual void SetLength(System::Int64 value) override { /* do nothing */ }

        virtual void Write(array<Byte> ^ buffer, int offset, int count) override
        {
          throw gcnew System::NotSupportedException("Write not supported by GeodeDataInputStream");
        }

        virtual void WriteByte(unsigned char value) override
        {
          throw gcnew System::NotSupportedException("WriteByte not supported by GeodeDataInputStream");
        }

        virtual int Read(array<Byte> ^ buffer, int offset, int count) override
        {
          try {/* due to auto replace */
          auto bytesRemaining = static_cast<int>(m_maxSize - m_buffer->BytesReadInternally);
					if(bytesRemaining <= 0)
						return bytesRemaining;
          auto actual = static_cast<int>(bytesRemaining < count ? bytesRemaining : count);
					if (actual > 0)
          {
            /*
            array<Byte>::ConstrainedCopy(m_buffer->ReadBytesOnly(actual), 0,
              buffer, offset, actual);
              */
            //pin_ptr<Byte> pin_buffer = &buffer[offset];
            //m_buffer->NativePtr->readBytesOnly((System::Byte*)pin_buffer, actual);
            m_buffer->ReadBytesOnly(buffer, offset, actual);
            m_position += actual;
          }
          return actual;
          }
        catch (const apache::geode::client::Exception& ex) {
          throw Apache::Geode::Client::GeodeException::Get(ex);
        }
        catch (System::AccessViolationException^ ex) {
          throw ex;
        }/* due to auto replace */
        }

        virtual void Flush() override { /* do nothing */ }

        property size_t BytesRead
        {
          size_t get()
          {
            return m_buffer->BytesReadInternally;
          }
        }

      private:
        size_t m_position;
        size_t m_maxSize;
        DataInput ^ m_buffer;
      };
    }  // namespace Client
  }  // namespace Geode
}  // namespace Apache
