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
#include "impl/ManagedVisitor.hpp"
#include "impl/ManagedString.hpp"
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

        void Visit( Apache::Geode::Client::ICacheableKey^ key, IGeodeSerializable^ value )
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
          native::CacheableKeyPtr keyptr = Serializable::GetUnmanagedValueGeneric<TPropKey>(key, nullptr);
          auto nativeptr = m_nativeptr->get()->find(keyptr);
          return Serializable::GetManagedValueGeneric<TPropValue>(nativeptr);
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }
      }

      generic<class TPropKey, class TPropValue>
      void Properties<TPropKey, TPropValue>::Insert( TPropKey key, TPropValue value )
      {
        native::CacheableKeyPtr keyptr = Serializable::GetUnmanagedValueGeneric<TPropKey>(key, true, nullptr);
        native::CacheablePtr valueptr = Serializable::GetUnmanagedValueGeneric<TPropValue>(value, true, nullptr);

        _GF_MG_EXCEPTION_TRY2

          try
          {
            m_nativeptr->get()->insert(keyptr, valueptr);
          }
          finally
          {
            GC::KeepAlive(m_nativeptr);
          }

        _GF_MG_EXCEPTION_CATCH_ALL2
      }

      generic<class TPropKey, class TPropValue>
      void Properties<TPropKey, TPropValue>::Remove( TPropKey key)
      {
        native::CacheableKeyPtr keyptr = Serializable::GetUnmanagedValueGeneric<TPropKey>(key, nullptr);

        _GF_MG_EXCEPTION_TRY2

          try
          {
            m_nativeptr->get()->remove( keyptr );
          }
          finally
          {
            GC::KeepAlive(m_nativeptr);
          }

        _GF_MG_EXCEPTION_CATCH_ALL2
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

          _GF_MG_EXCEPTION_TRY2

            try
            {
              m_nativeptr->get()->foreach( mg_visitor );
            }
            finally
            {
              GC::KeepAlive(m_nativeptr);
            }

          _GF_MG_EXCEPTION_CATCH_ALL2
        }
      }

      generic<class TPropKey, class TPropValue>
      System::UInt32 Properties<TPropKey, TPropValue>::Size::get( )
      {
        _GF_MG_EXCEPTION_TRY2

          try
          {
            return m_nativeptr->get()->getSize( );
          }
          finally
          {
            GC::KeepAlive(m_nativeptr);
          }

        _GF_MG_EXCEPTION_CATCH_ALL2
      }

      generic<class TPropKey, class TPropValue>
      void Properties<TPropKey, TPropValue>::AddAll( Properties<TPropKey, TPropValue>^ other )
      {
        _GF_MG_EXCEPTION_TRY2

          try
          {
            m_nativeptr->get()->addAll( other->GetNative() );
          }
          finally
          {
            GC::KeepAlive(m_nativeptr);
          }

        _GF_MG_EXCEPTION_CATCH_ALL2
      }

      generic<class TPropKey, class TPropValue>
      void Properties<TPropKey, TPropValue>::Load( String^ fileName )
      {
        ManagedString mg_fname( fileName );

        _GF_MG_EXCEPTION_TRY2

          try
          {
            m_nativeptr->get()->load( mg_fname.CharPtr );
          }
          finally
          {
            GC::KeepAlive(m_nativeptr);
          }

        _GF_MG_EXCEPTION_CATCH_ALL2
      }

      generic<class TPropKey, class TPropValue>
      String^ Properties<TPropKey, TPropValue>::ToString( )
      {
				return "";
      }

      // IGeodeSerializable methods

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
            _GF_MG_EXCEPTION_TRY2

                m_nativeptr->get()->toData(*nativeOutput);

            _GF_MG_EXCEPTION_CATCH_ALL2
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
      IGeodeSerializable^ Properties<TPropKey, TPropValue>::FromData( DataInput^ input )
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

        return this;
      }

      generic<class TPropKey, class TPropValue>
      void Properties<TPropKey, TPropValue>::FromData( native::DataInput& input )
      {
        _GF_MG_EXCEPTION_TRY2

          try
        {
          auto p = static_cast<native::Properties*>(m_nativeptr->get()->fromData(input));
          if (m_nativeptr->get() != p) {
            m_nativeptr->get_shared_ptr().reset(p);
          }
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }

        _GF_MG_EXCEPTION_CATCH_ALL2
      }

      generic<class TPropKey, class TPropValue>
      System::UInt32 Properties<TPropKey, TPropValue>::ObjectSize::get( )
      {
        //TODO::
        _GF_MG_EXCEPTION_TRY2

          try
          {
            return m_nativeptr->get()->objectSize( );
          }
          finally
          {
            GC::KeepAlive(m_nativeptr);
          }

        _GF_MG_EXCEPTION_CATCH_ALL2
      }

      // ISerializable methods

      //generic<class TPropKey, class TPropValue>
      //void Properties<TPropKey, TPropValue>::GetObjectData( SerializationInfo^ info,
      //  StreamingContext context )
      //{
      //  auto output = std::unique_ptr<native::DataOutput>(new native::DataOutput(*m_serializationRegistry->get_shared_ptr()));

      //  _GF_MG_EXCEPTION_TRY2

      //    try
      //    {
      //      m_nativeptr->get()->toData( *output );
      //    }
      //    finally
      //    {
      //      GC::KeepAlive(m_nativeptr);
      //    }

      //  _GF_MG_EXCEPTION_CATCH_ALL2

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
      //  StreamingContext context, native::SerializationRegistryPtr serializationRegistry)
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

      //    _GF_MG_EXCEPTION_TRY2

      //      native::DataInput input( (System::Byte*)pin_bytes, bytes->Length, *CacheImpl::getInstance()->getSerializationRegistry().get());
      //      FromData(input);
      //    _GF_MG_EXCEPTION_CATCH_ALL2
      //  }
      //}


    }  // namespace Client
  }  // namespace Geode
}  // namespace Apache
