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
#include "CacheImpl.hpp"
#include "SerializationRegistry.hpp"
#include "end_native.hpp"

#include "Properties.hpp"
#include "String.hpp"
#include "impl/ManagedVisitor.hpp"
#include "impl/SafeConvert.hpp"
#include "ExceptionTypes.hpp"

using namespace System;


namespace Apache
{
  namespace Geode
  {
    namespace Client
    {

      namespace native = apache::geode::client;

      // Visitor class to get string representations of a property object
      ref class PropertyToString
      {
      private:

        String^ m_str;

      public:

        inline PropertyToString( ) : m_str( "{" )
        { }

        void Visit( Apache::Geode::Client::ICacheableKey^ key, ISerializable^ value )
        {
          if ( m_str->Length > 1 ) {
            m_str += ",";
          }
          m_str += key->ToString( ) + "=" + value;
        }

        virtual String^ ToString( ) override
        {
          return m_str;
        }
      };

      generic<class TPropKey, class TPropValue>
      TPropValue Properties<TPropKey, TPropValue>::Find( TPropKey key)
      {
        try
        {
          std::shared_ptr<native::CacheableKey> keyptr = Serializable::GetUnmanagedValueGeneric<TPropKey>(key);
          auto nativeptr = m_nativeptr->get()->find(keyptr);
          return TypeRegistry::GetManagedValueGeneric<TPropValue>(nativeptr);
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }
      }

      generic<class TPropKey, class TPropValue>
      void Properties<TPropKey, TPropValue>::Insert( TPropKey key, TPropValue value )
      {
        std::shared_ptr<native::CacheableKey> keyptr = Serializable::GetUnmanagedValueGeneric<TPropKey>(key, true);
        std::shared_ptr<native::Cacheable> valueptr = Serializable::GetUnmanagedValueGeneric<TPropValue>(value, true);

        try {

          try
          {
            m_nativeptr->get()->insert(keyptr, valueptr);
          }
          finally
          {
            GC::KeepAlive(m_nativeptr);
          }

        }
        catch (const apache::geode::client::Exception& ex) {
          throw Apache::Geode::Client::GeodeException::Get(ex);
        }
        catch (System::AccessViolationException^ ex) {
          throw ex;
        }
      }

      generic<class TPropKey, class TPropValue>
      void Properties<TPropKey, TPropValue>::Remove( TPropKey key)
      {
        std::shared_ptr<native::CacheableKey> keyptr = Serializable::GetUnmanagedValueGeneric<TPropKey>(key);

        try {

          try
          {
            m_nativeptr->get()->remove( keyptr );
          }
          finally
          {
            GC::KeepAlive(m_nativeptr);
          }

        }
        catch (const apache::geode::client::Exception& ex) {
          throw Apache::Geode::Client::GeodeException::Get(ex);
        }
        catch (System::AccessViolationException^ ex) {
          throw ex;
        }
      }

      generic<class TPropKey, class TPropValue>
      void Properties<TPropKey, TPropValue>::ForEach( PropertyVisitorGeneric<TPropKey, TPropValue>^ visitor )
      {
       if (visitor != nullptr)
        {
          native::ManagedVisitorGeneric mg_visitor( visitor );

          auto proxy = gcnew PropertyVisitorProxy<TPropKey, TPropValue>();
          proxy->SetPropertyVisitorGeneric(visitor);

          auto otherVisitor = gcnew PropertyVisitor(proxy, &PropertyVisitorProxy<TPropKey, TPropValue>::Visit);
          mg_visitor.setptr(otherVisitor);

          try {

            try
            {
              m_nativeptr->get()->foreach( mg_visitor );
            }
            finally
            {
              GC::KeepAlive(m_nativeptr);
            }

          }
        catch (const apache::geode::client::Exception& ex) {
          throw Apache::Geode::Client::GeodeException::Get(ex);
        }
        catch (System::AccessViolationException^ ex) {
          throw ex;
        }
        }
      }

      generic<class TPropKey, class TPropValue>
      System::UInt32 Properties<TPropKey, TPropValue>::Size::get( )
      {
        try {

          try
          {
            return static_cast<uint32_t>(m_nativeptr->get()->getSize( ));
          }
          finally
          {
            GC::KeepAlive(m_nativeptr);
          }

        }
        catch (const apache::geode::client::Exception& ex) {
          throw Apache::Geode::Client::GeodeException::Get(ex);
        }
        catch (System::AccessViolationException^ ex) {
          throw ex;
        }
      }

      generic<class TPropKey, class TPropValue>
      void Properties<TPropKey, TPropValue>::AddAll( Properties<TPropKey, TPropValue>^ other )
      {
        try {

          try
          {
            m_nativeptr->get()->addAll( other->GetNative() );
          }
          finally
          {
            GC::KeepAlive(m_nativeptr);
          }

        }
        catch (const apache::geode::client::Exception& ex) {
          throw Apache::Geode::Client::GeodeException::Get(ex);
        }
        catch (System::AccessViolationException^ ex) {
          throw ex;
        }
      }

