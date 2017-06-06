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
#include <geode/Properties.hpp>
#include "SerializationRegistry.hpp"
#include "end_native.hpp"

#include "IGeodeSerializable.hpp"
#include "ICacheableKey.hpp"
#include "DataInput.hpp"
#include "DataOutput.hpp"
#include "CacheableString.hpp"
#include "native_shared_ptr.hpp"
#include "impl/SafeConvert.hpp"
#include "Serializable.hpp"

using namespace System;
using namespace System::Runtime::Serialization;
using namespace System::Collections::Generic;

namespace Apache
{
  namespace Geode
  {
    namespace Client
    {
      namespace native = apache::geode::client;

      delegate void PropertyVisitor(Apache::Geode::Client::ICacheableKey^ key, Apache::Geode::Client::IGeodeSerializable^ value);

      generic <class TPropKey, class TPropValue>
      ref class PropertyVisitorProxy;

      /// <summary>
      /// Delegate that represents visitor for the <c>Properties</c> class.
      /// </summary>
      /// <remarks>
      /// This delegate is passed to the <c>Properties.ForEach</c> function
      /// that invokes this delegate for each property having a key
      /// and a value.
      /// </remarks>
      /// <param name="key">The key of the property.</param>
      /// <param name="value">The value of the property.</param>
      generic<class TPropKey, class TPropValue>
	    public delegate void PropertyVisitorGeneric( TPropKey key, TPropValue value );

      generic<class TPropKey, class TPropValue>
      [Serializable]
      /// <summary>
      /// Provides a collection of properties, each of which is a key/value
      /// pair. Each key is a string, and the value may be a string
      /// or an integer.
      /// </summary>
      public ref class Properties sealed
        : public IGeodeSerializable //,public ISerializable
      {
      public:

        /// <summary>
        /// Default constructor: returns an empty collection.
        /// </summary>
         inline Properties()
        : Properties(native::Properties::create())
        {
        
        }

        /// <summary>
        /// Factory method to create an empty collection of properties.
        /// </summary>
        /// <returns>empty collection of properties</returns>
        generic<class TPropKey, class TPropValue>
        inline static Properties<TPropKey, TPropValue>^ Create()
        {
          return gcnew Properties<TPropKey, TPropValue>();
        }


        /// <summary>
        /// Return the value for the given key, or NULL if not found. 
        /// </summary>
        /// <param name="key">the key to find</param>
        /// <returns>the value for the key</returns>
        /// <exception cref="NullPointerException">
        /// if the key is null
        /// </exception>
        TPropValue Find( TPropKey key );

        /// <summary>
        /// Add or update the string value for key.
        /// </summary>
        /// <param name="key">the key to insert</param>
        /// <param name="value">the string value to insert</param>
        /// <exception cref="NullPointerException">
        /// if the key is null
        /// </exception>
        void Insert( TPropKey key, TPropValue value );

        /// <summary>
        /// Remove the key from the collection. 
        /// </summary>
        /// <param name="key">the key to remove</param>
        /// <exception cref="NullPointerException">
        /// if the key is null
        /// </exception>
        void Remove( TPropKey key );

        /// <summary>
        /// Execute the Visitor delegate for each entry in the collection.
        /// </summary>
        /// <param name="visitor">visitor delegate</param>
        void ForEach( PropertyVisitorGeneric<TPropKey, TPropValue>^ visitor );

        /// <summary>
        /// Return the number of entries in the collection.
        /// </summary>
        /// <returns>the number of entries</returns>
        property System::UInt32 Size
        {
          System::UInt32 get( );
        }

        /*/// <summary>
        /// Adds the contents of <c>other</c> to this instance, replacing
        /// any existing values with those from other.
        /// </summary>
        /// <param name="other">new set of properties</param>*/
        void AddAll( Properties<TPropKey, TPropValue>^ other );

        /// <summary>
        /// Reads property values from a file, overriding what is currently
        /// in the properties object. 
        /// </summary>
        /// <param name="fileName">the name of the file</param>
        void Load( String^ fileName );

        /// <summary>
        /// Returns a string representation of the current
        /// <c>Properties</c> object.
        /// </summary>
        /// <returns>
        /// A comma separated list of property name,value pairs.
        /// </returns>
        virtual String^ ToString( ) override;

        // IGeodeSerializable members

        /// <summary>
        /// Serializes this Properties object.
        /// </summary>
        /// <param name="output">
        /// the DataOutput stream to use for serialization
        /// </param>
        virtual void ToData( DataOutput^ output );

        /// <summary>
        /// Deserializes this Properties object.
        /// </summary>
        /// <param name="input">
        /// the DataInput stream to use for reading data
        /// </param>
        /// <returns>the deserialized Properties object</returns>
        virtual IGeodeSerializable^ FromData( DataInput^ input );

        /// <summary>
        /// return the size of this object in bytes
        /// </summary>
        virtual property System::UInt32 ObjectSize
        {
          virtual System::UInt32 get( ); 
        }

        /// <summary>
        /// Returns the classId of this class for serialization.
        /// </summary>
        /// <returns>classId of the Properties class</returns>
        /// <seealso cref="IGeodeSerializable.ClassId" />
        virtual property System::UInt32 ClassId
        {
          inline virtual System::UInt32 get( )
          {
            return GeodeClassIds::Properties;
          }
        }

        // End: IGeodeSerializable members

        // ISerializable members

        //virtual void GetObjectData( SerializationInfo^ info,
        //  StreamingContext context);

        // End: ISerializable members

      protected:

        // For deserialization using the .NET serialization (ISerializable)
        //Properties(SerializationInfo^ info, StreamingContext context, native::SerializationRegistryPtr serializationRegistry);


      internal:

        /// <summary>
        /// Internal factory function to wrap a native object pointer inside
        /// this managed class with null pointer check.
        /// </summary>
        /// <param name="nativeptr">The native object pointer</param>
        /// <returns>
        /// The managed wrapper object; null if the native pointer is null.
        /// </returns>
        //generic<class TPropKey, class TPropValue>
        static Properties<TPropKey, TPropValue>^ Create( native::PropertiesPtr nativeptr )
        {
          return __nullptr == nativeptr ? nullptr :
            gcnew Properties<TPropKey, TPropValue>( nativeptr );
        }

        std::shared_ptr<native::Properties> GetNative()
        {
          return m_nativeptr->get_shared_ptr();
        }

        inline static IGeodeSerializable^ CreateDeserializable()
        {
          return Create<TPropKey, TPropValue>();
        }

      private:

        /// <summary>
        /// Private constructor to wrap a native object pointer
        /// </summary>
        /// <param name="nativeptr">The native object pointer</param>
        inline Properties( native::PropertiesPtr nativeptr )
        {
          m_nativeptr = gcnew native_shared_ptr<native::Properties>(nativeptr);
        }

        native_shared_ptr<native::Properties>^ m_nativeptr;

        void FromData(native::DataInput & input);
      };

