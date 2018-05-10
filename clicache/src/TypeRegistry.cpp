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
#include <geode/Cache.hpp>
#include <geode/PoolManager.hpp>
#include "SerializationRegistry.hpp"
#include "CacheRegionHelper.hpp"
#include "end_native.hpp"

#include "TypeRegistry.hpp"
#include "impl/DelegateWrapper.hpp"
#include "DataOutput.hpp"
#include "DataInput.hpp"
#include "CacheableStringArray.hpp"
#include "CacheableBuiltins.hpp"
#include "impl/SafeConvert.hpp"
#include "CacheableHashTable.hpp"
#include "Struct.hpp"
#include "CacheableUndefined.hpp"
#include "CacheableObject.hpp"
#include "CacheableStack.hpp"
#include "CacheableObjectXml.hpp"
#include "CacheableHashSet.hpp"
#include "CacheableObjectArray.hpp"
#include "CacheableLinkedList.hpp"
#include "CacheableFileName.hpp"
#include "CacheableIdentityHashMap.hpp"
#include "IPdxSerializer.hpp"
#include "impl/DotNetTypes.hpp"
#include "CacheRegionHelper.hpp"
#include "Cache.hpp"

using namespace apache::geode::client;
using namespace System::Reflection;
using namespace System::Reflection::Emit;

namespace Apache
{
  namespace Geode
  {
    namespace Client
    {
      namespace native = apache::geode::client;

      String^ TypeRegistry::GetPdxTypeName(String^ localTypeName)
      {
        if (pdxTypeMapper == nullptr)
        {
          return localTypeName;
        }

        String^ pdxTypeName;
        if (localTypeNameToPdx->TryGetValue(localTypeName, pdxTypeName)) {
          return pdxTypeName;
        }


        pdxTypeName = pdxTypeMapper->ToPdxTypeName(localTypeName);
        if (pdxTypeName == nullptr)
        {
          throw gcnew IllegalStateException("PdxTypeName should not be null for local type " + localTypeName);
        }

        localTypeNameToPdx[localTypeName] = pdxTypeName;
        pdxTypeNameToLocal[pdxTypeName] = localTypeName;

        return pdxTypeName;
      }

      String^ TypeRegistry::GetLocalTypeName(String^ pdxTypeName)
      {
        if (pdxTypeMapper == nullptr)
        {
          return pdxTypeName;
        }

        String^ localTypeName;
        if (pdxTypeNameToLocal->TryGetValue(pdxTypeName, localTypeName))
        {
          return localTypeName;
        }

        localTypeName = pdxTypeMapper->FromPdxTypeName(pdxTypeName);
        if (localTypeName == nullptr)
        {
          throw gcnew IllegalStateException("LocalTypeName should not be null for pdx type " + pdxTypeName);
        }

        localTypeNameToPdx[localTypeName] = pdxTypeName;
        pdxTypeNameToLocal[pdxTypeName] = localTypeName;

        return localTypeName;
      }

      Type^ TypeRegistry::GetType(String^ className)
      {
        Type^ type = nullptr;

        if (classNameVsType->TryGetValue(className, type)) {
          return type;
        }

        auto referedAssembly = gcnew Dictionary<Assembly^, bool>();
        auto MyDomain = AppDomain::CurrentDomain;
        array<Assembly^>^ AssembliesLoaded = MyDomain->GetAssemblies();
        for each(Assembly^ assembly in AssembliesLoaded)
        {
          type = GetTypeFromRefrencedAssemblies(className, referedAssembly, assembly);
          if (type) {
            classNameVsType[className] = type;
            return type;
          }
        }
        
        return type;
      }

      void TypeRegistry::RegisterPdxType(PdxTypeFactoryMethod^ creationMethod)
      {
        if (creationMethod == nullptr) {
          throw gcnew IllegalArgumentException("Serializable.RegisterPdxType(): "
            "null PdxTypeFactoryMethod delegate passed");
        }
        IPdxSerializable^ obj = creationMethod();
        PdxDelegateMap[obj->GetType()->FullName] = creationMethod;
        Log::Debug("RegisterPdxType: class registered: " + obj->GetType()->FullName);
      }