      generic<class TPropKey, class TPropValue>
      void Properties<TPropKey, TPropValue>::Load( String^ fileName )
      {
        try {

          try
          {
            m_nativeptr->get()->load( Apache::Geode::Client::to_utf8(fileName) );
          }
          finally
          {
            GC::KeepAlive(m_nativeptr);
          }

        }
        catch (const apache::geode::client::Exception& ex) {
          throw Apache::Geode::Client::GeodeException::Get(ex);
        }
        catch (System::AccessViolationException^ ex) {
          throw ex;
        }
      }

      generic<class TPropKey, class TPropValue>
      String^ Properties<TPropKey, TPropValue>::ToString( )
      {
				return "";
      }

      // ISerializable methods

      generic<class TPropKey, class TPropValue>
      void Properties<TPropKey, TPropValue>::ToData( DataOutput^ output )
      {
        if (output->IsManagedObject()) {
        //TODO::??
          output->WriteBytesToUMDataOutput();          
        }
        
        try
        {
          auto nativeOutput = output->GetNative();
          if (nativeOutput != nullptr)
          {
            try {

                m_nativeptr->get()->toData(*nativeOutput);

            }
        catch (const apache::geode::client::Exception& ex) {
          throw Apache::Geode::Client::GeodeException::Get(ex);
        }
        catch (System::AccessViolationException^ ex) {
          throw ex;
        }
          }

          if (output->IsManagedObject()) {
            output->SetBuffer();          
          }
        }
        finally
        {
          GC::KeepAlive(output);
          GC::KeepAlive(m_nativeptr);
        }
      }

      generic<class TPropKey, class TPropValue>
      void Properties<TPropKey, TPropValue>::FromData( DataInput^ input )
      {
        if(input->IsManagedObject()) {
          input->AdvanceUMCursor();
        }

        auto nativeInput = input->GetNative();
        if (nativeInput != nullptr)
        {
          FromData(*nativeInput);
        }
        
        if(input->IsManagedObject()) {
          input->SetBuffer();
        }
      }

      generic<class TPropKey, class TPropValue>
      void Properties<TPropKey, TPropValue>::FromData( native::DataInput& input )
      {
        try {

        try
        {
          m_nativeptr->get()->fromData(input);
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }

        }
        catch (const apache::geode::client::Exception& ex) {
          throw Apache::Geode::Client::GeodeException::Get(ex);
        }
        catch (System::AccessViolationException^ ex) {
          throw ex;
        }
      }

      generic<class TPropKey, class TPropValue>
      System::UInt64 Properties<TPropKey, TPropValue>::ObjectSize::get( )
      {
        //TODO::
        try {

          try
          {
            return m_nativeptr->get()->objectSize( );
          }
          finally
          {
            GC::KeepAlive(m_nativeptr);
          }

        }
        catch (const apache::geode::client::Exception& ex) {
          throw Apache::Geode::Client::GeodeException::Get(ex);
        }
        catch (System::AccessViolationException^ ex) {
          throw ex;
        }
      }

      // ISerializable methods

      //generic<class TPropKey, class TPropValue>
      //void Properties<TPropKey, TPropValue>::GetObjectData( SerializationInfo^ info,
      //  StreamingContext context )
      //{
      //  auto output = std::unique_ptr<native::DataOutput>(new native::DataOutput(*m_serializationRegistry->get_shared_ptr()));

      //  try {

      //    try
      //    {
      //      m_nativeptr->get()->toData( *output );
      //    }
      //    finally
      //    {
      //      GC::KeepAlive(m_nativeptr);
      //    }

      //  }
      //  catch (const apache::geode::client::Exception& ex) {
      //    throw Apache::Geode::Client::GeodeException::Get(ex);
      //  }
      //  catch (System::AccessViolationException^ ex) {
      //    throw ex;
      //  }

      //  auto bytes = gcnew array<Byte>( output->getBufferLength( ) );
      //  {
      //    pin_ptr<const Byte> pin_bytes = &bytes[0];
      //    memcpy( (System::Byte*)pin_bytes, output->getBuffer( ),
      //      output->getBufferLength( ) );
      //  }
      //  info->AddValue( "bytes", bytes, array<Byte>::typeid );
      //}
      //
      //generic<class TPropKey, class TPropValue>
      //Properties<TPropKey, TPropValue>::Properties( SerializationInfo^ info,
      //  StreamingContext context, std::shared_ptr<native::SerializationRegistry> serializationRegistry)
      //  : Properties(serializationRegistry)
      //{
      //  array<Byte>^ bytes = nullptr;
      //  try {
      //    bytes = dynamic_cast<array<Byte>^>( info->GetValue( "bytes",
      //      array<Byte>::typeid ) );
      //  }
      //  catch ( System::Exception^ ) {
      //    // could not find the header -- null value
      //  }
      //  if (bytes != nullptr) {
      //    pin_ptr<const Byte> pin_bytes = &bytes[0];

      //    try {

      //      native::DataInput input( (System::Byte*)pin_bytes, bytes->Length, *CacheImpl::getInstance()->getSerializationRegistry().get());
      //      FromData(input);
      //    }
      //    catch (const apache::geode::client::Exception& ex) {
      //      throw Apache::Geode::Client::GeodeException::Get(ex);
      //    }
      //    catch (System::AccessViolationException^ ex) {
      //      throw ex;
      //    }
      //  }
      //}


    }  // namespace Client
  }  // namespace Geode
}  // namespace Apache
