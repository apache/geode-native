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
        if (!typeRegistry->ManagedDelegatesGeneric->ContainsKey(typeId + 0x80000000))
        {
          typeRegistry->ManagedDelegatesGeneric->Add(typeId + 0x80000000, creationMethod);
        }

        // register the type in the DelegateMap
        /* TODO serializable 
        IGeodeSerializable^ tmp = creationMethod();
        Log::Finer("Registering(,) serializable class ID " + tmp->ClassId);
        m_cache->TypeRegistry->DelegateMapGeneric[tmp->ClassId] = creationMethod;

        try
        {
          CacheImpl *cacheImpl = CacheRegionHelper::getCacheImpl(m_cache->GetNative().get());
          if (tmp->ClassId < 0xa0000000)
          {
            cacheImpl->getSerializationRegistry()->addType(typeId,
              (std::shared_ptr<native::Serializable>(*)())System::Runtime::InteropServices::
              Marshal::GetFunctionPointerForDelegate(
                nativeDelegate).ToPointer());
          }
          else
          {//special case for CacheableUndefined type
            cacheImpl->getSerializationRegistry()->addType2(typeId,
              (std::shared_ptr<native::Serializable>(*)())System::Runtime::InteropServices::
              Marshal::GetFunctionPointerForDelegate(
                nativeDelegate).ToPointer());
          }

        }
        catch (native::IllegalStateException&)
        {
          //ignore it as this is internal only
        }
        */
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
      TValue TypeRegistry::GetManagedValueGeneric(std::shared_ptr<native::Serializable> val)
      {
        if (val == nullptr)
        {
          return TValue();
        }

        // TODO serializable
        /*
        Byte typeId = val->typeId();
        //Log::Debug("TypeRegistry::GetManagedValueGeneric typeid = " + typeId);
        switch (typeId)
        {
        case native::GeodeTypeIds::CacheableByte:
        {
          return (TValue)(int8_t)safe_cast<int8_t>(Serializable::getByte(val));
        }
        case native::GeodeTypeIds::CacheableBoolean:
        {
          return safe_cast<TValue>(Serializable::getBoolean(val));
        }
        case native::GeodeTypeIds::CacheableCharacter:
        {
          return safe_cast<TValue>(Serializable::getChar(val));
        }
        case native::GeodeTypeIds::CacheableDouble:
        {
          return safe_cast<TValue>(Serializable::getDouble(val));
        }
        case native::GeodeTypeIds::CacheableASCIIString:
        case native::GeodeTypeIds::CacheableASCIIStringHuge:
        case native::GeodeTypeIds::CacheableString:
        case native::GeodeTypeIds::CacheableStringHuge:
        {
          //TODO: need to look all strings types
          return safe_cast<TValue>(Serializable::getASCIIString(val));
        }
        case native::GeodeTypeIds::CacheableFloat:
        {
          return safe_cast<TValue>(Serializable::getFloat(val));
        }
        case native::GeodeTypeIds::CacheableInt16:
        {
          return safe_cast<TValue>(Serializable::getInt16(val));
        }
        case native::GeodeTypeIds::CacheableInt32:
        {
          return safe_cast<TValue>(Serializable::getInt32(val));
        }
        case native::GeodeTypeIds::CacheableInt64:
        {
          return safe_cast<TValue>(Serializable::getInt64(val));
        }
        case native::GeodeTypeIds::CacheableDate:
        {
          //TODO::
          Apache::Geode::Client::CacheableDate^ ret = static_cast<Apache::Geode::Client::CacheableDate ^>
            (SafeGenericUMSerializableConvert<Apache::Geode::Client::CacheableDate^>(val));

          System::DateTime dt(ret->Value.Ticks);
          return safe_cast<TValue>(dt);
        }
        case native::GeodeTypeIdsImpl::CacheableUserData:
        case native::GeodeTypeIdsImpl::CacheableUserData2:
        case native::GeodeTypeIdsImpl::CacheableUserData4:
        {
          //TODO::split 
          IGeodeSerializable^ ret = SafeUMSerializableConvertGeneric(val);
          return safe_cast<TValue>(ret);
          //return TValue();
        }
        case native::GeodeTypeIdsImpl::PDX:
        {
          IPdxSerializable^ ret = SafeUMSerializablePDXConvert(val);

          PdxWrapper^ pdxWrapper = dynamic_cast<PdxWrapper^>(ret);

          if (pdxWrapper != nullptr)
          {
            return safe_cast<TValue>(pdxWrapper->GetObject());
          }

          return safe_cast<TValue>(ret);
        }
        case native::GeodeTypeIds::CacheableBytes:
        {
          Apache::Geode::Client::CacheableBytes^ ret = safe_cast<Apache::Geode::Client::CacheableBytes ^>
            (SafeGenericUMSerializableConvert<Apache::Geode::Client::CacheableBytes^>(val));

          return safe_cast<TValue>(ret->Value);
        }
        case native::GeodeTypeIds::CacheableDoubleArray:
        {
          Apache::Geode::Client::CacheableDoubleArray^ ret = safe_cast<Apache::Geode::Client::CacheableDoubleArray ^>
            (SafeGenericUMSerializableConvert<Apache::Geode::Client::CacheableDoubleArray^>(val));

          return safe_cast<TValue>(ret->Value);
        }
        case native::GeodeTypeIds::CacheableFloatArray:
        {
          Apache::Geode::Client::CacheableFloatArray^ ret = safe_cast<Apache::Geode::Client::CacheableFloatArray^>
            (SafeGenericUMSerializableConvert<Apache::Geode::Client::CacheableFloatArray^>(val));

          return safe_cast<TValue>(ret->Value);
        }
        case native::GeodeTypeIds::CacheableInt16Array:
        {
          Apache::Geode::Client::CacheableInt16Array^ ret = safe_cast<Apache::Geode::Client::CacheableInt16Array^>
            (SafeGenericUMSerializableConvert<Apache::Geode::Client::CacheableInt16Array^>(val));

          return safe_cast<TValue>(ret->Value);
        }
        case native::GeodeTypeIds::CacheableInt32Array:
        {
          Apache::Geode::Client::CacheableInt32Array^ ret = safe_cast<Apache::Geode::Client::CacheableInt32Array^>
            (SafeGenericUMSerializableConvert<Apache::Geode::Client::CacheableInt32Array^>(val));

          return safe_cast<TValue>(ret->Value);
        }
        case native::GeodeTypeIds::CacheableInt64Array:
        {
          Apache::Geode::Client::CacheableInt64Array^ ret = safe_cast<Apache::Geode::Client::CacheableInt64Array^>
            (SafeGenericUMSerializableConvert<Apache::Geode::Client::CacheableInt64Array^>(val));

          return safe_cast<TValue>(ret->Value);
        }
        case native::GeodeTypeIds::CacheableStringArray:
        {
          Apache::Geode::Client::CacheableStringArray^ ret = safe_cast<Apache::Geode::Client::CacheableStringArray^>
            (SafeGenericUMSerializableConvert<Apache::Geode::Client::CacheableStringArray^>(val));

          return safe_cast<TValue>(ret->GetValues());
        }
        case native::GeodeTypeIds::CacheableArrayList://Ilist generic
        {
          Apache::Geode::Client::CacheableArrayList^ ret = safe_cast<Apache::Geode::Client::CacheableArrayList^>
            (SafeGenericUMSerializableConvert<Apache::Geode::Client::CacheableArrayList^>(val));

          return safe_cast<TValue>(ret->Value);
        }
        case native::GeodeTypeIds::CacheableLinkedList://LinkedList generic
        {
          Apache::Geode::Client::CacheableLinkedList^ ret = safe_cast<Apache::Geode::Client::CacheableLinkedList^>
            (SafeGenericUMSerializableConvert<Apache::Geode::Client::CacheableLinkedList^>(val));

          return safe_cast<TValue>(ret->Value);
        }
        case native::GeodeTypeIds::CacheableHashTable://collection::hashtable
        {
          Apache::Geode::Client::CacheableHashTable^ ret = safe_cast<Apache::Geode::Client::CacheableHashTable^>
            (SafeGenericUMSerializableConvert<Apache::Geode::Client::CacheableHashTable^>(val));

          return safe_cast<TValue>(ret->Value);
        }
        case native::GeodeTypeIds::CacheableHashMap://generic dictionary
        {
          Apache::Geode::Client::CacheableHashMap^ ret = safe_cast<Apache::Geode::Client::CacheableHashMap^>
            (SafeGenericUMSerializableConvert<Apache::Geode::Client::CacheableHashMap^>(val));

          return safe_cast<TValue>(ret->Value);
        }
        case native::GeodeTypeIds::CacheableIdentityHashMap:
        {
          Apache::Geode::Client::CacheableIdentityHashMap^ ret = static_cast<Apache::Geode::Client::CacheableIdentityHashMap^>
            (SafeGenericUMSerializableConvert<Apache::Geode::Client::CacheableIdentityHashMap^>(val));
          return safe_cast<TValue>(ret->Value);
        }
        case native::GeodeTypeIds::CacheableHashSet://no need of it, default case should work
        {
          Apache::Geode::Client::CacheableHashSet^ ret = static_cast<Apache::Geode::Client::CacheableHashSet^>
            (SafeGenericUMSerializableConvert<Apache::Geode::Client::CacheableHashSet^>(val));
          return safe_cast<TValue>(ret);
        }
        case native::GeodeTypeIds::CacheableLinkedHashSet://no need of it, default case should work
        {
          Apache::Geode::Client::CacheableLinkedHashSet^ ret = static_cast<Apache::Geode::Client::CacheableLinkedHashSet^>
            (SafeGenericUMSerializableConvert<Apache::Geode::Client::CacheableLinkedHashSet^>(val));
          return safe_cast<TValue>(ret);
        }
        case native::GeodeTypeIds::CacheableFileName:
        {
          Apache::Geode::Client::CacheableFileName^ ret = static_cast<Apache::Geode::Client::CacheableFileName^>
            (SafeGenericUMSerializableConvert<Apache::Geode::Client::CacheableFileName^>(val));
          return safe_cast<TValue>(ret);
        }
        case native::GeodeTypeIds::CacheableObjectArray:
        {
          Apache::Geode::Client::CacheableObjectArray^ ret = static_cast<Apache::Geode::Client::CacheableObjectArray^>
            (SafeGenericUMSerializableConvert<Apache::Geode::Client::CacheableObjectArray^>(val));
          return safe_cast<TValue>(ret);
        }
        case native::GeodeTypeIds::CacheableVector://collection::arraylist
        {
          Apache::Geode::Client::CacheableVector^ ret = static_cast<Apache::Geode::Client::CacheableVector^>
            (SafeGenericUMSerializableConvert<Apache::Geode::Client::CacheableVector^>(val));
          return safe_cast<TValue>(ret->Value);
        }
        case native::GeodeTypeIds::CacheableUndefined:
        {
          Apache::Geode::Client::CacheableUndefined^ ret = static_cast<Apache::Geode::Client::CacheableUndefined^>
            (SafeGenericUMSerializableConvert<Apache::Geode::Client::CacheableUndefined^>(val));
          return safe_cast<TValue>(ret);
        }
        case native::GeodeTypeIds::Struct:
        {
          return safe_cast<TValue>(Apache::Geode::Client::Struct::Create(val));
        }
        case native::GeodeTypeIds::CacheableStack:
        {
          Apache::Geode::Client::CacheableStack^ ret = static_cast<Apache::Geode::Client::CacheableStack^>
            (SafeGenericUMSerializableConvert<Apache::Geode::Client::CacheableStack^>(val));
          return safe_cast<TValue>(ret->Value);
        }
        case 7: //GeodeClassIds::CacheableManagedObject
        {
          Apache::Geode::Client::CacheableObject^ ret = static_cast<Apache::Geode::Client::CacheableObject^>
            (SafeGenericUMSerializableConvert<Apache::Geode::Client::CacheableObject^>(val));
          return safe_cast<TValue>(ret);
        }
        case 8://GeodeClassIds::CacheableManagedObjectXml
        {
          Apache::Geode::Client::CacheableObjectXml^ ret = static_cast<Apache::Geode::Client::CacheableObjectXml^>
            (SafeGenericUMSerializableConvert<Apache::Geode::Client::CacheableObjectXml^>(val));
          return safe_cast<TValue>(ret);
        }
        */
        /*  TODO: replace with IDictionary<K, V>
        case native::GeodeTypeIds::Properties:
        {
        Apache::Geode::Client::Properties^ ret = safe_cast<Apache::Geode::Client::Properties^>
        ( SafeGenericUMSerializableConvert<Apache::Geode::Client::Properties^>(val));

        return safe_cast<TValue>(ret);
        }*/

        /* TODO serializable
        case native::GeodeTypeIds::BooleanArray:
        {
          Apache::Geode::Client::BooleanArray^ ret = safe_cast<Apache::Geode::Client::BooleanArray^>
            (SafeGenericUMSerializableConvert<Apache::Geode::Client::BooleanArray^>(val));

          return safe_cast<TValue>(ret->Value);
        }
        case native::GeodeTypeIds::CharArray:
        {
          Apache::Geode::Client::CharArray^ ret = safe_cast<Apache::Geode::Client::CharArray^>
            (SafeGenericUMSerializableConvert<Apache::Geode::Client::CharArray^>(val));

          return safe_cast<TValue>(ret->Value);
        }
        case 0://UserFunctionExecutionException unregistered
        {
          Apache::Geode::Client::UserFunctionExecutionException^ ret = static_cast<Apache::Geode::Client::UserFunctionExecutionException^>
            (SafeGenericUMSerializableConvert<Apache::Geode::Client::UserFunctionExecutionException^>(val));
          return safe_cast<TValue>(ret);
        }
        default:
          throw gcnew System::Exception("not found typeid");
        }

        */
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