      IPdxSerializable^ TypeRegistry::GetPdxType(String^ className)
      {
        PdxTypeFactoryMethod^ retVal = nullptr;

        if (!PdxDelegateMap->TryGetValue(className, retVal))
        {
          if (pdxSerializer != nullptr)
          {
            return gcnew PdxWrapper(className);
          }
          try
          {
            Object^ retObj = CreateObject(className);

            IPdxSerializable^ retPdx = dynamic_cast<IPdxSerializable^>(retObj);
            if (retPdx != nullptr)
            {
              return retPdx;
            }
          }
          catch (System::Exception^ ex)
          {
            Log::Error("Unable to create object using reflection for class: " + className + " : " + ex->Message);
          }
          throw gcnew IllegalStateException("Pdx factory method (or PdxSerializer ) not registered (or don't have zero arg constructor)"
            " to create default instance for class: " + className);
        }

        return retVal();
      }

      void TypeRegistry::RegisterType(TypeFactoryMethod^ creationMethod)
      {
        if (creationMethod == nullptr) {
          throw gcnew IllegalArgumentException("Serializable.RegisterType(): "
            "null TypeFactoryMethod delegate passed");
        }

        //--------------------------------------------------------------

        auto typeRegistry = m_cache->TypeRegistry;
        int32_t classId;

        //adding user type as well in global builtin hashmap
        auto obj = creationMethod();
        if (auto dataSerializable = dynamic_cast<IDataSerializable^>(obj))
        {
          classId = dataSerializable->ClassId;
        } else
        {
          throw gcnew IllegalArgumentException("Unknown serialization type.");
        }

        if (!typeRegistry->ManagedDelegatesGeneric->ContainsKey(classId))
          typeRegistry->ManagedDelegatesGeneric->Add(classId, creationMethod);

        auto delegateObj = gcnew DelegateWrapperGeneric(creationMethod);
        auto nativeDelegate = gcnew TypeFactoryNativeMethodGeneric(delegateObj,
            &DelegateWrapperGeneric::NativeDelegateGeneric);

        // this is avoid object being Gced
        m_cache->TypeRegistry->NativeDelegatesGeneric->Add(nativeDelegate);

        // register the type in the DelegateMap, this is pure c# for create domain object 
        Log::Fine("Registering serializable class ID " + classId);
        m_cache->TypeRegistry->DelegateMapGeneric[classId] = creationMethod;

        _GF_MG_EXCEPTION_TRY2
          auto&& nativeTypeRegistry = CacheRegionHelper::getCacheImpl(m_cache->GetNative().get())->getSerializationRegistry();
          auto nativeDelegateFunction = static_cast<std::shared_ptr<native::Serializable>(*)()>(
              System::Runtime::InteropServices::Marshal::GetFunctionPointerForDelegate(nativeDelegate).ToPointer());
          nativeTypeRegistry->addType(nativeDelegateFunction);
        _GF_MG_EXCEPTION_CATCH_ALL2
      }

      void TypeRegistry::RegisterType(Byte typeId, TypeFactoryMethod^ creationMethod, Type^ type)
      {
        if (creationMethod == nullptr) {
          throw gcnew IllegalArgumentException("Serializable.RegisterType(): ");
        }
        DelegateWrapperGeneric^ delegateObj = gcnew DelegateWrapperGeneric(creationMethod);
        TypeFactoryNativeMethodGeneric^ nativeDelegate =
          gcnew TypeFactoryNativeMethodGeneric(delegateObj,
            &DelegateWrapperGeneric::NativeDelegateGeneric);

        m_cache->TypeRegistry->BuiltInDelegatesGeneric[typeId] = nativeDelegate;

        if (type != nullptr)
        {
          m_cache->TypeRegistry->ManagedTypeToTypeId[type] = typeId;
        }

        auto typeRegistry = m_cache->TypeRegistry;

        //This is hashmap for manged builtin objects
        int32_t classId = typeId + 0x80000000;
        if (!typeRegistry->ManagedDelegatesGeneric->ContainsKey(classId))
        {
          typeRegistry->ManagedDelegatesGeneric->Add(classId, creationMethod);
        }

        // register the type in the DelegateMap
        Log::Finer("Registering(,) serializable class ID " + classId);
        m_cache->TypeRegistry->DelegateMapGeneric[classId] = creationMethod;

        try
        {
          auto&& serializationRegistry = CacheRegionHelper::getCacheImpl(m_cache->GetNative().get())->getSerializationRegistry();
          auto nativeDelegateFunction = static_cast<std::shared_ptr<native::Serializable>(*)()>(
              System::Runtime::InteropServices::Marshal::GetFunctionPointerForDelegate(nativeDelegate).ToPointer());
          if (classId < 0xa0000000)
          {
            serializationRegistry->addType(typeId, nativeDelegateFunction);
          }
          else
          {
            //special case for CacheableUndefined type
            serializationRegistry->addType2(typeId, nativeDelegateFunction);
          }

        }
        catch (native::IllegalStateException&)
        {
          //ignore it as this is internal only
        }
      }

