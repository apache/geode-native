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
      /// <summary>
      /// Registry for custom serializable types, both PDXSerializable and DataSerializable.
      /// </summary>
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
        /// ISerializable.
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
        void RegisterType(TypeFactoryMethod^ creationMethod, int32_t id);

      internal:
        void RegisterDataSerializablePrimitiveOverrideNativeDeserialization(
            int8_t dsCode, TypeFactoryMethod^ creationMethod, Type^ managedType);

        void RegisterDataSerializableFixedIdTypeOverrideNativeDeserialization(
            Int32 fixedId, TypeFactoryMethod^ creationMethod);
        

        /// <summary>
        /// Unregister the type with the given typeId
        /// </summary>
        /// <param name="typeId">typeId of the type to unregister.</param>
        void UnregisterTypeGeneric(Byte typeId);

        /// <summary>
        /// This is to get manged delegates.
        /// </summary>
        TypeFactoryMethod^ GetManagedObjectFactory(System::Int64 typeId)
        {
          TypeFactoryMethod^ ret = nullptr;
          ObjectIDDelegatesMap->TryGetValue(typeId, ret);
          return ret;
        }
        System::Int32  GetIdForManagedType(System::Type^ type)
        {
          System::Int32 ret;
          ObjectTypeIDMap->TryGetValue(type, ret);
          return ret;
        }

        generic<class TValue>
          static TValue GetManagedValueGeneric(std::shared_ptr<native::Serializable> val);

        static void RegisterDataSerializablePrimitiveWrapper(
            DataSerializablePrimitiveWrapperDelegate^ wrapperMethod,
            int8_t dsCode, System::Type^ type);

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

        static inline int8_t GetDsCodeForManagedType(Type^ managedType)
        {
          int8_t retVal = 0;
          if (!ManagedTypeToDsCode->TryGetValue(managedType, retVal))
          {
            if (managedType->IsGenericType)
            {
              ManagedTypeToDsCode->TryGetValue(managedType->GetGenericTypeDefinition(), retVal);
            }
          }
          return retVal;
        }

        inline TypeFactoryMethod^ GetDataSerializableFixedTypeFactoryMethodForFixedId(Int32 fixedId)
        {
          return FixedIdToDataSerializableFixedIdTypeFactoryMethod[fixedId];
        }

        inline TypeFactoryMethod^ GetDataSerializablePrimitiveTypeFactoryMethodForDsCode(int8_t dsCode)
        {
          return DsCodeToDataSerializablePrimitiveTypeFactoryMethod[dsCode];
        }

        static inline DataSerializablePrimitiveWrapperDelegate^ GetDataSerializablePrimitiveWrapperDelegateForDsCode(int8_t dsCode)
        {
          return DsCodeToDataSerializablePrimitiveWrapperDelegate[dsCode];
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

        Dictionary<System::Int64, TypeFactoryMethod^>^ ObjectIDDelegatesMap =
          gcnew Dictionary<System::Int64, TypeFactoryMethod^>();
        List<TypeFactoryNativeMethodGeneric^>^ NativeDelegatesGeneric =
          gcnew List<TypeFactoryNativeMethodGeneric^>();
        Dictionary<UInt32, TypeFactoryMethod^>^ DelegateMapGeneric =
          gcnew Dictionary<UInt32, TypeFactoryMethod^>();

        Dictionary<System::Type^, System::Int32>^ ObjectTypeIDMap =
          gcnew Dictionary<System::Type^, System::Int32>();

        Dictionary<Byte, TypeFactoryMethod^>^ DsCodeToDataSerializablePrimitiveTypeFactoryMethod =
          gcnew Dictionary<Byte, TypeFactoryMethod^>();

        Dictionary<Byte, TypeFactoryNativeMethodGeneric^>^ DsCodeToDataSerializablePrimitiveNativeDelegate =
          gcnew Dictionary<Byte, TypeFactoryNativeMethodGeneric^>();

        Dictionary<Int32, TypeFactoryMethod^>^ FixedIdToDataSerializableFixedIdTypeFactoryMethod =
          gcnew Dictionary<Int32, TypeFactoryMethod^>();

        Dictionary<Int32, TypeFactoryNativeMethodGeneric^>^ FixedIdToDataSerializableFixedIdNativeDelegate =
          gcnew Dictionary<Int32, TypeFactoryNativeMethodGeneric^>();

        // Fixed .NET to DSCode mapping
        static Dictionary<System::Type^, int8_t>^ ManagedTypeToDsCode =
          gcnew Dictionary<System::Type^, int8_t>();

        static array<DataSerializablePrimitiveWrapperDelegate^>^ DsCodeToDataSerializablePrimitiveWrapperDelegate =
          gcnew array<DataSerializablePrimitiveWrapperDelegate^>(128);

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
          InitializeManagedTypeToDsCode();
          RegisterDataSerializablePrimitivesWrapNativeDeserialization();
        }

        /// <summary>
        /// Initializes a static map of .NET types to DataSerializable codes. Internally
        /// we call it TypeId.
        /// </summary>
        static void InitializeManagedTypeToDsCode()
        {
          Dictionary<Object^, Object^>^ dic = gcnew Dictionary<Object^, Object^>();
          ManagedTypeToDsCode[dic->GetType()] = static_cast<int8_t>(native::internal::DSCode::CacheableHashMap);
          ManagedTypeToDsCode[dic->GetType()->GetGenericTypeDefinition()] = static_cast<int8_t>(native::internal::DSCode::CacheableHashMap);

          System::Collections::ArrayList^ arr = gcnew System::Collections::ArrayList();
          ManagedTypeToDsCode[arr->GetType()] = static_cast<int8_t>(native::internal::DSCode::CacheableVector);

          System::Collections::Generic::LinkedList<Object^>^ linketList = gcnew  System::Collections::Generic::LinkedList<Object^>();
          ManagedTypeToDsCode[linketList->GetType()] = static_cast<int8_t>(native::internal::DSCode::CacheableLinkedList);
          ManagedTypeToDsCode[linketList->GetType()->GetGenericTypeDefinition()] = static_cast<int8_t>(native::internal::DSCode::CacheableLinkedList);

          System::Collections::Generic::IList<Object^>^ iList = gcnew System::Collections::Generic::List<Object^>();
          ManagedTypeToDsCode[iList->GetType()] = static_cast<int8_t>(native::internal::DSCode::CacheableArrayList);
          ManagedTypeToDsCode[iList->GetType()->GetGenericTypeDefinition()] = static_cast<int8_t>(native::internal::DSCode::CacheableArrayList);

          //TODO: Linked list, non generic stack, some other map types and see if more

          System::Collections::Generic::Stack<Object^>^ stack = gcnew System::Collections::Generic::Stack<Object^>();
          ManagedTypeToDsCode[stack->GetType()] = static_cast<int8_t>(native::internal::DSCode::CacheableStack);
          ManagedTypeToDsCode[stack->GetType()->GetGenericTypeDefinition()] = static_cast<int8_t>(native::internal::DSCode::CacheableStack);

          ManagedTypeToDsCode[Byte::typeid] = static_cast<int8_t>(native::internal::DSCode::CacheableByte);
          ManagedTypeToDsCode[Boolean::typeid] = static_cast<int8_t>(native::internal::DSCode::CacheableBoolean);
          ManagedTypeToDsCode[Char::typeid] = static_cast<int8_t>(native::internal::DSCode::CacheableCharacter);
          ManagedTypeToDsCode[Double::typeid] = static_cast<int8_t>(native::internal::DSCode::CacheableDouble);
          ManagedTypeToDsCode[String::typeid] = static_cast<int8_t>(native::internal::DSCode::CacheableASCIIString);
          ManagedTypeToDsCode[float::typeid] = static_cast<int8_t>(native::internal::DSCode::CacheableFloat);
          ManagedTypeToDsCode[Int16::typeid] = static_cast<int8_t>(native::internal::DSCode::CacheableInt16);
          ManagedTypeToDsCode[Int32::typeid] = static_cast<int8_t>(native::internal::DSCode::CacheableInt32);
          ManagedTypeToDsCode[Int64::typeid] = static_cast<int8_t>(native::internal::DSCode::CacheableInt64);
          ManagedTypeToDsCode[Type::GetType("System.Byte[]")] = static_cast<int8_t>(native::internal::DSCode::CacheableBytes);
          ManagedTypeToDsCode[Type::GetType("System.Double[]")] = static_cast<int8_t>(native::internal::DSCode::CacheableDoubleArray);
          ManagedTypeToDsCode[Type::GetType("System.Single[]")] = static_cast<int8_t>(native::internal::DSCode::CacheableFloatArray);
          ManagedTypeToDsCode[Type::GetType("System.Int16[]")] = static_cast<int8_t>(native::internal::DSCode::CacheableInt16Array);
          ManagedTypeToDsCode[Type::GetType("System.Int32[]")] = static_cast<int8_t>(native::internal::DSCode::CacheableInt32Array);
          ManagedTypeToDsCode[Type::GetType("System.Int64[]")] = static_cast<int8_t>(native::internal::DSCode::CacheableInt64Array);
          ManagedTypeToDsCode[Type::GetType("System.String[]")] = static_cast<int8_t>(native::internal::DSCode::CacheableStringArray);
          ManagedTypeToDsCode[Type::GetType("System.DateTime")] = static_cast<int8_t>(native::internal::DSCode::CacheableDate);
          ManagedTypeToDsCode[Type::GetType("System.Collections.Hashtable")] = static_cast<int8_t>(native::internal::DSCode::CacheableHashTable);
        }

        static void RegisterDataSerializablePrimitivesWrapNativeDeserialization();

      };
    }  // namespace Client
  }  // namespace Geode
}  // namespace Apache
