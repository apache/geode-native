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


#include "geode_defs.hpp"
#include "begin_native.hpp"
#include <geode/DataOutput.hpp>
#include "end_native.hpp"

#include "native_conditional_unique_ptr.hpp"
#include "Log.hpp"
#include "ExceptionTypes.hpp"
#include "Serializable.hpp"

#include "CacheableString.hpp"
#include "CacheableDate.hpp"
#include "CacheableVector.hpp"
#include "CacheableArrayList.hpp"
#include "CacheableStack.hpp"

using namespace System;
using namespace System::Runtime::CompilerServices;

namespace Apache
{
  namespace Geode
  {
    namespace Client
    {
      namespace native = apache::geode::client;

      ref class Cache;
      interface class ISerializable;

      /// <summary>
      /// Provides operations for writing primitive data values, and
      /// user-defined objects implementing ISerializable, to a byte stream.
      /// This class is intentionally not thread safe.
      /// </summary>
      public ref class DataOutput sealed
      {
      private:
        System::Int32 m_cursor;
        bool m_isManagedObject;
        System::Byte * m_bytes;
        System::Int32 m_remainingBufferLength;
        bool m_ispdxSerialization;
        native_conditional_unique_ptr<native::DataOutput>^ m_nativeptr;
        Apache::Geode::Client::Cache^ m_cache;

      public:

        /// <summary>
        /// Default constructor.
        /// </summary>
        DataOutput(Apache::Geode::Client::Cache^ cache);

        /// <summary>
        /// Write length of the array to the <c>DataOutput</c>.
        /// </summary>
        /// <param name="len">Array len to write.</param>
        void WriteArrayLen( System::Int32 len );
        
        /// <summary>
        /// Write a signed byte to the <c>DataOutput</c>.
        /// </summary>
        /// <param name="value">The signed byte to write.</param>
        void WriteSByte( SByte value );

        /// <summary>
        /// Write a boolean value to the <c>DataOutput</c>.
        /// </summary>
        /// <param name="value">The boolean value to write.</param>
        void WriteBoolean( bool value );

				/// <summary>
        /// Write a char value to the <c>DataOutput</c>.
        /// </summary>
        /// <param name="value">The char value to write.</param>
        void WriteChar( Char value );

        /// <summary>
        /// Write a given length of bytes to the <c>DataOutput</c>.
        /// </summary>
        /// <param name="bytes">The array of bytes to write.</param>
        /// <param name="len">
        /// The number of bytes from the start of array to write.
        /// </param>
        void WriteBytes( array<Byte>^ bytes, System::Int32 len );

        /// <summary>
        /// Write an array of bytes to the <c>DataOutput</c>.
        /// </summary>
        /// <param name="bytes">The array of bytes to write.</param>
        inline void WriteBytes( array<Byte>^ bytes )
        {
          WriteBytes( bytes, ( bytes == nullptr ? -1 : bytes->Length ) );
        }

        /// <summary>
        /// Write a given length of signed bytes to the <c>DataOutput</c>.
        /// </summary>
        /// <param name="bytes">The array of signed bytes to write.</param>
        /// <param name="len">
        /// The number of bytes from the start of array to write.
        /// </param>
        void WriteSBytes( array<SByte>^ bytes, System::Int32 len );

        /// <summary>
        /// Write an array of signed bytes to the <c>DataOutput</c>.
        /// </summary>
        /// <param name="bytes">The array of signed bytes to write.</param>
        inline void WriteSBytes( array<SByte>^ bytes )
        {
          WriteSBytes( bytes, ( bytes == nullptr ? -1 : bytes->Length )  );
        }

        /// <summary>
        /// Write a given length of bytes without its length to the
        /// <c>DataOutput</c>.
        /// </summary>
        /// <param name="bytes">The array of bytes to write.</param>
        /// <param name="len">
        /// The number of bytes from the start of array to write.
        /// </param>
        void WriteBytesOnly( array<Byte>^ bytes, System::UInt32 len );

        void WriteBytesOnly( array<Byte>^ bytes, System::UInt32 len, System::UInt32 offset );

        /// <summary>
        /// Write an array of bytes without its length to the
        /// <c>DataOutput</c>.
        /// </summary>
        /// <param name="bytes">The array of bytes to write.</param>
        inline void WriteBytesOnly( array<Byte>^ bytes )
        {
          WriteBytesOnly( bytes, ( bytes == nullptr ? 0 : bytes->Length )  );
        }

        /// <summary>
        /// Write a given length of signed bytes without its length
        /// to the <c>DataOutput</c>.
        /// </summary>
        /// <param name="bytes">The array of signed bytes to write.</param>
        /// <param name="len">
        /// The number of bytes from the start of array to write.
        /// </param>
        void WriteSBytesOnly( array<SByte>^ bytes, System::UInt32 len );

        /// <summary>
        /// Write an array of signed bytes without its length
        /// to the <c>DataOutput</c>.
        /// </summary>
        /// <param name="bytes">The array of signed bytes to write.</param>
        inline void WriteSBytesOnly( array<SByte>^ bytes )
        {
          WriteSBytesOnly( bytes, ( bytes == nullptr ? 0 : bytes->Length )  );
        }        

        /// <summary>
        /// Write a 16-bit integer to the <c>DataOutput</c>.
        /// </summary>
        /// <param name="value">The 16-bit integer to write.</param>
        void WriteInt16( System::Int16 value );

        /// <summary>
        /// Write a 32-bit integer to the <c>DataOutput</c>.
        /// </summary>
        /// <param name="value">The 32-bit integer to write.</param>
        void WriteInt32( System::Int32 value );

        /// <summary>
        /// Write a 64-bit integer to the <c>DataOutput</c>.
        /// </summary>
        /// <param name="value">The 64-bit integer to write.</param>
        void WriteInt64( System::Int64 value );

        /// <summary>
        /// Write a float to the DataOutput.
        /// </summary>
        /// <param name="value">The float value to write.</param>
        void WriteFloat( float value );

        /// <summary>
        /// Write a double precision real number to the <c>DataOutput</c>.
        /// </summary>
        /// <param name="value">
        /// The double precision real number to write.
        /// </param>
        void WriteDouble( double value );

        /// <summary>
        /// Write a string using java-modified UTF-8 encoding to
        /// <c>DataOutput</c>.
        /// The maximum length supported is 2^16-1 beyond which the string
        /// shall be truncated.
        /// </summary>
        /// <param name="value">The UTF encoded string to write.</param>
        void WriteUTF( String^ value );

        void WriteString(String^ value);

        /// <summary>
        /// Write a <c>Serializable</c> object to the <c>DataOutput</c>.
        /// This is provided to conveniently pass primitive types (like string)
        /// that shall be implicitly converted to corresponding
        /// <c>ISerializable</c> wrapper types.
        /// </summary>
        /// <param name="obj">The object to write.</param>
        void WriteObject( Object^ obj );

        /// <summary>
        /// Advance the buffer cursor by the given offset.
        /// </summary>
        /// <param name="offset">
        /// The offset by which to advance the cursor.
        /// </param>
        void AdvanceCursor( System::UInt32 offset );

        /// <summary>
        /// Rewind the buffer cursor by the given offset.
        /// </summary>
        /// <param name="offset">
        /// The offset by which to rewind the cursor.
        /// </param>
        void RewindCursor( System::UInt32 offset );

        /// <summary>
        /// Get a copy of the current buffer.
        /// </summary>
        array<Byte>^ GetBuffer( );

        /// <summary>
        /// Get the length of current data in the buffer.
        /// </summary>
        property size_t BufferLength
        {
          size_t get( );
        }
        
        property Apache::Geode::Client::Cache^ Cache
        {
          Apache::Geode::Client::Cache^ get() { return m_cache; }
        }

        /// <summary>
        /// Reset the cursor to the start of the buffer.
        /// </summary>
        void Reset( );
       
        /// <summary>
        /// Write a Dictionary to the DataOutput.
        /// </summary>
        /// <param name="value">The object which implements IDictionary to write.</param>
 			  void WriteDictionary(System::Collections::IDictionary^ value);              

        /// <summary>
        /// Write a date to the DataOutput.
        /// </summary>
        /// <param name="value">The date value to write.</param>
        void WriteDate(System::DateTime value);

        /// <summary>
        /// Write a collection to the DataOutput.
        /// </summary>
        /// <param name="value">The object which implements IList to write.</param>
        void WriteCollection(System::Collections::IList^ value);
        
        /// <summary>
        /// Write a char array to the DataOutput.
        /// </summary>
        /// <param name="value">The char array to write.</param>
        void WriteCharArray(array<Char>^ value);

        /// <summary>
        /// Write a bool array to the DataOutput.
        /// </summary>
        /// <param name="value">The bool array to write.</param>
				void WriteBooleanArray(array<bool>^ value);

        /// <summary>
        /// Write a short array to the DataOutput.
        /// </summary>
        /// <param name="value">The short array to write.</param>
				void WriteShortArray(array<Int16>^ value);

        /// <summary>
        /// Write an unsigned short array to the DataOutput.
        /// </summary>
        /// <param name="value">The unsigned short array to write.</param>
				void WriteUShortArray(array<UInt16>^ value);

        /// <summary>
        /// Write a int array to the DataOutput.
        /// </summary>
        /// <param name="value">The int array to write.</param>
				void WriteIntArray(array<Int32>^ value);

        /// <summary>
        /// Write an unsigned int array to the DataOutput.
        /// </summary>
        /// <param name="value">The unsigned int array to write.</param>
				void WriteUIntArray(array<UInt32>^ value);

        /// <summary>
        /// Write a long array to the DataOutput.
        /// </summary>
        /// <param name="value">The long array to write.</param>
				void WriteLongArray(array<Int64>^ value);

        /// <summary>
        /// Write an unsigned long array to the DataOutput.
        /// </summary>
        /// <param name="value">The unsigned long array to write.</param>
				void WriteULongArray(array<UInt64>^ value);

        /// <summary>
        /// Write a float array to the DataOutput.
        /// </summary>
        /// <param name="value">The float array to write.</param>
				void WriteFloatArray(array<float>^ value);

        /// <summary>
        /// Write a double array to the DataOutput.
        /// </summary>
        /// <param name="value">The double array to write.</param>
				void WriteDoubleArray(array<double>^ value);

        /// <summary>
        /// Write a object array to the DataOutput.
        /// </summary>
        /// <param name="value">The object array to write.</param>
        void WriteObjectArray(List<Object^>^ value);

        /// <summary>
        /// Write a array of unsigned byte arrays to the DataOutput.
        /// </summary>
        /// <param name="value">The array of unsigned byte arrays to write.</param>
        void WriteArrayOfByteArrays(array<array<Byte>^>^ value);

        /// <summary>
        /// Write a array of signed byte arrays to the DataOutput.
        /// </summary>
        /// <param name="value">The array of signed byte arrays to write.</param>
        void WriteArrayOfSByteArrays(array<array<SByte>^>^ value);
               
      internal:

        native::DataOutput* GetNative()
        {
          return m_nativeptr->get();
        }

        void WriteDotNetObjectArray(Object^ objectArray);

        /// <summary>
        /// Write a byte to the <c>DataOutput</c>.
        /// </summary>
        /// <param name="value">The byte to write.</param>
        void WriteByte( Byte value );

        /// <summary>
        /// Write an unsigned short integer (System::Int16) to the <c>DataOutput</c>.
        /// </summary>
        /// <param name="value">The unsigned 16-bit integer to write.</param>
        void WriteUInt16( System::UInt16 value );

        /// <summary>
        /// Write an unsigned 32-bit integer to the <c>DataOutput</c>.
        /// </summary>
        /// <param name="value">The unsigned 32-bit integer to write.</param>
        void WriteUInt32( System::UInt32 value );

        /// <summary>
        /// Write an unsigned 64-bit integer to the <c>DataOutput</c>.
        /// </summary>
        /// <param name="value">The unsigned 64-bit integer to write.</param>
        void WriteUInt64( System::UInt64 value );

        /// <summary>
        /// Write a string using java-modified UTF-8 encoding to
        /// <c>DataOutput</c>.
        /// Length should be more than 2^16 -1. 
        /// </summary>
        /// <param name="value">The UTF encoded string to write.</param>
        void WriteUTFHuge( String^ value );

        /// <summary>
        /// Write a string(only ASCII char) to
        /// <c>DataOutput</c>.
        /// Length should be more than 2^16 -1.
        /// </summary>
        /// <param name="value">The UTF encoded string to write.</param>
        void WriteASCIIHuge( String^ value );


			  System::Int32 GetBufferLengthPdx()
        {
          try
          {
            return (System::Int32)m_nativeptr->get()->getBufferLength();
          }
          finally
          {
            GC::KeepAlive(m_nativeptr);
          }
        }

        System::Int32 GetCursorPdx()
        {
          return m_cursor;
        }

        native::Pool* GetPool();

        void WriteStringArray(array<String^>^ strArray);

        void EncodeUTF8String( String^ input, int encLength )
        {
          const int strLength = input->Length;
          const int end = m_cursor + encLength;
          for ( int i = 0; i < strLength && m_cursor < end; i++ )
          {
            unsigned short c = (unsigned short)input[i];

            if( c == 0 )
            {
                m_bytes[m_cursor++] = 0xc0;
                m_bytes[m_cursor++] = 0x80;
            }
            else if ( c < 0x80 )//ASCII character
            {
              // 7-bits done in one byte.
              m_bytes[m_cursor++] = (System::Byte)c;
            }
            else if ( c < 0x800 )
            {
              // 8-11 bits done in 2 bytes
              m_bytes[m_cursor++] = ( 0xC0 | c >> 6 );
              m_bytes[m_cursor++] = ( 0x80 | c & 0x3F );
            }
            else 
            {
              // 12-16 bits done in 3 bytes
              m_bytes[m_cursor++] = ( 0xE0 | c >> 12 );
              m_bytes[m_cursor++] = ( 0x80 | c >> 6 & 0x3F );
              m_bytes[m_cursor++] = ( 0x80 | c & 0x3F );
            }            
          }

		  // TODO ASSERT end = m_cursor
        }
       
        static int getEncodedLength(String^ input)
        {
          int count = 0;
          for ( int i = 0; i < input->Length; i++ )
          {
            unsigned short c = (unsigned short)input[i];

            if( c == 0)
            {
              count += 2;
            }
            else if ( c < 0x80 )//ASCII character
            {
              count++;
            }
            else if ( c < 0x800 )
            {
              count += 2;
            }
            else
            {
               count += 3;
            }
          }// end for

          return count;
        }

        void setPdxSerialization(bool val)
        {
          m_ispdxSerialization = val;
        }

        void WriteStringWithType( String^ value );

        static int8_t GetTypeId(System::UInt32 classId );
        
        static int8_t DSFID(System::UInt32 classId);        
  
        static inline DSCode getDataSerializableDsCode(int32_t classId) {
          if (classId <= std::numeric_limits<int8_t>::max() &&
              classId >= std::numeric_limits<int8_t>::min()) {
            return DSCode::CacheableUserData;
          } else if (classId <= std::numeric_limits<int16_t>::max() &&
                     classId >= std::numeric_limits<int16_t>::min()) {
            return DSCode::CacheableUserData2;
          } else {
            return DSCode::CacheableUserData4;
          }
        }

				static inline int8_t getDataSerializableFixedIdDsCode(int32_t fixedId) {
          if (fixedId <= std::numeric_limits<int8_t>::max() &&
              fixedId >= std::numeric_limits<int8_t>::min()) {
            return static_cast<int8_t>(DSCode::FixedIDByte);
          } else if (fixedId <= std::numeric_limits<int16_t>::max() &&
                     fixedId >= std::numeric_limits<int16_t>::min()) {
            return static_cast<int8_t>(DSCode::FixedIDShort);
          } else {
            return static_cast<int8_t>(DSCode::FixedIDInt);
          }
        }

        void WriteObjectInternal( ISerializable^ obj );     

        void WriteBytesToUMDataOutput();
        
        void WriteObject(bool% obj);        

        void WriteObject(Byte% obj);        

        void WriteObject(Char% obj);        

        void WriteObject(Double% obj);
        
        void WriteObject(Single% obj);
        
        void WriteObject(System::Int16% obj);
        
        void WriteObject(System::Int32% obj);
        
        void WriteObject(System::Int64% obj);
        
				void WriteObject(UInt16% obj);
        
        void WriteObject(UInt32% obj);       

        void WriteObject(UInt64% obj);

        
        template <typename mType>
        void WriteObject(array<mType>^ %objArray)
        {
          if(objArray != nullptr) {
            int arrayLen = objArray->Length;
            WriteArrayLen(arrayLen);
            if(arrayLen > 0) {
              int i = 0;
              for( i = 0; i < arrayLen; i++ ){
                WriteObject(objArray[i]);
              }
            }
          }
          else {
            WriteByte(0xff);
          }
        }

        bool IsManagedObject()
        {
          return m_isManagedObject;
        }

        void SetBuffer()
        {
          m_cursor = 0;
          try
          {
            m_bytes = const_cast<System::Byte *>(m_nativeptr->get()->getCursor());
            m_remainingBufferLength = (System::Int32)m_nativeptr->get()->getRemainingBufferLength();
          }
          finally
          {
            GC::KeepAlive(m_nativeptr);
          }
        }

				System::Byte* GetStartBufferPosition()
        {
          try
          {
            return const_cast<System::Byte *>( m_nativeptr->get()->getBuffer());
          }
          finally
          {
            GC::KeepAlive(m_nativeptr);
          };
        }

        inline void EnsureCapacity( System::Int32 size )
        {
          System::Int32 bytesLeft = m_remainingBufferLength - m_cursor;
          if ( bytesLeft < size ) {
            try
            {
              auto p = m_nativeptr->get();
              p->ensureCapacity(m_cursor + size);
              m_bytes = const_cast<System::Byte *>( p->getCursor());
              m_remainingBufferLength = (System::Int32)p->getRemainingBufferLength();
            }
            catch(apache::geode::client::OutOfMemoryException ex )
            {
              throw gcnew OutOfMemoryException(ex);
            }            
            finally
            {
              GC::KeepAlive(m_nativeptr);
            }
          }
        }

        //it expects list is not null
        inline void WriteList(System::Collections::IList^ list)
        {
          this->WriteArrayLen(list->Count);
          for each (Object^ obj in list) 
						this->WriteObject(obj);
        }

        System::Byte* GetBytes(System::Byte* src, System::UInt32 size)
        {
          try
          {
            return m_nativeptr->get()->getBufferCopyFrom(src, size);
          }
          finally
          {
            GC::KeepAlive(m_nativeptr);
          }
        }
 
        System::Int32 GetRemainingBufferLength()
        {
          try
          {
            return (System::Int32) m_nativeptr->get()->getRemainingBufferLength();
          }
          finally
          {
            GC::KeepAlive(m_nativeptr);
          }
        }

        /// <summary>
        /// Internal constructor to wrap a native object pointer
        /// </summary>
        /// <param name="nativeptr">The native object pointer</param>
        inline DataOutput(apache::geode::client::DataOutput* nativeptr, bool managedObject, Apache::Geode::Client::Cache^ cache)
        {
          m_cache = cache;
          m_nativeptr = gcnew native_conditional_unique_ptr<native::DataOutput>(nativeptr);
          m_isManagedObject = managedObject;
          m_cursor = 0;
          m_bytes = const_cast<System::Byte *>(nativeptr->getCursor());
          m_remainingBufferLength = (System::Int32)nativeptr->getRemainingBufferLength();
          m_ispdxSerialization = false;
        }
      };
    }  // namespace Client
  }  // namespace Geode
}  // namespace Apache