      void TypeRegistry::UnregisterTypeGeneric(Byte typeId)
      {
        BuiltInDelegatesGeneric->Remove(typeId);
        _GF_MG_EXCEPTION_TRY2

          CacheRegionHelper::getCacheImpl(m_cache->GetNative().get())->getSerializationRegistry()->removeType(typeId);

        _GF_MG_EXCEPTION_CATCH_ALL2
      }

      void TypeRegistry::RegisterWrapperGeneric(
        WrapperDelegateGeneric^ wrapperMethod, Byte typeId, System::Type^ type)
      {
        if (typeId < 0 || typeId > WrapperEndGeneric)
        {
          throw gcnew GeodeException("The TypeID (" + typeId + ") being "
            "registered is beyond acceptable range of 0-" + WrapperEndGeneric);
        }
        NativeWrappersGeneric[typeId] = wrapperMethod;
        ManagedTypeToTypeId[type] = typeId;
      }

      void TypeRegistry::UnregisterNativesGeneric(Cache^ cache)
      {
        cache->TypeRegistry->BuiltInDelegatesGeneric->Clear();
        for (Byte typeId = 0; typeId <= WrapperEndGeneric; ++typeId) {
          cache->TypeRegistry->NativeWrappersGeneric[typeId] = nullptr;
        }
        //TODO:: unregister from managed hashmap as well.
        //  ManagedDelegates->Clear();
      }

      Type^ TypeRegistry::GetTypeFromRefrencedAssemblies(String^ className, Dictionary<Assembly^, bool>^ referedAssembly, Assembly^ currentAssembly)
      {
        auto type = currentAssembly->GetType(className);
        if (type != nullptr)
        {          
          return type;
        }

        if (referedAssembly->ContainsKey(currentAssembly))
          return nullptr;
        referedAssembly[currentAssembly] = true;

        //get all refrenced assembly
        array<AssemblyName^>^ ReferencedAssemblies = currentAssembly->GetReferencedAssemblies();
        for each(AssemblyName^ assembly in ReferencedAssemblies)
        {
          try
          {
            Assembly^ loadedAssembly = Assembly::Load(assembly);
            if (loadedAssembly != nullptr && (!referedAssembly->ContainsKey(loadedAssembly)))
            {
              type = GetTypeFromRefrencedAssemblies(className, referedAssembly, loadedAssembly);
              if (!type) {
                return type;
              }
            }
          }
          catch (System::Exception^){//ignore
          }
        }
        return nullptr;
      }

