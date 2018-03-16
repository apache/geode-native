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

      public ref class TypeRegistry
      {
      public:
        TypeRegistry(Cache^ cache) :m_cache(cache)
        {
          Dictionary<Object^, Object^>^ dic = gcnew Dictionary<Object^, Object^>();
          ManagedTypeMappingGeneric[dic->GetType()] = native::GeodeTypeIds::CacheableHashMap;
          ManagedTypeMappingGeneric[dic->GetType()->GetGenericTypeDefinition()] = native::GeodeTypeIds::CacheableHashMap;

          System::Collections::ArrayList^ arr = gcnew System::Collections::ArrayList();
          ManagedTypeMappingGeneric[arr->GetType()] = native::GeodeTypeIds::CacheableVector;

          System::Collections::Generic::LinkedList<Object^>^ linketList = gcnew  System::Collections::Generic::LinkedList<Object^>();
          ManagedTypeMappingGeneric[linketList->GetType()] = native::GeodeTypeIds::CacheableLinkedList;
          ManagedTypeMappingGeneric[linketList->GetType()->GetGenericTypeDefinition()] = native::GeodeTypeIds::CacheableLinkedList;

          System::Collections::Generic::IList<Object^>^ iList = gcnew System::Collections::Generic::List<Object^>();
          ManagedTypeMappingGeneric[iList->GetType()] = native::GeodeTypeIds::CacheableArrayList;
          ManagedTypeMappingGeneric[iList->GetType()->GetGenericTypeDefinition()] = native::GeodeTypeIds::CacheableArrayList;

        //TODO: Linked list, non generic stack, some other map types and see if more
        
          System::Collections::Generic::Stack<Object^>^ stack = gcnew System::Collections::Generic::Stack<Object^>();
          ManagedTypeMappingGeneric[stack->GetType()] = native::GeodeTypeIds::CacheableStack;
          ManagedTypeMappingGeneric[stack->GetType()->GetGenericTypeDefinition()] = native::GeodeTypeIds::CacheableStack;
        
          ManagedTypeMappingGeneric[SByte::typeid] = native::GeodeTypeIds::CacheableByte;
          ManagedTypeMappingGeneric[Boolean::typeid] = native::GeodeTypeIds::CacheableBoolean;
          ManagedTypeMappingGeneric[Char::typeid] = native::GeodeTypeIds::CacheableCharacter;
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
        void RegisterTypeGeneric(Byte typeId, TypeFactoryMethodGeneric^ creationMethod, Type^ type);


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
          TValue GetManagedValueGeneric(std::shared_ptr<native::Serializable> val);

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
        void RegisterWrapperGeneric(WrapperDelegateGeneric^ wrapperMethod,
          Byte typeId, System::Type^ type);

        /// <summary>
        /// Internal static method to remove managed artifacts created by
        /// RegisterType and RegisterWrapper methods when
        /// <see cref="DistributedSystem.Disconnect" /> is called.
        /// </summary>
        void UnregisterNativesGeneric();

        void Clear()
        {
          pdxTypeNameToLocal->Clear();
          localTypeNameToPdx->Clear();
          classNameVsType->Clear();
        }

        Byte GetManagedTypeMappingGeneric(Type^ type)
        {
          Byte retVal = 0;
          ManagedTypeMappingGeneric->TryGetValue(type, retVal);
          return retVal;
        }

        inline WrapperDelegateGeneric^ GetWrapperGeneric(Byte typeId)
        {
          if (typeId >= 0 && typeId <= WrapperEndGeneric) {
            return NativeWrappersGeneric[typeId];
          }
          return nullptr;
        }

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
        Dictionary<System::Type^, Byte>^ ManagedTypeMappingGeneric =
          gcnew Dictionary<System::Type^, Byte>();
        literal Byte WrapperEndGeneric = 128;
        array<WrapperDelegateGeneric^>^ NativeWrappersGeneric =
          gcnew array<WrapperDelegateGeneric^>(WrapperEndGeneric + 1);

        Type^ GetTypeFromRefrencedAssemblies(String^ className, Dictionary<Assembly^, bool>^ referedAssembly, Assembly^ currentAssembly);
      };
    }  // namespace Client
  }  // namespace Geode
}  // namespace Apache