      generic <class TPropKey, class TPropValue>
      ref class PropertyVisitorProxy
      {
      public:
        void Visit(Apache::Geode::Client::ICacheableKey^ key,
          Apache::Geode::Client::IGeodeSerializable^ value)
        {
          TPropKey tpkey = Apache::Geode::Client::Serializable::
            GetManagedValueGeneric<TPropKey>(SerializablePtr(SafeMSerializableConvertGeneric(key)));
          TPropValue tpvalue = Apache::Geode::Client::Serializable::
            GetManagedValueGeneric<TPropValue>(SerializablePtr(SafeMSerializableConvertGeneric(value)));
          m_visitor->Invoke(tpkey, tpvalue);
        }

        void SetPropertyVisitorGeneric(
          Apache::Geode::Client::PropertyVisitorGeneric<TPropKey, TPropValue>^ visitor)
        {
          m_visitor = visitor;
        }

      private:

        Apache::Geode::Client::PropertyVisitorGeneric<TPropKey, TPropValue>^ m_visitor;

      };

  /*    ref class PropertiesFactory {
      public:
          PropertiesFactory(native::SerializationRegistryPtr serializationRegistry)
          {
             m_serializationRegistry = gcnew native_shared_ptr<native::SerializationRegistry>(serializationRegistry);
          }
          IGeodeSerializable^ CreateDeserializable() {
            return Properties<String^, String^>::CreateDeserializable(m_serializationRegistry->get_shared_ptr());
          }
      private:
        native_shared_ptr<native::SerializationRegistry>^  m_serializationRegistry;
        };*/
    }  // namespace Client
  }  // namespace Geode
}  // namespace Apache