      generic<class TValue>
      TValue wrap(std::shared_ptr<native::DataSerializablePrimitive> dataSerializablePrimitive)
      {
        switch (dataSerializablePrimitive->getDsCode())
        {
          case native::GeodeTypeIds::CacheableDate:
          {
            auto ret = SafeGenericUMSerializableConvert<CacheableDate^>(dataSerializablePrimitive);
            return safe_cast<TValue>(ret->Value);
          }
          case native::GeodeTypeIds::CacheableBytes:
          {
            auto ret = SafeGenericUMSerializableConvert<CacheableBytes^>(dataSerializablePrimitive);
            return safe_cast<TValue>(ret->Value);
          }
          case native::GeodeTypeIds::CacheableDoubleArray:
          {
            auto ret = SafeGenericUMSerializableConvert<CacheableDoubleArray^>(dataSerializablePrimitive);
            return safe_cast<TValue>(ret->Value);
          }
          case native::GeodeTypeIds::CacheableFloatArray:
          {
            auto ret = SafeGenericUMSerializableConvert<CacheableFloatArray^>(dataSerializablePrimitive);
            return safe_cast<TValue>(ret->Value);
          }
          case native::GeodeTypeIds::CacheableInt16Array:
          {
            auto ret = SafeGenericUMSerializableConvert<CacheableInt16Array^>(dataSerializablePrimitive);
            return safe_cast<TValue>(ret->Value);
          }
          case native::GeodeTypeIds::CacheableInt32Array:
          {
            auto ret = SafeGenericUMSerializableConvert<CacheableInt32Array^>(dataSerializablePrimitive);
            return safe_cast<TValue>(ret->Value);
          }
          case native::GeodeTypeIds::CacheableInt64Array:
          {
            auto ret = SafeGenericUMSerializableConvert<CacheableInt64Array^>(dataSerializablePrimitive);
            return safe_cast<TValue>(ret->Value);
          }
          case native::GeodeTypeIds::CacheableStringArray:
          {
            auto ret = SafeGenericUMSerializableConvert<CacheableStringArray^>(dataSerializablePrimitive);
            return safe_cast<TValue>(ret->GetValues());
          }
          case native::GeodeTypeIds::CacheableArrayList://Ilist generic
          {
            auto ret = SafeGenericUMSerializableConvert<CacheableArrayList^>(dataSerializablePrimitive);
            return safe_cast<TValue>(ret->Value);
          }
          case native::GeodeTypeIds::CacheableLinkedList://LinkedList generic
          {
            auto ret = SafeGenericUMSerializableConvert<CacheableLinkedList^>(dataSerializablePrimitive);
            return safe_cast<TValue>(ret->Value);
          }
          case native::GeodeTypeIds::CacheableHashTable://collection::hashtable
          {
            auto ret = SafeGenericUMSerializableConvert<CacheableHashTable^>(dataSerializablePrimitive);
            return safe_cast<TValue>(ret->Value);
          }
          case native::GeodeTypeIds::CacheableHashMap://generic dictionary
          {
            auto ret = SafeGenericUMSerializableConvert<CacheableHashMap^>(dataSerializablePrimitive);
            return safe_cast<TValue>(ret->Value);
          }
          case native::GeodeTypeIds::CacheableIdentityHashMap:
          {
            auto ret = SafeGenericUMSerializableConvert<CacheableIdentityHashMap^>(dataSerializablePrimitive);
            return safe_cast<TValue>(ret->Value);
          }
          case native::GeodeTypeIds::CacheableHashSet://no need of it, default case should work
          {
            auto ret = SafeGenericUMSerializableConvert<CacheableHashSet^>(dataSerializablePrimitive);
            return safe_cast<TValue>(ret);
          }
          case native::GeodeTypeIds::CacheableLinkedHashSet://no need of it, default case should work
          {
            auto ret = SafeGenericUMSerializableConvert<CacheableLinkedHashSet^>(dataSerializablePrimitive);
            return safe_cast<TValue>(ret);
          }
          case native::GeodeTypeIds::CacheableFileName:
          {
            auto ret = SafeGenericUMSerializableConvert<CacheableFileName^>(dataSerializablePrimitive);
            return safe_cast<TValue>(ret);
          }
          case native::GeodeTypeIds::CacheableObjectArray:
          {
            auto ret = SafeGenericUMSerializableConvert<CacheableObjectArray^>(dataSerializablePrimitive);
            return safe_cast<TValue>(ret);
          }
          case native::GeodeTypeIds::CacheableVector://collection::arraylist
          {
            auto ret = SafeGenericUMSerializableConvert<CacheableVector^>(dataSerializablePrimitive);
            return safe_cast<TValue>(ret->Value);
          }
          case native::GeodeTypeIds::CacheableUndefined:
          {
            auto ret = SafeGenericUMSerializableConvert<CacheableUndefined^>(dataSerializablePrimitive);
            return safe_cast<TValue>(ret);
          }
          case native::GeodeTypeIds::Struct:
          {
            return safe_cast<TValue>(Struct::Create(dataSerializablePrimitive));
          }
          case native::GeodeTypeIds::CacheableStack:
          {
            auto ret = SafeGenericUMSerializableConvert<CacheableStack^>(dataSerializablePrimitive);
            return safe_cast<TValue>(ret->Value);
          }
          case 7: //GeodeClassIds::CacheableManagedObject
          {
            auto ret = SafeGenericUMSerializableConvert<CacheableObject^>(dataSerializablePrimitive);
            return safe_cast<TValue>(ret);
          }
          case 8://GeodeClassIds::CacheableManagedObjectXml
          {
            auto ret = SafeGenericUMSerializableConvert<CacheableObjectXml^>(dataSerializablePrimitive);
            return safe_cast<TValue>(ret);
          }
          /*
          case native::GeodeTypeIds::Properties: // TODO: replace with IDictionary<K, V>
          {
            auto ret = SafeGenericUMSerializableConvert<Properties^>(dataSerializablePrimitive);
            return safe_cast<TValue>(ret);
          }
          */
          case native::GeodeTypeIds::BooleanArray:
          {
            auto ret = SafeGenericUMSerializableConvert<BooleanArray^>(dataSerializablePrimitive);
            return safe_cast<TValue>(ret->Value);
          }
          case native::GeodeTypeIds::CharArray:
          {
            auto ret = SafeGenericUMSerializableConvert<CharArray^>(dataSerializablePrimitive);
            return safe_cast<TValue>(ret->Value);
          }
          case 0://UserFunctionExecutionException unregistered
          {
            auto ret = SafeGenericUMSerializableConvert<UserFunctionExecutionException^>(dataSerializablePrimitive);
            return safe_cast<TValue>(ret);
          }
        }

        throw gcnew IllegalArgumentException("Unknown type");
      }

