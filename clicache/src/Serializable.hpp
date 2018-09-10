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
#include <geode/CacheableKey.hpp>
#include <geode/CacheableBuiltins.hpp>
#include "end_native.hpp"

#include "ISerializable.hpp"
#include "IDelta.hpp"
#include "native_shared_ptr.hpp"
#include "impl/EnumInfo.hpp"
#include "Log.hpp"
#include <vcclr.h>
#include "IPdxTypeMapper.hpp"

using namespace System::Reflection;
using namespace System;
using namespace System::Collections::Generic;

namespace Apache
{
  namespace Geode
  {
    namespace Client
    {
      using namespace apache::geode::client;
      namespace native = apache::geode::client;

        interface class IPdxSerializable;
        interface class IPdxSerializer;
        ref class Cache;

      /// <summary>
      /// Signature of native function delegates passed to native
      /// <c>native::Serializable::registerType</c>.
      /// Such functions should return an empty instance of the type they
      /// represent. The instance will typically be initialized immediately
      /// after creation by a call to native
      /// <c>native::Serializable::fromData</c>.
      /// </summary>
      delegate std::shared_ptr<native::Serializable> TypeFactoryNativeMethodGeneric();

      /// <summary>
      /// Signature of function delegates passed to
      /// <see cref="Serializable.RegisterType" />. Such functions should
      /// return an empty instance of the type they represent.
      /// The delegate shall be stored in the internal <c>DelegateWrapper</c>
      /// class and an instance will be initialized in the
      /// <c>DelegateWrapper.NativeDelegate</c> method by a call to
      /// <see cref="ISerializable.FromData" />.
      /// </summary>
      public delegate Apache::Geode::Client::ISerializable^ TypeFactoryMethod();
      /// <summary>
      /// Delegate to wrap a native <c>native::Serializable</c> type.
      /// </summary>
      /// <remarks>
      /// This delegate should return an object of type <c>ISerializable</c>
      /// given a native object.
      /// </remarks>
      delegate Apache::Geode::Client::ISerializable^ DataSerializablePrimitiveWrapperDelegate(std::shared_ptr<native::Serializable> obj);

			/// <summary>
      /// Signature of function delegates passed to
      /// <see cref="Serializable.RegisterPdxType" />. Such functions should
      /// return an empty instance of the type they represent.
      /// New instance will be created during de-serialization of Pdx Types
      /// <see cref="IPdxSerializable" />.
      /// </summary>
      public delegate Apache::Geode::Client::IPdxSerializable^ PdxTypeFactoryMethod();
      
      /// <summary>
      /// This class wraps the native C++ <c>native::Serializable</c> objects
      /// as managed <see cref="ISerializable" /> objects.
      /// </summary>
      public ref class Serializable
        : public Apache::Geode::Client::ISerializable
      {
      public:        
        /// <summary>
        /// return the size of this object in bytes
        /// </summary>
        virtual property System::UInt64 ObjectSize
        {
          virtual System::UInt64 get(); 
        }

        /// <summary>
        /// Return a string representation of the object.
        /// It simply returns the string representation of the underlying
        /// native object by calling its <c>toString()</c> function.
        /// </summary>
        virtual String^ ToString() override;


      internal:
        static std::shared_ptr<CacheableKey> GetNativeCacheableKeyWrapperForManagedISerializable(ISerializable^ managedObject);
				static System::Int32 GetPDXIdForType(native::Pool* pool, ISerializable^ pdxType, Cache^ cache);
				static ISerializable^ GetPDXTypeById(native::Pool* pool, System::Int32 typeId, Cache^ cache);
        
        static int GetEnumValue(Internal::EnumInfo^ ei, Cache^ cache);
        static Internal::EnumInfo^ GetEnum(int val, Cache^ cache);

        // These are the new static methods to get/put data from c++

        //byte
        static Byte getByte(std::shared_ptr<native::Serializable> nativeptr);
        
        static std::shared_ptr<native::CacheableKey> getCacheableByte(Byte val);
        
        //boolean
        static bool getBoolean(std::shared_ptr<native::Serializable> nativeptr);
        
        static std::shared_ptr<native::CacheableKey> getCacheableBoolean(bool val);
        
        //widechar
        static Char getChar(std::shared_ptr<native::Serializable> nativeptr);
        
        static std::shared_ptr<native::CacheableKey> getCacheableWideChar(Char val);
        
        //double
        static double getDouble(std::shared_ptr<native::Serializable> nativeptr);
        
        static std::shared_ptr<native::CacheableKey> getCacheableDouble(double val);
        
        //float
        static float getFloat(std::shared_ptr<native::Serializable> nativeptr);
        
        static std::shared_ptr<native::CacheableKey> getCacheableFloat(float val);
        
        //int16
        static System::Int16 getInt16(std::shared_ptr<native::Serializable> nativeptr);
        
        static std::shared_ptr<native::CacheableKey> getCacheableInt16(int val);
        
        //int32
        static System::Int32 getInt32(std::shared_ptr<native::Serializable> nativeptr);
        
        static std::shared_ptr<native::CacheableKey> getCacheableInt32(System::Int32 val);
        
        //int64
        static System::Int64 getInt64(std::shared_ptr<native::Serializable> nativeptr);
        
        static std::shared_ptr<native::CacheableKey> getCacheableInt64(System::Int64 val);
        
        static String^ getString(std::shared_ptr<native::Serializable> nativeptr);
                
        static std::shared_ptr<native::CacheableString> GetCacheableString(String^ value);

        static array<Byte>^ getSByteArray(array<SByte>^ sArray);
        
        static array<System::Int16>^ getInt16Array(array<System::UInt16>^ sArray);
        
        static array<System::Int32>^ getInt32Array(array<System::UInt32>^ sArray);        

        static array<System::Int64>^ getInt64Array(array<System::UInt64>^ sArray);
        

        /// <summary>
        /// Default constructor.
        /// </summary>
        inline Apache::Geode::Client::Serializable()
        :Serializable(__nullptr) { }

        /// <summary>
        /// Internal constructor to wrap a native object pointer
        /// </summary>
        /// <param name="nativeptr">The native object pointer</param>
        inline Apache::Geode::Client::Serializable(std::shared_ptr<native::Serializable> nativeptr)
        {
          m_nativeptr = gcnew native_shared_ptr<native::Serializable>(nativeptr);
        }

        generic<class TKey>
        static std::shared_ptr<native::CacheableKey> GetUnmanagedValueGeneric(TKey ky);

        generic<class TKey>
        static std::shared_ptr<native::CacheableKey> GetUnmanagedValueGeneric(TKey key, bool isAciiChar);

        generic<class TKey>
        static std::shared_ptr<native::CacheableKey> GetUnmanagedValueGeneric(Type^ managedType, TKey key);

        generic<class TKey>
        static std::shared_ptr<native::CacheableKey> GetUnmanagedValueGeneric(Type^ managedType, TKey key, bool isAsciiChar);

        static Serializable()
        {
          
        }

        protected:
          native_shared_ptr<native::Serializable>^ m_nativeptr;
      };
    }  // namespace Client
  }  // namespace Geode
}  // namespace Apache

