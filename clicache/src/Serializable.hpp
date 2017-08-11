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

#include "IGeodeSerializable.hpp"
#include "IGeodeDelta.hpp"
#include "impl/ManagedString.hpp"
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
      delegate native::Serializable* TypeFactoryNativeMethodGeneric();

      /// <summary>
      /// Signature of function delegates passed to
      /// <see cref="Serializable.RegisterType" />. Such functions should
      /// return an empty instance of the type they represent.
      /// The delegate shall be stored in the internal <c>DelegateWrapper</c>
      /// class and an instance will be initialized in the
      /// <c>DelegateWrapper.NativeDelegate</c> method by a call to
      /// <see cref="IGeodeSerializable.FromData" />.
      /// </summary>
      public delegate Apache::Geode::Client::IGeodeSerializable^ TypeFactoryMethodGeneric();
      /// <summary>
      /// Delegate to wrap a native <c>native::Serializable</c> type.
      /// </summary>
      /// <remarks>
      /// This delegate should return an object of type <c>IGeodeSerializable</c>
      /// given a native object.
      /// </remarks>
      delegate Apache::Geode::Client::IGeodeSerializable^ WrapperDelegateGeneric(native::SerializablePtr obj);

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
      /// as managed <see cref="IGeodeSerializable" /> objects.
      /// </summary>
      public ref class Serializable
        : public Apache::Geode::Client::IGeodeSerializable
      {
      public:
        /// <summary>
        /// Serializes this native (C++) object.
        /// </summary>
        /// <param name="output">
        /// the DataOutput object to use for serializing the object
        /// </param>
        virtual void ToData(Apache::Geode::Client::DataOutput^ output);

        /// <summary>
        /// Deserializes the native (C++) object -- returns an instance of the
        /// <c>Serializable</c> class with the native object wrapped inside.
        /// </summary>
        /// <param name="input">
        /// the DataInput stream to use for reading the object data
        /// </param>
        /// <returns>the deserialized object</returns>
        virtual Apache::Geode::Client::IGeodeSerializable^
          FromData(Apache::Geode::Client::DataInput^ input);
        
        /// <summary>
        /// return the size of this object in bytes
        /// </summary>
        virtual property System::UInt32 ObjectSize
        {
          virtual System::UInt32 get(); 
        }

        /// <summary>
        /// Returns the classId of the instance being serialized.
        /// This is used by deserialization to determine what instance
        /// type to create and deserialize into.
        /// </summary>
        /// <returns>the classId</returns>
        virtual property System::UInt32 ClassId
        {
          virtual System::UInt32 get();
        }

        /// <summary>
        /// Return a string representation of the object.
        /// It simply returns the string representation of the underlying
        /// native object by calling its <c>toString()</c> function.
        /// </summary>
        virtual String^ ToString() override;

        // Static conversion function from primitive types string, integer
        // and byte array.

        /// <summary>
        /// Implicit conversion operator from a boolean
        /// to a <c>Serializable</c>.
        /// </summary>
        static operator Apache::Geode::Client::Serializable^ (bool value);

        /// <summary>
        /// Implicit conversion operator from a byte
        /// to a <c>Serializable</c>.
        /// </summary>
        static operator Apache::Geode::Client::Serializable^ (Byte value);

        /// <summary>
        /// Implicit conversion operator from an array of bytes
        /// to a <c>Serializable</c>.
        /// </summary>
        static operator Apache::Geode::Client::Serializable^ (array<Byte>^ value);

        /// <summary>
        /// Implicit conversion operator from an boolean array
        /// to a <c>Serializable</c>.
        /// </summary>
        static operator Apache::Geode::Client::Serializable^ (array<bool>^ value);

        /// <summary>
        /// Implicit conversion operator from a double
        /// to a <c>Serializable</c>.
        /// </summary>
        static operator Apache::Geode::Client::Serializable^ (Double value);

        /// <summary>
        /// Implicit conversion operator from a double array
        /// to a <c>Serializable</c>.
        /// </summary>
        static operator Apache::Geode::Client::Serializable^ (array<Double>^ value);

        /// <summary>
        /// Implicit conversion operator from a float
        /// to a <c>Serializable</c>.
        /// </summary>
        static operator Apache::Geode::Client::Serializable^ (Single value);

        /// <summary>
        /// Implicit conversion operator from a float array
        /// to a <c>Serializable</c>.
        /// </summary>
        static operator Apache::Geode::Client::Serializable^ (array<Single>^ value);

        /// <summary>
        /// Implicit conversion operator from a 16-bit integer
        /// to a <c>Serializable</c>.
        /// </summary>
        static operator Apache::Geode::Client::Serializable^ (System::Int16 value);

        /// <summary>
        /// Implicit conversion operator from a character
        /// to a <c>Serializable</c>.
        /// </summary>
        static operator Apache::Geode::Client::Serializable^ (Char value);

        /// <summary>
        /// Implicit conversion operator from a character array
        /// to a <c>Serializable</c>.
        /// </summary>
        static operator Apache::Geode::Client::Serializable^ (array<Char>^ value);

        /// <summary>
        /// Implicit conversion operator from a 16-bit integer array
        /// to a <c>Serializable</c>.
        /// </summary>
        static operator Apache::Geode::Client::Serializable^ (array<System::Int16>^ value);

        /// <summary>
        /// Implicit conversion operator from a 32-bit integer
        /// to a <c>Serializable</c>.
        /// </summary>
        static operator Apache::Geode::Client::Serializable^ (System::Int32 value);

        /// <summary>
        /// Implicit conversion operator from a 32-bit integer array
        /// to a <c>Serializable</c>.
        /// </summary>
        static operator Apache::Geode::Client::Serializable^ (array<System::Int32>^ value);

        /// <summary>
        /// Implicit conversion operator from a 64-bit integer
        /// to a <c>Serializable</c>.
        /// </summary>
        static operator /*Apache::Geode::Client::*/Serializable^ (System::Int64 value);

        /// <summary>
        /// Implicit conversion operator from a 64-bit integer array
        /// to a <c>Serializable</c>.
        /// </summary>
        static operator Apache::Geode::Client::Serializable^ (array<System::Int64>^ value);

        /// <summary>
        /// Implicit conversion operator from a string
        /// to a <c>Serializable</c>.
        /// </summary>
        static operator Apache::Geode::Client::Serializable^ (String^ value);

        /// <summary>
        /// Implicit conversion operator from a string array
        /// to a <c>Serializable</c>.
        /// </summary>
        static operator Apache::Geode::Client::Serializable^ (array<String^>^ value);
        
        
        /// <summary>
        /// Register an instance factory method for a given type.
        /// This should be used when registering types that implement
        /// IGeodeSerializable.
        /// </summary>
        /// <param name="creationMethod">
        /// the creation function to register
        /// </param>
        /// <exception cref="IllegalArgumentException">
        /// if the method is null
        /// </exception>
        /// <exception cref="IllegalStateException">
        /// if the typeId has already been registered, or there is an error
        /// in registering the type; check <c>Utils::LastError</c> for more
        /// information in the latter case.
        /// </exception>
        static void RegisterTypeGeneric(TypeFactoryMethodGeneric^ creationMethod, Cache^ cache);

        /// <summary>
        /// Set the PDX serializer for the cache. If this serializer is set,
        /// it will be consulted to see if it can serialize any domain classes which are 
        /// added to the cache in portable data exchange format. 
        /// </summary>
        static void RegisterPdxSerializer(IPdxSerializer^ pdxSerializer);
        
				/// <summary>
        /// Register an instance factory method for a given type.
        /// This should be used when registering types that implement
        /// IPdxSerializable.
        /// </summary>
        /// <param name="creationMethod">
        /// the creation function to register
        /// </param>
        /// <exception cref="IllegalArgumentException">
        /// if the method is null
        /// </exception>
        
        static void RegisterPdxType(PdxTypeFactoryMethod^ creationMethod);

        /// <summary>
        /// Register an PdxTypeMapper to map the local types to pdx types
        /// </summary>
        /// <param name="pdxTypeMapper">
        /// Object which implements IPdxTypeMapper interface
        /// </param>
       

        static void SetPdxTypeMapper(IPdxTypeMapper^ pdxTypeMapper);        

      internal:

				static System::Int32 GetPDXIdForType(const char* poolName, IGeodeSerializable^ pdxType, const native::Cache* cache);
				static IGeodeSerializable^ GetPDXTypeById(const char* poolName, System::Int32 typeId, const native::Cache* cache);
				static IPdxSerializable^ Serializable::GetPdxType(String^ className);
				static void RegisterPDXManagedCacheableKey(bool appDomainEnable, Cache^ cache);
        static bool IsObjectAndPdxSerializerRegistered(String^ className);

        static IPdxSerializer^ GetPdxSerializer();
        static String^ GetPdxTypeName(String^ localTypeName);
        static String^ GetLocalTypeName(String^ pdxTypeName);
        static void Clear();

        static Type^ GetType(String^ className);

        static int GetEnumValue(Internal::EnumInfo^ ei, const native::Cache* cache);
        static Internal::EnumInfo^ GetEnum(int val, const native::Cache* cache);

         static Dictionary<String^, PdxTypeFactoryMethod^>^ PdxDelegateMap =
          gcnew Dictionary<String^, PdxTypeFactoryMethod^>();
       
        static String^ GetString(native::CacheableStringPtr cStr);//native::CacheableString*
        
        // These are the new static methods to get/put data from c++

        //byte
        static Byte getByte(native::SerializablePtr nativeptr);
        
        static native::CacheableKeyPtr getCacheableByte(SByte val);
        
        //boolean
        static bool getBoolean(native::SerializablePtr nativeptr);
        
        static native::CacheableKeyPtr getCacheableBoolean(bool val);
        
        //widechar
        static Char getChar(native::SerializablePtr nativeptr);
        
        static native::CacheableKeyPtr getCacheableWideChar(Char val);
        
        //double
        static double getDouble(native::SerializablePtr nativeptr);
        
        static native::CacheableKeyPtr getCacheableDouble(double val);
        
        //float
        static float getFloat(native::SerializablePtr nativeptr);
        
        static native::CacheableKeyPtr getCacheableFloat(float val);
        
        //int16
        static System::Int16 getInt16(native::SerializablePtr nativeptr);
        
        static native::CacheableKeyPtr getCacheableInt16(int val);
        
        //int32
        static System::Int32 getInt32(native::SerializablePtr nativeptr);
        
        static native::CacheableKeyPtr getCacheableInt32(System::Int32 val);
        
        //int64
        static System::Int64 getInt64(native::SerializablePtr nativeptr);
        
        static native::CacheableKeyPtr getCacheableInt64(System::Int64 val);
        
        //cacheable ascii string
        static String^ getASCIIString(native::SerializablePtr nativeptr);        

        static native::CacheableKeyPtr getCacheableASCIIString(String^ val);

        static native::CacheableKeyPtr getCacheableASCIIString2(String^ val);
        
        //cacheable ascii string huge
        static String^ getASCIIStringHuge(native::SerializablePtr nativeptr);
        
        static native::CacheableKeyPtr getCacheableASCIIStringHuge(String^ val);        

        //cacheable string
        static String^ getUTFString(native::SerializablePtr nativeptr);        

        static native::CacheableKeyPtr getCacheableUTFString(String^ val);
        

        //cacheable string huge
        static String^ getUTFStringHuge(native::SerializablePtr nativeptr);
        

        static native::CacheableKeyPtr getCacheableUTFStringHuge(String^ val);
        

       static native::CacheableStringPtr GetCacheableString(String^ value);       

       static native::CacheableStringPtr GetCacheableString2(String^ value); 

       /*
        static String^ GetString(native::CacheableStringPtr cStr)//native::CacheableString*
        {
          if (cStr == nullptr) {
            return nullptr;
          }
          else if (cStr->isWideString()) {
            return ManagedString::Get(cStr->asWChar());
          }
          else {
            return ManagedString::Get(cStr->asChar());
          }
        }
        */

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
        inline Apache::Geode::Client::Serializable(native::SerializablePtr nativeptr)
        {
          m_nativeptr = gcnew native_shared_ptr<native::Serializable>(nativeptr);
        }

        /// <summary>
        /// Register an instance factory method for a given type and typeId.
        /// This should be used when registering types that implement
        /// IGeodeSerializable.
        /// </summary>
        /// <param name="typeId">typeId of the type being registered.</param>
        /// <param name="creationMethod">
        /// the creation function to register
        /// </param>
        /// <exception cref="IllegalArgumentException">
        /// if the method is null
        /// </exception>
        static void RegisterTypeGeneric(Byte typeId, TypeFactoryMethodGeneric^ creationMethod, Type^ type, Cache^ cache);


        /// <summary>
        /// Unregister the type with the given typeId
        /// </summary>
        /// <param name="typeId">typeId of the type to unregister.</param>
        static void UnregisterTypeGeneric(Byte typeId, Cache^ cache);

        generic<class TValue>
        static TValue GetManagedValueGeneric(native::SerializablePtr val);

        generic<class TKey>
        static native::CacheableKeyPtr GetUnmanagedValueGeneric(TKey key, native::Cache* cache);

        generic<class TKey>
        static native::CacheableKeyPtr GetUnmanagedValueGeneric(TKey key, bool isAciiChar, native::Cache* cache);

        generic<class TKey>
        static native::CacheableKeyPtr GetUnmanagedValueGeneric(
          Type^ managedType, TKey key, native::Cache* cache);

        generic<class TKey>
        static native::CacheableKeyPtr GetUnmanagedValueGeneric(
          Type^ managedType, TKey key, bool isAsciiChar, native::Cache* cache);

        /// <summary>
        /// Static map of <c>TypeFactoryMethod</c> delegates created
        /// for managed <c>TypeFactoryMethod</c> delegates.
        /// </summary>
        static Dictionary<System::Type^, Byte>^ ManagedTypeMappingGeneric =
          gcnew Dictionary<System::Type^, Byte>();

        static Byte GetManagedTypeMappingGeneric (Type^ type)
        {
          Byte retVal = 0;
          ManagedTypeMappingGeneric->TryGetValue(type, retVal);
          return retVal;
        }

        /// <summary>
        /// Static list of <c>TypeFactoryNativeMethod</c> delegates created
        /// from registered managed <c>TypeFactoryMethod</c> delegates.
        /// This is so that the underlying managed objects do not get GCed.
        /// </summary>
        static List<TypeFactoryNativeMethodGeneric^>^ NativeDelegatesGeneric =
          gcnew List<TypeFactoryNativeMethodGeneric^>();

        /// <summary>
        /// Static map of <c>TypeFactoryMethod</c> delegates created
        /// from registered managed <c>TypeFactoryMethod</c> delegates.
        /// This is for cross AppDomain object creations.
        /// </summary>
        static Dictionary<UInt32, TypeFactoryMethodGeneric^>^ DelegateMapGeneric =
          gcnew Dictionary<UInt32, TypeFactoryMethodGeneric^>();

        static Dictionary<UInt32, TypeFactoryMethodGeneric^>^ InternalDelegateMapGeneric =
          gcnew Dictionary<UInt32, TypeFactoryMethodGeneric^>();

        static TypeFactoryMethodGeneric^ GetTypeFactoryMethodGeneric(UInt32 classid)
        {
         // Log::Finer("TypeFactoryMethodGeneric type id " + classid + " domainid :" + System::Threading::Thread::GetDomainID() );
          if(DelegateMapGeneric->ContainsKey(classid) )
            return DelegateMapGeneric[classid];
          else
            return InternalDelegateMapGeneric[classid];//builtin types
        }

        /// <summary>
        /// Static map of <c>TypeFactoryNativeMethod</c> delegates created
        /// for builtin managed <c>TypeFactoryMethod</c> delegates.
        /// This is so that the underlying managed objects do not get GCed.
        /// </summary>
        static Dictionary<Byte, TypeFactoryNativeMethodGeneric^>^ BuiltInDelegatesGeneric =
          gcnew Dictionary<Byte, TypeFactoryNativeMethodGeneric^>();

        /// <summary>
        /// Static map of <c>TypeFactoryMethod</c> delegates created
        /// for managed <c>TypeFactoryMethod</c> delegates.
        /// </summary>
        static Dictionary<System::Int64, TypeFactoryMethodGeneric^>^ ManagedDelegatesGeneric =
          gcnew Dictionary<System::Int64, TypeFactoryMethodGeneric^>();

        /// <summary>
        /// This is to get manged delegates.
        /// </summary>
        static TypeFactoryMethodGeneric^ GetManagedDelegateGeneric(System::Int64 typeId)
        {
          TypeFactoryMethodGeneric^ ret = nullptr;
          ManagedDelegatesGeneric->TryGetValue(typeId, ret);
          return ret;
        }

        static IPdxSerializer^ PdxSerializer = nullptr;
        static IPdxTypeMapper^ PdxTypeMapper = nullptr;
        static Object^ LockObj = gcnew Object();
        static Dictionary<String^, String^>^ PdxTypeNameToLocal =
          gcnew Dictionary<String^, String^>();
        static Dictionary<String^, String^>^ LocalTypeNameToPdx =
          gcnew Dictionary<String^, String^>();


        static Object^ ClassNameVsTypeLockObj = gcnew Object();
        static Dictionary<String^, Type^>^ ClassNameVsType =
          gcnew Dictionary<String^, Type^>();

        delegate Object^ CreateNewObjectDelegate();
        static CreateNewObjectDelegate^ CreateNewObjectDelegateF(Type^ type);
       
        delegate Object^ CreateNewObjectArrayDelegate(int len);
        static CreateNewObjectArrayDelegate^ CreateNewObjectArrayDelegateF(Type^ type);
        
        static array<Type^>^ singleIntTypeA = gcnew array<Type^>{ Int32::typeid };

        static Type^ createNewObjectDelegateType = Type::GetType("Apache.Geode.Client.Serializable+CreateNewObjectDelegate");
        static Type^ createNewObjectArrayDelegateType = Type::GetType("Apache.Geode.Client.Serializable+CreateNewObjectArrayDelegate");

        static array<Type^>^ singleIntType = gcnew array<Type^>(1){Int32::typeid};

        static Object^ CreateObject(String^ className);
        static Object^ GetArrayObject(String^ className, int len);
        static Type^ getTypeFromRefrencedAssemblies(String^ className, Dictionary<Assembly^, bool>^ referedAssembly, Assembly^ currentAsm);

        static Dictionary<String^, CreateNewObjectDelegate^>^ ClassNameVsCreateNewObjectDelegate =
          gcnew Dictionary<String^, CreateNewObjectDelegate^>();

        static Dictionary<String^, CreateNewObjectArrayDelegate^>^ ClassNameVsCreateNewObjectArrayDelegate =
          gcnew Dictionary<String^, CreateNewObjectArrayDelegate^>();

        static Object^ CreateObjectEx(String^ className);
        static Object^ GetArrayObjectEx(String^ className, int len);
        /// <summary>
        /// Static array of managed <c>WrapperDelegate</c> delegates that
        /// maintains a mapping of built-in native typeIds to their corresponding
        /// wrapper type delegates.
        /// </summary>
        /// <remarks>
        /// This is as an array to make lookup as fast as possible, taking
        /// advantage of the fact that the range of wrapped built-in typeIds is
        /// small. <b>IMPORTANT:</b> If the built-in native typeIds encompass a
        /// greater range then change <c>WrapperEnd</c> in this accordingly
        /// or move to using a Dictionary instead.
        /// </remarks>
        static array<WrapperDelegateGeneric^>^ NativeWrappersGeneric =
          gcnew array<WrapperDelegateGeneric^>(WrapperEndGeneric + 1);
        literal Byte WrapperEndGeneric = 128;

        /// <summary>
        /// Static method to register a managed wrapper for a native
        /// <c>native::Serializable</c> type.
        /// </summary>
        /// <param name="wrapperMethod">
        /// A factory delegate of the managed wrapper class that returns the
        /// managed object given the native object.
        /// </param>
        /// <param name="typeId">The typeId of the native type.</param>
        /// <seealso cref="NativeWrappers" />
        static void RegisterWrapperGeneric(WrapperDelegateGeneric^ wrapperMethod,
          Byte typeId, System::Type^ type);

        /// <summary>
        /// Internal static method to remove managed artifacts created by
        /// RegisterType and RegisterWrapper methods when
        /// <see cref="DistributedSystem.Disconnect" /> is called.
        /// </summary>
        static void UnregisterNativesGeneric();

        /// <summary>
        /// Static method to lookup the wrapper delegate for a given typeId.
        /// </summary>
        /// <param name="typeId">
        /// The typeId of the native <c>native::Serializable</c> type.
        /// </param>
        /// <returns>
        /// If a managed wrapper is registered for the given typeId then the
        /// wrapper delegate is returned, else this returns null.
        /// </returns>
        inline static WrapperDelegateGeneric^ GetWrapperGeneric(Byte typeId)
        {
          if (typeId >= 0 && typeId <= WrapperEndGeneric) {
            return NativeWrappersGeneric[typeId];
          }
          return nullptr;
        }

				static Serializable()
        {
          PdxTypeMapper = nullptr;
          //RegisterPDXManagedCacheableKey();

          {
          Dictionary<Object^, Object^>^ dic = gcnew Dictionary<Object^, Object^>();
          ManagedTypeMappingGeneric[dic->GetType()] = native::GeodeTypeIds::CacheableHashMap;
          ManagedTypeMappingGeneric[dic->GetType()->GetGenericTypeDefinition()] = native::GeodeTypeIds::CacheableHashMap;
          }

          {
          System::Collections::ArrayList^ arr = gcnew System::Collections::ArrayList();
          ManagedTypeMappingGeneric[arr->GetType()] = native::GeodeTypeIds::CacheableVector;
          }
		  
          {
          System::Collections::Generic::LinkedList<Object^>^ linketList = gcnew  System::Collections::Generic::LinkedList<Object^>();
          ManagedTypeMappingGeneric[linketList->GetType()] = native::GeodeTypeIds::CacheableLinkedList;
          ManagedTypeMappingGeneric[linketList->GetType()->GetGenericTypeDefinition()] = native::GeodeTypeIds::CacheableLinkedList;
          }
		  
          {
          System::Collections::Generic::IList<Object^>^ iList = gcnew System::Collections::Generic::List<Object^>();
          ManagedTypeMappingGeneric[iList->GetType()] = native::GeodeTypeIds::CacheableArrayList;
          ManagedTypeMappingGeneric[iList->GetType()->GetGenericTypeDefinition()] = native::GeodeTypeIds::CacheableArrayList;
          }

          //TODO: Linked list, non generic stack, some other map types and see if more

          {
            System::Collections::Generic::Stack<Object^>^ stack = gcnew System::Collections::Generic::Stack<Object^>();
            ManagedTypeMappingGeneric[stack->GetType()] = native::GeodeTypeIds::CacheableStack;
            ManagedTypeMappingGeneric[stack->GetType()->GetGenericTypeDefinition()] = native::GeodeTypeIds::CacheableStack;
          }
          {
            ManagedTypeMappingGeneric[SByte::typeid] = native::GeodeTypeIds::CacheableByte;
            ManagedTypeMappingGeneric[Boolean::typeid] = native::GeodeTypeIds::CacheableBoolean;
            ManagedTypeMappingGeneric[Char::typeid] = native::GeodeTypeIds::CacheableWideChar;
            ManagedTypeMappingGeneric[Double::typeid] = native::GeodeTypeIds::CacheableDouble;
            ManagedTypeMappingGeneric[String::typeid] = native::GeodeTypeIds::CacheableASCIIString;
            ManagedTypeMappingGeneric[float::typeid] = native::GeodeTypeIds::CacheableFloat;
            ManagedTypeMappingGeneric[Int16::typeid] = native::GeodeTypeIds::CacheableInt16;
            ManagedTypeMappingGeneric[Int32::typeid] = native::GeodeTypeIds::CacheableInt32;
            ManagedTypeMappingGeneric[Int64::typeid] = native::GeodeTypeIds::CacheableInt64;
            ManagedTypeMappingGeneric[Type::GetType("System.Byte[]")] = native::GeodeTypeIds::CacheableBytes;
            ManagedTypeMappingGeneric[Type::GetType("System.Double[]")] = native::GeodeTypeIds::CacheableDoubleArray;
            ManagedTypeMappingGeneric[Type::GetType("System.Single[]")] = native::GeodeTypeIds::CacheableFloatArray;
            ManagedTypeMappingGeneric[Type::GetType("System.Int16[]")] = native::GeodeTypeIds::CacheableInt16Array;
            ManagedTypeMappingGeneric[Type::GetType("System.Int32[]")] = native::GeodeTypeIds::CacheableInt32Array;
            ManagedTypeMappingGeneric[Type::GetType("System.Int64[]")] = native::GeodeTypeIds::CacheableInt64Array;
            ManagedTypeMappingGeneric[Type::GetType("System.String[]")] = native::GeodeTypeIds::CacheableStringArray;
            ManagedTypeMappingGeneric[Type::GetType("System.DateTime")] = native::GeodeTypeIds::CacheableDate;
            ManagedTypeMappingGeneric[Type::GetType("System.Collections.Hashtable")] = native::GeodeTypeIds::CacheableHashTable;
          }
        }

        protected:
          native_shared_ptr<native::Serializable>^ m_nativeptr;
      };
    }  // namespace Client
  }  // namespace Geode
}  // namespace Apache