      generic<class TValue>
      TValue TypeRegistry::GetManagedValueGeneric(std::shared_ptr<native::Serializable> val)
      {
        if (val == nullptr)
        {
          return TValue();
        }

        if (auto dataSerializablePrimitive = std::dynamic_pointer_cast<native::DataSerializablePrimitive>(val))
        {
          switch (dataSerializablePrimitive->getDsCode())
          {
            case native::GeodeTypeIds::CacheableByte:
            {
              return (TValue)(int8_t)safe_cast<int8_t>(Serializable::getByte(dataSerializablePrimitive));
            }
            case native::GeodeTypeIds::CacheableBoolean:
            {
              return safe_cast<TValue>(Serializable::getBoolean(dataSerializablePrimitive));
            }
            case native::GeodeTypeIds::CacheableCharacter:
            {
              return safe_cast<TValue>(Serializable::getChar(dataSerializablePrimitive));
            }
            case native::GeodeTypeIds::CacheableDouble:
            {
              return safe_cast<TValue>(Serializable::getDouble(dataSerializablePrimitive));
            }
            case native::GeodeTypeIds::CacheableASCIIString:
            case native::GeodeTypeIds::CacheableASCIIStringHuge:
            case native::GeodeTypeIds::CacheableString:
            case native::GeodeTypeIds::CacheableStringHuge:
            {
              //TODO serializable: need to look all strings types
              return safe_cast<TValue>(Serializable::getASCIIString(dataSerializablePrimitive));
            }
            case native::GeodeTypeIds::CacheableFloat:
            {
              return safe_cast<TValue>(Serializable::getFloat(dataSerializablePrimitive));
            }
            case native::GeodeTypeIds::CacheableInt16:
            {
              return safe_cast<TValue>(Serializable::getInt16(dataSerializablePrimitive));
            }
            case native::GeodeTypeIds::CacheableInt32:
            {
              return safe_cast<TValue>(Serializable::getInt32(dataSerializablePrimitive));
            }
            case native::GeodeTypeIds::CacheableInt64:
            {
              return safe_cast<TValue>(Serializable::getInt64(dataSerializablePrimitive));
            }
            default:
              return wrap<TValue>(dataSerializablePrimitive);
          }
        }
        else if (auto dataSerializable = std::dynamic_pointer_cast<native::DataSerializable>(val))
        {
          auto ret = SafeUMSerializableConvertGeneric(dataSerializable);
          return safe_cast<TValue>(ret);
        }
        else if (auto pdxSerializable = std::dynamic_pointer_cast<native::PdxSerializable>(val))
        {
          auto ret = SafeUMSerializablePDXConvert(pdxSerializable);
          if (auto pdxWrapper = dynamic_cast<PdxWrapper^>(ret))
          {
            return safe_cast<TValue>(pdxWrapper->GetObject());
          }

          return safe_cast<TValue>(ret);
        }
        else
        {
          throw gcnew IllegalStateException("Unknown serialization type.");
        }

        throw gcnew System::Exception("not found typeid");
      }

