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

#include "IPdxSerializer.hpp"
#include "Serializable.hpp"
#include <geode/CacheableBuiltins.hpp>



namespace Apache
{
  namespace Geode
  {
    namespace Client
    {

      using namespace System::Collections::Concurrent;
      using namespace System::Reflection;

      namespace native = apache::geode::client;
      
      ref class Cache;

      public ref class TypeRegistry
      {
      public:
        TypeRegistry(Cache^ cache) : m_cache(cache)
        {
          ClassNameVsCreateNewObjectLockObj = gcnew Object();

          singleIntTypeA = gcnew array<Type^>{ Int32::typeid };
          singleIntType = gcnew array<Type^>(1) { Int32::typeid };

          ClassNameVsCreateNewObjectDelegate =
            gcnew Dictionary<String^, CreateNewObjectDelegate^>();
          ClassNameVsCreateNewObjectArrayDelegate =
            gcnew Dictionary<String^, CreateNewObjectArrayDelegate^>();

          createNewObjectDelegateType = Type::GetType("Apache.Geode.Client.TypeRegistry+CreateNewObjectDelegate");
          createNewObjectArrayDelegateType = Type::GetType("Apache.Geode.Client.TypeRegistry+CreateNewObjectArrayDelegate");
        }

        property IPdxSerializer^ PdxSerializer
        {
          IPdxSerializer^ get() {
            return pdxSerializer; 
          }

          void set(IPdxSerializer^ pdxSerializer) {
            this->pdxSerializer = pdxSerializer; 
          }
        }

        /// <summary>
        /// Register an PdxTypeMapper to map the local types to pdx types
        /// </summary>
        property IPdxTypeMapper^ PdxTypeMapper
        {
          IPdxTypeMapper^ get()
          {
            return pdxTypeMapper;
          }

          void set(IPdxTypeMapper^ pdxTypeMapper)
          {
            this->pdxTypeMapper = pdxTypeMapper;
          }
        }

        String^ GetPdxTypeName(String^ localTypeName);
 
        String^ GetLocalTypeName(String^ pdxTypeName);

        Type^ GetType(String^ className);

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

        void RegisterPdxType(PdxTypeFactoryMethod^ creationMethod);

        IPdxSerializable^ GetPdxType(String^ className);

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
        void RegisterTypeGeneric(TypeFactoryMethodGeneric^ creationMethod);

      internal:

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
        void RegisterTypeGeneric(Byte typeId, TypeFactoryMethodGeneric^ creationMethod, 
          Type^ type);


        /// <summary>
        /// Unregister the type with the given typeId
        /// </summary>
        /// <param name="typeId">typeId of the type to unregister.</param>
        void UnregisterTypeGeneric(Byte typeId);

        /// <summary>
        /// This is to get manged delegates.
        /// </summary>
        TypeFactoryMethodGeneric^ GetManagedDelegateGeneric(System::Int64 typeId)
        {
          TypeFactoryMethodGeneric^ ret = nullptr;
          ManagedDelegatesGeneric->TryGetValue(typeId, ret);
          return ret;
        }

        generic<class TValue>
          static TValue GetManagedValueGeneric(std::shared_ptr<native::Serializable> val);

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
        static void UnregisterNativesGeneric(Cache^ cache);

        void Clear()
        {
          pdxTypeNameToLocal->Clear();
          localTypeNameToPdx->Clear();
          classNameVsType->Clear();
          ClassNameVsCreateNewObjectDelegate->Clear();
          ClassNameVsCreateNewObjectArrayDelegate->Clear();
        }

        static Byte GetManagedTypeMappingGeneric(Type^ type)
        {
          Byte retVal = 0;
          ManagedTypeToTypeId->TryGetValue(type, retVal);
          return retVal;
        }

        static inline WrapperDelegateGeneric^ GetWrapperGeneric(Byte typeId)
        {
          if (typeId >= 0 && typeId <= WrapperEndGeneric) {
            return NativeWrappersGeneric[typeId];
          }
          return nullptr;
        }
        
        delegate Object^ CreateNewObjectDelegate();
        CreateNewObjectDelegate^ CreateNewObjectDelegateF(Type^ type);

        delegate Object^ CreateNewObjectArrayDelegate(int len);
        CreateNewObjectArrayDelegate^ CreateNewObjectArrayDelegateF(Type^ type);

        Object^ CreateObject(String^ className);
        Object^ GetArrayObject(String^ className, int length);

        Object^ CreateObjectEx(String^ className);
        Object^ GetArrayObjectEx(String^ className, int length);

      private:
        Cache^ m_cache;
        IPdxSerializer^ pdxSerializer;
        IPdxTypeMapper^ pdxTypeMapper;
        ConcurrentDictionary<String^, String^>^ pdxTypeNameToLocal =
          gcnew ConcurrentDictionary<String^, String^>();
        ConcurrentDictionary<String^, String^>^ localTypeNameToPdx =
          gcnew ConcurrentDictionary<String^, String^>();
        ConcurrentDictionary<String^, Type^>^ classNameVsType =
          gcnew ConcurrentDictionary<String^, Type^>();
        Dictionary<String^, PdxTypeFactoryMethod^>^ PdxDelegateMap =
          gcnew Dictionary<String^, PdxTypeFactoryMethod^>();

        Dictionary<System::Int64, TypeFactoryMethodGeneric^>^ ManagedDelegatesGeneric =
          gcnew Dictionary<System::Int64, TypeFactoryMethodGeneric^>();
        List<TypeFactoryNativeMethodGeneric^>^ NativeDelegatesGeneric =
          gcnew List<TypeFactoryNativeMethodGeneric^>();
        Dictionary<UInt32, TypeFactoryMethodGeneric^>^ DelegateMapGeneric =
          gcnew Dictionary<UInt32, TypeFactoryMethodGeneric^>();

        Dictionary<Byte, TypeFactoryNativeMethodGeneric^>^ BuiltInDelegatesGeneric =
          gcnew Dictionary<Byte, TypeFactoryNativeMethodGeneric^>();

        // Fixed .NET to DSCode mapping
        static Dictionary<System::Type^, Byte>^ ManagedTypeToTypeId =
          gcnew Dictionary<System::Type^, Byte>();

        literal Byte WrapperEndGeneric = 128;
        static array<WrapperDelegateGeneric^>^ NativeWrappersGeneric =
          gcnew array<WrapperDelegateGeneric^>(WrapperEndGeneric + 1);

        Type^ GetTypeFromRefrencedAssemblies(String^ className, Dictionary<Assembly^, bool>^ referedAssembly, Assembly^ currentAssembly);
        
        Object^ ClassNameVsCreateNewObjectLockObj;
        
        array<Type^>^ singleIntTypeA;
        array<Type^>^ singleIntType;
        
        Dictionary<String^, CreateNewObjectDelegate^>^ ClassNameVsCreateNewObjectDelegate;
                Dictionary<String^, CreateNewObjectArrayDelegate^>^ ClassNameVsCreateNewObjectArrayDelegate;
        
        Type^ createNewObjectDelegateType;
        Type^ createNewObjectArrayDelegateType;

        static TypeRegistry()
        {
          InitializeManagedTypeToTypeId();
        }

        /// <summary>
        /// Initializes a static map of .NET types to DataSerializable codes. Internally
        /// we call it TypeId.
        /// </summary>
        static void InitializeManagedTypeToTypeId()
        {
          Dictionary<Object^, Object^>^ dic = gcnew Dictionary<Object^, Object^>();
          ManagedTypeToTypeId[dic->GetType()] = native::GeodeTypeIds::CacheableHashMap;
          ManagedTypeToTypeId[dic->GetType()->GetGenericTypeDefinition()] = native::GeodeTypeIds::CacheableHashMap;

          System::Collections::ArrayList^ arr = gcnew System::Collections::ArrayList();
          ManagedTypeToTypeId[arr->GetType()] = native::GeodeTypeIds::CacheableVector;

          System::Collections::Generic::LinkedList<Object^>^ linketList = gcnew  System::Collections::Generic::LinkedList<Object^>();
          ManagedTypeToTypeId[linketList->GetType()] = native::GeodeTypeIds::CacheableLinkedList;
          ManagedTypeToTypeId[linketList->GetType()->GetGenericTypeDefinition()] = native::GeodeTypeIds::CacheableLinkedList;

          System::Collections::Generic::IList<Object^>^ iList = gcnew System::Collections::Generic::List<Object^>();
          ManagedTypeToTypeId[iList->GetType()] = native::GeodeTypeIds::CacheableArrayList;
          ManagedTypeToTypeId[iList->GetType()->GetGenericTypeDefinition()] = native::GeodeTypeIds::CacheableArrayList;

          //TODO: Linked list, non generic stack, some other map types and see if more

          System::Collections::Generic::Stack<Object^>^ stack = gcnew System::Collections::Generic::Stack<Object^>();
          ManagedTypeToTypeId[stack->GetType()] = native::GeodeTypeIds::CacheableStack;
          ManagedTypeToTypeId[stack->GetType()->GetGenericTypeDefinition()] = native::GeodeTypeIds::CacheableStack;

          ManagedTypeToTypeId[SByte::typeid] = native::GeodeTypeIds::CacheableByte;
          ManagedTypeToTypeId[Boolean::typeid] = native::GeodeTypeIds::CacheableBoolean;
          ManagedTypeToTypeId[Char::typeid] = native::GeodeTypeIds::CacheableCharacter;
          ManagedTypeToTypeId[Double::typeid] = native::GeodeTypeIds::CacheableDouble;
          ManagedTypeToTypeId[String::typeid] = native::GeodeTypeIds::CacheableASCIIString;
          ManagedTypeToTypeId[float::typeid] = native::GeodeTypeIds::CacheableFloat;
          ManagedTypeToTypeId[Int16::typeid] = native::GeodeTypeIds::CacheableInt16;
          ManagedTypeToTypeId[Int32::typeid] = native::GeodeTypeIds::CacheableInt32;
          ManagedTypeToTypeId[Int64::typeid] = native::GeodeTypeIds::CacheableInt64;
          ManagedTypeToTypeId[Type::GetType("System.Byte[]")] = native::GeodeTypeIds::CacheableBytes;
          ManagedTypeToTypeId[Type::GetType("System.Double[]")] = native::GeodeTypeIds::CacheableDoubleArray;
          ManagedTypeToTypeId[Type::GetType("System.Single[]")] = native::GeodeTypeIds::CacheableFloatArray;
          ManagedTypeToTypeId[Type::GetType("System.Int16[]")] = native::GeodeTypeIds::CacheableInt16Array;
          ManagedTypeToTypeId[Type::GetType("System.Int32[]")] = native::GeodeTypeIds::CacheableInt32Array;
          ManagedTypeToTypeId[Type::GetType("System.Int64[]")] = native::GeodeTypeIds::CacheableInt64Array;
          ManagedTypeToTypeId[Type::GetType("System.String[]")] = native::GeodeTypeIds::CacheableStringArray;
          ManagedTypeToTypeId[Type::GetType("System.DateTime")] = native::GeodeTypeIds::CacheableDate;
          ManagedTypeToTypeId[Type::GetType("System.Collections.Hashtable")] = native::GeodeTypeIds::CacheableHashTable;
        }

      };
    }  // namespace Client
  }  // namespace Geode
}  // namespace Apache

