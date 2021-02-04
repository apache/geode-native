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
#include <geode/DataInput.hpp>
#include "end_native.hpp"

#include "native_conditional_unique_ptr.hpp"
#include "native_unique_ptr.hpp"
#include "Log.hpp"
#include "ExceptionTypes.hpp"

namespace Apache
{
  namespace Geode
  {
    namespace Client
    {
      using namespace System;
      using namespace System::Collections::Generic;

      namespace native = apache::geode::client;

      interface class ISerializable;

      ref class Cache;

      /// <summary>
      /// Provides operations for reading primitive data values, byte arrays,
      /// strings, <c>ISerializable</c> objects from a byte stream.
      /// </summary>
      public ref class DataInput sealed
      {
      public:

        /// <summary>
        /// Construct <c>DataInput</c> using an given array of bytes.
        /// </summary>
        /// <param name="buffer">
        /// The buffer to use for reading data values
        /// </param>
        /// <exception cref="IllegalArgumentException">
        /// if the buffer is null
        /// </exception>
        DataInput( array<Byte>^ buffer, Cache^ cache );

        /// <summary>
        /// Construct <c>DataInput</c> using a given length of an array of
        /// bytes.
        /// </summary>
        /// <param name="buffer">
        /// The buffer to use for reading data values.
        /// </param>
        /// <param name="len">
        /// The number of bytes from the start of the buffer to use.
        /// </param>
        /// <exception cref="IllegalArgumentException">
        /// if the buffer is null
        /// </exception>
        DataInput( array<Byte>^ buffer, size_t len, Cache^ cache );

        /// <summary>
        /// Dispose: frees the internal buffer.
        /// </summary>
        ~DataInput( ) { }

        /// <summary>
        /// Finalizer: frees the internal buffer.
        /// </summary>
        !DataInput( ) { }

        /// <summary>
        /// Read a signed byte from the stream.
        /// </summary>
        SByte ReadSByte( );

        /// <summary>
        /// Read a boolean value from the stream.
        /// </summary>
        bool ReadBoolean( );

				/// <summary>
        /// Read a char value from the stream.
        /// </summary>
        Char ReadChar( );

        /// <summary>
        /// Read an array of bytes from the stream reading the length
        /// from the stream first.
        /// </summary>
        array<Byte>^ ReadBytes( );

        /// <summary>
        /// Read an array of signed bytes from the stream reading the length
        /// from the stream first.
        /// </summary>
        array<SByte>^ ReadSBytes( );

        /// <summary>
        /// Read the given number of bytes from the stream.
        /// </summary>
        /// <param name="len">Number of bytes to read.</param>
        array<Byte>^ ReadBytesOnly( System::UInt32 len );

        void ReadBytesOnly( array<Byte> ^ buffer, int offset, int count );

        /// <summary>
        /// Read the given number of signed bytes from the stream.
        /// </summary>
        /// <param name="len">Number of signed bytes to read.</param>
        array<SByte>^ ReadSBytesOnly( System::UInt32 len );

        /// <summary>
        /// Read a array len based on array size.
        /// </summary>
        int ReadArrayLen( );

        /// <summary>
        /// Read a 16-bit integer from the stream.
        /// </summary>
        System::Int16 ReadInt16( );

        /// <summary>
        /// Read a 32-bit integer from the stream.
        /// </summary>
        System::Int32 ReadInt32( );

        /// <summary>
        /// Read a 64-bit integer from the stream.
        /// </summary>
        System::Int64 ReadInt64( );

        /// <summary>
        /// Read a floating point number from the stream.
        /// </summary>
        float ReadFloat( );

        /// <summary>
        /// Read a double precision number from the stream.
        /// </summary>
        double ReadDouble( );

        /// <summary>
        /// Read a string after java-modified UTF-8 decoding from the stream.
        /// The maximum length supported is 2^16-1 beyond which the string
        /// shall be truncated.
        /// </summary>
        String^ ReadUTF( );

        String^ ReadString();

        /// <summary>
        /// Read a serializable object from the data. Null objects are handled.
        /// </summary>
        Object^ ReadObject( );
        
        /// <summary>
        /// Get the count of bytes that have been read from the stream.
        /// </summary>
        property size_t BytesRead
        {
          size_t get( );
        }

        /// <summary>
        /// Get the count of bytes that are remaining in the buffer.
        /// </summary>
        property size_t BytesRemaining
        {
          size_t get();
        }

        property Apache::Geode::Client::Cache^ Cache
        {
          Apache::Geode::Client::Cache^ get() { return m_cache; }
        }

        /// <summary>
        /// Advance the cursor of the buffer by the given offset.
        /// </summary>
        /// <param name="offset">
        /// The offset(number of bytes) by which to advance the cursor.
        /// </param>
        void AdvanceCursor( size_t offset );

        /// <summary>
        /// Rewind the cursor of the buffer by the given offset.
        /// </summary>
        /// <param name="offset">
        /// The offset(number of bytes) by which to rewind the cursor.
        /// </param>
        void RewindCursor( size_t offset );

        /// <summary>
        /// Reset the cursor to the start of buffer.
        /// </summary>
        void Reset();
        
        /// <summary>
        /// Read a dictionary from the stream in a given dictionary instance.
        /// </summary>
        /// <param name="dictionary">Object which implements System::Collections::IDictionary interface.</param>
        void ReadDictionary(System::Collections::IDictionary^ dictionary);
        
        /// <summary>
        /// Read a date from the stream.
        /// </summary>
				System::DateTime ReadDate( );

        /// <summary>
        /// Read a collection from the stream in a given collection instance.
        /// </summary>
        /// <param name="list">Object which implements System::Collections::IList interface.</param>
        void ReadCollection(System::Collections::IList^ list);
        
        /// <summary>
        /// Read a char array from the stream.
        /// </summary>
        array<Char>^ ReadCharArray( );

        /// <summary>
        /// Read a bool array from the stream.
        /// </summary>
				array<bool>^ ReadBooleanArray( );

        /// <summary>
        /// Read a short int array from the stream.
        /// </summary>
				array<Int16>^ ReadShortArray( );

        /// <summary>
        /// Read a int array from the stream.
        /// </summary>
				array<Int32>^ ReadIntArray();

        /// <summary>
        /// Read a long array from the stream.
        /// </summary>
				array<Int64>^ ReadLongArray();

        /// <summary>
        /// Read a float array from the stream.
        /// </summary>
				array<float>^ ReadFloatArray();

        /// <summary>
        /// Read a double array from the stream.
        /// </summary>
				array<double>^ ReadDoubleArray();

        /// <summary>
        /// Read a object array from the stream from the stream.
        /// </summary>
        List<Object^>^ ReadObjectArray();

        /// <summary>
        /// Read a array of signed byte array from the stream.
        /// </summary>
        array<array<Byte>^>^ ReadArrayOfByteArrays( );

      internal:

        native::DataInput* GetNative()
        {
          return m_nativeptr->get();
        }

        void setPdxdeserialization(bool val)
        {
          m_ispdxDesrialization = true;
        }
        bool isRootObjectPdx()
        {
          return m_isRootObjectPdx;
        }
        void setRootObjectPdx(bool val)
        {
          m_isRootObjectPdx = val;
        }

        Object^ readDotNetObjectArray();
        System::Collections::Generic::IDictionary<Object^, Object^>^ ReadDictionary();

        /// <summary>
        /// Read a string after java-modified UTF-8 decoding from the stream.
        /// </summary>
        String^ ReadUTFHuge( );

        /// <summary>
        /// Read a ASCII string from the stream. Where size is more than 2^16-1 
        /// </summary>
        String^ ReadASCIIHuge( );

        native::Pool* GetPool();

        Object^ ReadDotNetTypes(int8_t typeId);

        /// <summary>
        /// Get the count of bytes that have been read from the stream, for internal use only.
        /// </summary>
        property size_t BytesReadInternally
        {
          size_t get( );
        }

        void ReadObject(bool% obj)
        {
          obj = ReadBoolean();
        }

        void ReadObject(Byte% obj)
        {
          obj = ReadByte();
        }

        void ReadObject(Char% obj)
        {
          obj = (Char)ReadUInt16();
        }

        inline Char decodeChar( )
        {
          Char retChar;
          int b = m_buffer[ m_cursor++ ] & 0xff;
          int k = b >> 5;
          switch (  k )
            {
            default:
              retChar = ( Char ) ( b & 0x7f );
              break;
            case 6:
              {
                // two byte encoding
                // 110yyyyy 10xxxxxx
                // use low order 6 bits
                int y = b & 0x1f;
                // use low order 6 bits of the next byte
                // It should have high order bits 10, which we don't check.
                int x = m_buffer[ m_cursor++ ] & 0x3f;
                // 00000yyy yyxxxxxx
                retChar = ( Char ) ( y << 6 | x );
                break;
              }
            case 7:
              {
                // three byte encoding
                // 1110zzzz 10yyyyyy 10xxxxxx
                //assert ( b & 0x10 )
                  //     == 0 : "UTF8Decoder does not handle 32-bit characters";
                // use low order 4 bits
                int z = b & 0x0f;
                // use low order 6 bits of the next byte
                // It should have high order bits 10, which we don't check.
                int y = m_buffer[ m_cursor++ ] & 0x3f;
                // use low order 6 bits of the next byte
                // It should have high order bits 10, which we don't check.
                int x = m_buffer[ m_cursor++ ] & 0x3f;
                // zzzzyyyy yyxxxxxx
                int asint = ( z << 12 | y << 6 | x );
                retChar = ( Char ) asint;
                break;
              }
            }// end switch

            return retChar;
        }

        System::Collections::Hashtable^ ReadHashtable()
        {
          int len = this->ReadArrayLen();

          if(len == -1)
            return nullptr;
          else
          {
            System::Collections::Hashtable^ dict = gcnew System::Collections::Hashtable();
            for(int i =0; i< len; i++)
            {
              Object^ key = this->ReadObject();
              Object^ val = this->ReadObject();

              dict->Add(key, val);
            }
            return dict;
          }
        }

        /// <summary>
        /// Read a byte from the stream.
        /// </summary>
        Byte ReadByte( );

        /// <summary>
        /// Read a 16-bit unsigned integer from the stream.
        /// </summary>
        System::UInt16 ReadUInt16( );

        /// <summary>
        /// Read a 32-bit unsigned integer from the stream.
        /// </summary>
        System::UInt32 ReadUInt32( );
       
        /// <summary>
        /// Read a 64-bit unsigned integer from the stream.
        /// </summary>
        System::UInt64 ReadUInt64( );

        void ReadObject(Double% obj)
        {
          obj = ReadDouble();
        }

        void ReadObject(Single% obj)
        {
          obj = ReadFloat();
        }

        void ReadObject(System::Int16% obj)
        {
          obj = ReadInt16();
        }

        void ReadObject(System::Int32% obj)
        {
          obj = ReadInt32();
        }

        void ReadObject(System::Int64% obj)
        {
          obj = ReadInt64();
        }

				 void ReadObject(array<SByte>^% obj)
        {
          obj = ReadSBytes();
        }        

        void ReadObject(array<UInt16>^% obj);
        void ReadObject(array<UInt32>^% obj);
        void ReadObject(array<UInt64>^% obj);

        template <typename mType>
        void ReadObject(array<mType>^ %objArray)
        {
          int arrayLen = ReadArrayLen();
          if(arrayLen >= 0) {
            objArray = gcnew array<mType>(arrayLen);

            int i = 0;
            for( i = 0; i < arrayLen; i++ ){
              mType tmp;
              ReadObject(tmp);
              objArray[i] =  tmp; 
            }
          }
        }

				array<String^>^ ReadStringArray()
       {
          int len = this->ReadArrayLen();
          if ( len == -1)
          {
            return nullptr;
          }
          else 
          {
            array<String^>^ ret = gcnew array<String^>(len);
            if (len > 0)
            {
              for( int i = 0; i < len; i++)
              {
                Object^ obj = this->ReadObject();
                if(obj != nullptr)
                  ret[i] = static_cast<String^>(obj);
                else
                  ret[i] = nullptr;
              }
            }
            return ret;
          }
        }

				System::Byte* GetCursor()
        {
          return m_buffer + m_cursor;
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

        
        void AdvanceUMCursor()
        {
          try {
            m_nativeptr->get()->advanceCursor(m_cursor);
          }
          finally
          {
            GC::KeepAlive(m_nativeptr);
          }
          m_cursor = 0;
          m_bufferLength = 0;
        }

				void AdvanceCursorPdx(int offset)
        {
          m_cursor += offset;
        }

        void RewindCursorPdx(int rewind)
        {
          m_cursor = 0;
        }

        void ResetAndAdvanceCursorPdx(int offset)
        {
          m_cursor = offset;
        }

        void ResetPdx(int offset)
        {
          try
          {
            m_nativeptr->get()->reset(offset);
          }
          finally
          {
            GC::KeepAlive(m_nativeptr);
          }
          SetBuffer();
        }

        inline array<Byte>^ ReadReverseBytesOnly(int len);

        void SetBuffer()
        {
          try
          {
            m_buffer = const_cast<System::Byte*> (m_nativeptr->get()->currentBufferPosition());
            m_cursor = 0;
            m_bufferLength = static_cast<decltype(m_bufferLength)>(m_nativeptr->get()->getBytesRemaining());
          }
          finally
          {
            GC::KeepAlive(m_nativeptr);
          }
        }

        String^ DecodeBytes(int length)
        {
          //array<Char>^ output = gcnew array<Char>(length);
        
          if(m_forStringDecode->Length < length)
            m_forStringDecode = gcnew array<Char>(length);
          // index input[]
          int i = 0;
          // index output[]
          int j = 0;
          while ( i < length )
          {
            // get next byte unsigned
            //Byte b = m_buffer[ m_cursor++ ] & 0xff;
            Byte b = ReadByte();
            i++;
            Byte k = b >> 5;
            // classify based on the high order 3 bits
            switch (  k )
              {
              default:
                // one byte encoding
                // 0xxxxxxx
                // use just low order 7 bits
                // 00000000 0xxxxxxx
                m_forStringDecode[ j++ ] = ( Char ) ( b & 0x7f );
                break;
              case 6:
                {
                  // two byte encoding
                  // 110yyyyy 10xxxxxx
                  // use low order 6 bits
                  int y = b & 0x1f;
                  // use low order 6 bits of the next byte
                  // It should have high order bits 10, which we don't check.
                  int x = m_buffer[ m_cursor++ ] & 0x3f;
                  i++;
                  // 00000yyy yyxxxxxx
                  m_forStringDecode[ j++ ] = ( Char ) ( y << 6 | x );
                  break;
                }
              case 7:
                {
                  // three byte encoding
                  // 1110zzzz 10yyyyyy 10xxxxxx
                  //assert ( b & 0x10 )
                    //     == 0 : "UTF8Decoder does not handle 32-bit characters";
                  // use low order 4 bits
                  int z = b & 0x0f;
                  // use low order 6 bits of the next byte
                  // It should have high order bits 10, which we don't check.
                  int y = m_buffer[ m_cursor++ ] & 0x3f;
                  i++;
                  // use low order 6 bits of the next byte
                  // It should have high order bits 10, which we don't check.
                  int x = m_buffer[ m_cursor++ ] & 0x3f;
                  i++;
                  // zzzzyyyy yyxxxxxx
                  int asint = ( z << 12 | y << 6 | x );
                  m_forStringDecode[ j++ ] = ( Char ) asint;
                  break;
                }
              }// end switch
          }// end while

          String^ str = gcnew String(m_forStringDecode, 0, j);
          return str;
        }

        void CheckBufferSize(int size);
       
        Object^ ReadInternalObject();

        DataInput^ GetClone();

        /// <summary>
        /// Internal constructor to wrap a native object pointer
        /// </summary>
        /// <param name="nativeptr">The native object pointer</param>
        inline DataInput( apache::geode::client::DataInput* nativeptr, bool managedObject, Apache::Geode::Client::Cache^ cache )
        { 
          m_nativeptr = gcnew native_conditional_unique_ptr<native::DataInput>(nativeptr);
          m_ispdxDesrialization = false;
          m_isRootObjectPdx = false;
          m_cache = cache;
          m_cursor = 0;
          m_isManagedObject = managedObject;
          m_forStringDecode = gcnew array<Char>(100);
          m_buffer = const_cast<System::Byte*>(nativeptr->currentBufferPosition());
          if ( m_buffer != NULL) {
            m_bufferLength = static_cast<decltype(m_bufferLength)>(nativeptr->getBytesRemaining());
          }
          else {
            m_bufferLength = 0;
          }
        }

        DataInput( System::Byte* buffer, size_t size, Apache::Geode::Client::Cache^ cache );

        bool IsManagedObject()
        {
          return m_isManagedObject;
        }

        int GetPdxBytes()
        {
          return static_cast<int>(m_bufferLength);
        }

      private:

        /// <summary>
        /// Internal buffer managed by the class.
        /// This is freed in the disposer/destructor.
        /// </summary>
        bool m_ispdxDesrialization;
        bool m_isRootObjectPdx;
        Apache::Geode::Client::Cache^ m_cache;
        System::Byte* m_buffer;
        native_unique_ptr<System::Byte[]>^ m_ownedBuffer;
        size_t m_bufferLength;
        size_t m_cursor;
        bool m_isManagedObject;
        array<Char>^ m_forStringDecode;

        native_conditional_unique_ptr<native::DataInput>^ m_nativeptr;
      };
    }  // namespace Client
  }  // namespace Geode
}  // namespace Apache