      Object^ TypeRegistry::CreateObject(String^ className)
      {
        Object^ retVal = CreateObjectEx(className);

        if (retVal == nullptr)
        {
          auto type = GetType(className);
          if (type)
          {
            retVal = type->GetConstructor(Type::EmptyTypes)->Invoke(nullptr);
            return retVal;
          }
        }
        return retVal;
      }

      Object^ TypeRegistry::CreateObjectEx(String^ className)
      {
        CreateNewObjectDelegate^ del = nullptr;
        Dictionary<String^, CreateNewObjectDelegate^>^ tmp = ClassNameVsCreateNewObjectDelegate;

        tmp->TryGetValue(className, del);

        if (del != nullptr)
        {
          return del();
        }
        auto type = GetType(className);
        if (type)
        {
          msclr::lock lockInstance(ClassNameVsCreateNewObjectLockObj);
          {
            tmp = ClassNameVsCreateNewObjectDelegate;
            tmp->TryGetValue(className, del);
            if (del != nullptr)
              return del();
            del = CreateNewObjectDelegateF(type);
            ClassNameVsCreateNewObjectDelegate[className] = del;
            return del();
          }
        }
        return nullptr;
      }

      Object^ TypeRegistry::GetArrayObject(String^ className, int length)
      {
        Object^ retArr = GetArrayObjectEx(className, length);
        if (retArr == nullptr)
        {
          Type^ type = GetType(className);
          if (type)
          {
            retArr = type->MakeArrayType()->GetConstructor(singleIntType)->Invoke(gcnew array<Object^>(1) { length });
            return retArr;
          }
        }
        return retArr;
      }

      Object^ TypeRegistry::GetArrayObjectEx(String^ className, int length)
      {
        CreateNewObjectArrayDelegate^ del = nullptr;
        Dictionary<String^, CreateNewObjectArrayDelegate^>^ tmp = ClassNameVsCreateNewObjectArrayDelegate;

        tmp->TryGetValue(className, del);

        if (del != nullptr)
        {
          return del(length);
        }

        Type^ t = GetType(className);
        if (t)
        {
          msclr::lock lockInstance(ClassNameVsCreateNewObjectLockObj);
          {
            tmp = ClassNameVsCreateNewObjectArrayDelegate;
            tmp->TryGetValue(className, del);
            if (del != nullptr)
              return del(length);
            del = CreateNewObjectArrayDelegateF(t);
            tmp = gcnew Dictionary<String^, CreateNewObjectArrayDelegate^>(ClassNameVsCreateNewObjectArrayDelegate);
            tmp[className] = del;
            ClassNameVsCreateNewObjectArrayDelegate = tmp;
            return del(length);
          }
        }
        return nullptr;
      }

      //delegate Object^ CreateNewObject();
      //static CreateNewObjectDelegate^ CreateNewObjectDelegateF(Type^ type);
      TypeRegistry::CreateNewObjectDelegate^ TypeRegistry::CreateNewObjectDelegateF(Type^ type)
      {
        DynamicMethod^ dynam = gcnew DynamicMethod("", Internal::DotNetTypes::ObjectType, Type::EmptyTypes, type, true);
        ILGenerator^ il = dynam->GetILGenerator();

        ConstructorInfo^ ctorInfo = type->GetConstructor(Type::EmptyTypes);
        if (ctorInfo == nullptr) {
          Log::Error("Object missing public no arg constructor");
          throw gcnew IllegalStateException("Object missing public no arg constructor");
        }

        il->Emit(OpCodes::Newobj, ctorInfo);
        il->Emit(OpCodes::Ret);

        return (TypeRegistry::CreateNewObjectDelegate^)dynam->CreateDelegate(createNewObjectDelegateType);
      }

      //delegate Object^ CreateNewObjectArray(int len);
      TypeRegistry::CreateNewObjectArrayDelegate^ TypeRegistry::CreateNewObjectArrayDelegateF(Type^ type)
      {
        DynamicMethod^ dynam = gcnew DynamicMethod("", Internal::DotNetTypes::ObjectType, singleIntTypeA, type, true);
        ILGenerator^ il = dynam->GetILGenerator();

        il->Emit(OpCodes::Ldarg_0);

        il->Emit(OpCodes::Newarr, type);
        il->Emit(OpCodes::Ret);

        return (TypeRegistry::CreateNewObjectArrayDelegate^)dynam->CreateDelegate(createNewObjectArrayDelegateType);
      }


    }  // namespace Client
  }  // namespace Geode
}  // namespace Apache
