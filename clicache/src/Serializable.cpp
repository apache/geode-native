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

#include <msclr/lock.h>

#include "Serializable.hpp"
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

#pragma warning(disable:4091)

using namespace System::Reflection;
using namespace System::Reflection::Emit;

using namespace System;
using namespace System::Collections::Generic;
using namespace Runtime::InteropServices;
using namespace apache::geode::client;

namespace Apache
{
  namespace Geode
  {
    namespace Client
    {
      namespace native = apache::geode::client;

      void Apache::Geode::Client::Serializable::ToData(
        Apache::Geode::Client::DataOutput^ output)
      {
        if (output->IsManagedObject()) {
          output->WriteBytesToUMDataOutput();
        }
        try
        {
          auto nativeOutput = output->GetNative();
          m_nativeptr->get()->toData(*nativeOutput);
        }
        finally
        {
          GC::KeepAlive(output);
          GC::KeepAlive(m_nativeptr);
        }
        if (output->IsManagedObject()) {
          output->SetBuffer();
        }
      }

      void Serializable::FromData(DataInput^ input)
      {
        if (input->IsManagedObject()) {
          input->AdvanceUMCursor();
        }
        auto* nativeInput = input->GetNative();

        try
        {
          m_nativeptr->get()->fromData(*nativeInput);
          if (input->IsManagedObject()) {
            input->SetBuffer();
          }
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }
      }

      System::UInt32 Apache::Geode::Client::Serializable::ObjectSize::get()
      {
        try
        {
          return m_nativeptr->get()->objectSize();
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }
      }

      System::UInt32 Apache::Geode::Client::Serializable::ClassId::get()
      {
        try
        {
          auto n = m_nativeptr->get();
          int8_t typeId = n->typeId();
          if (typeId == native::GeodeTypeIdsImpl::CacheableUserData ||
            typeId == native::GeodeTypeIdsImpl::CacheableUserData2 ||
            typeId == native::GeodeTypeIdsImpl::CacheableUserData4) {
            return n->classId();
          }
          else {
            return typeId + 0x80000000 + (0x20000000 * n->DSFID());
          }
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }
      }

      String^ Apache::Geode::Client::Serializable::ToString()
      {
        try
        {
          auto cStr = m_nativeptr->get()->toString();
          if (cStr->isWideString()) {
            return ManagedString::Get(cStr->asWChar());
          }
          else {
            return ManagedString::Get(cStr->asChar());
          }
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }
      }

      Apache::Geode::Client::Serializable::operator Apache::Geode::Client::Serializable ^ (Byte value)
      {
        return (Apache::Geode::Client::Serializable^) CacheableByte::Create(value);
      }

      Apache::Geode::Client::Serializable::operator Apache::Geode::Client::Serializable ^ (bool value)
      {
        return (Apache::Geode::Client::Serializable^)CacheableBoolean::Create(value);
      }

      Apache::Geode::Client::Serializable::operator Apache::Geode::Client::Serializable ^ (array<bool>^ value)
      {
        // return (Apache::Geode::Client::Serializable^)Apache::Geode::Client::CacheableBooleanArray::Create(value);
        //TODO:split
        return nullptr;
      }

      Apache::Geode::Client::Serializable::operator Apache::Geode::Client::Serializable ^ (array<Byte>^ value)
      {
        return (Apache::Geode::Client::Serializable^)CacheableBytes::Create(value);
      }

      Apache::Geode::Client::Serializable::operator Apache::Geode::Client::Serializable ^ (Char value)
      {
        return (Apache::Geode::Client::Serializable^)CacheableCharacter::Create(value);
      }

      Apache::Geode::Client::Serializable::operator Apache::Geode::Client::Serializable ^ (array<Char>^ value)
      {
        //return (Apache::Geode::Client::Serializable^)Apache::Geode::Client::CacheableCharArray::Create(value);
        //TODO:split
        return nullptr;

      }

      Apache::Geode::Client::Serializable::operator Apache::Geode::Client::Serializable ^ (Double value)
      {
        return (Apache::Geode::Client::Serializable^)CacheableDouble::Create(value);
      }

      Apache::Geode::Client::Serializable::operator Apache::Geode::Client::Serializable ^ (array<Double>^ value)
      {
        return (Apache::Geode::Client::Serializable^)CacheableDoubleArray::Create(value);
      }

      Apache::Geode::Client::Serializable::operator Apache::Geode::Client::Serializable ^ (Single value)
      {
        return (Apache::Geode::Client::Serializable^)CacheableFloat::Create(value);
      }

      Apache::Geode::Client::Serializable::operator Apache::Geode::Client::Serializable ^ (array<Single>^ value)
      {
        return (Apache::Geode::Client::Serializable^)CacheableFloatArray::Create(value);
      }

      Apache::Geode::Client::Serializable::operator Apache::Geode::Client::Serializable ^ (System::Int16 value)
      {
        return (Apache::Geode::Client::Serializable^)CacheableInt16::Create(value);
      }

      Apache::Geode::Client::Serializable::operator Apache::Geode::Client::Serializable ^ (array<System::Int16>^ value)
      {
        return (Apache::Geode::Client::Serializable^)CacheableInt16Array::Create(value);
      }

      Apache::Geode::Client::Serializable::operator Apache::Geode::Client::Serializable ^ (System::Int32 value)
      {
        return (Apache::Geode::Client::Serializable^)CacheableInt32::Create(value);
      }

      Apache::Geode::Client::Serializable::operator Apache::Geode::Client::Serializable ^ (array<System::Int32>^ value)
      {
        return (Apache::Geode::Client::Serializable^)CacheableInt32Array::Create(value);
      }

      Apache::Geode::Client::Serializable::operator Apache::Geode::Client::Serializable ^ (System::Int64 value)
      {
        return (Apache::Geode::Client::Serializable^)CacheableInt64::Create(value);
      }

      /*Apache::Geode::Client::*/Serializable::operator /*Apache::Geode::Client::*/Serializable ^ (array<System::Int64>^ value)
      {
        return (Apache::Geode::Client::Serializable^)Apache::Geode::Client::CacheableInt64Array::Create(value);
      }

      Apache::Geode::Client::Serializable::operator Apache::Geode::Client::Serializable ^ (String^ value)
      {
        return (Apache::Geode::Client::Serializable^)CacheableString::Create(value);
      }

      Apache::Geode::Client::Serializable::operator Apache::Geode::Client::Serializable ^ (array<String^>^ value)
      {
        return (Apache::Geode::Client::Serializable^)CacheableStringArray::Create(value);
      }

      System::Int32 Serializable::GetPDXIdForType(const char* poolName, IGeodeSerializable^ pdxType, Cache^ cache)
      {
        native::CacheablePtr kPtr(SafeMSerializableConvertGeneric(pdxType));
        return CacheRegionHelper::getCacheImpl(cache->GetNative().get())->getSerializationRegistry()->GetPDXIdForType(cache->GetNative()->getPoolManager().find(poolName), kPtr);
      }

      IGeodeSerializable^ Serializable::GetPDXTypeById(const char* poolName, System::Int32 typeId, Cache^ cache)
      {        
        SerializablePtr sPtr = CacheRegionHelper::getCacheImpl(cache->GetNative().get())->getSerializationRegistry()->GetPDXTypeById(cache->GetNative()->getPoolManager().find(poolName), typeId);
        return SafeUMSerializableConvertGeneric(sPtr);
      }

      int Serializable::GetEnumValue(Internal::EnumInfo^ ei, Cache^ cache)
      {
        native::CacheablePtr kPtr(SafeMSerializableConvertGeneric(ei));
        return  CacheRegionHelper::getCacheImpl(cache->GetNative().get())->getSerializationRegistry()->GetEnumValue(cache->GetNative()->getPoolManager().getAll().begin()->second, kPtr);
      }

      Internal::EnumInfo^ Serializable::GetEnum(int val, Cache^ cache)
      {
        SerializablePtr sPtr = CacheRegionHelper::getCacheImpl(cache->GetNative().get())->getSerializationRegistry()->GetEnum(cache->GetNative()->getPoolManager().getAll().begin()->second, val);
        return (Internal::EnumInfo^)SafeUMSerializableConvertGeneric(sPtr);
      }

      void Serializable::RegisterPdxType(PdxTypeFactoryMethod^ creationMethod)
      {
        if (creationMethod == nullptr) {
          throw gcnew IllegalArgumentException("Serializable.RegisterPdxType(): "
                                               "null PdxTypeFactoryMethod delegate passed");
        }
        IPdxSerializable^ obj = creationMethod();
        PdxDelegateMap[obj->GetType()->FullName] = creationMethod;
        Log::Debug("RegisterPdxType: class registered: " + obj->GetType()->FullName);
      }

      Object^ Serializable::CreateObject(String^ className)
      {

        Object^ retVal = CreateObjectEx(className);

        if (retVal == nullptr)
        {
          Type^ t = GetType(className);
          if (t)
          {
            retVal = t->GetConstructor(Type::EmptyTypes)->Invoke(nullptr);
            return retVal;
          }
        }
        return retVal;
      }

      Object^ Serializable::CreateObjectEx(String^ className)
      {
        CreateNewObjectDelegate^ del = nullptr;
        Dictionary<String^, CreateNewObjectDelegate^>^ tmp = ClassNameVsCreateNewObjectDelegate;

        tmp->TryGetValue(className, del);

        if (del != nullptr)
        {
          return del();
        }

        Type^ t = GetType(className);
        if (t)
        {
          msclr::lock lockInstance(ClassNameVsTypeLockObj);
          {
            tmp = ClassNameVsCreateNewObjectDelegate;
            tmp->TryGetValue(className, del);
            if (del != nullptr)
              return del();
            del = CreateNewObjectDelegateF(t);
            tmp = gcnew Dictionary<String^, CreateNewObjectDelegate^>(ClassNameVsCreateNewObjectDelegate);
            tmp[className] = del;
            ClassNameVsCreateNewObjectDelegate = tmp;
            return del();
          }
        }
        return nullptr;
      }

      Object^ Serializable::GetArrayObject(String^ className, int len)
      {
        Object^ retArr = GetArrayObjectEx(className, len);
        if (retArr == nullptr)
        {
          Type^ t = GetType(className);
          if (t)
          {
            retArr = t->MakeArrayType()->GetConstructor(singleIntType)->Invoke(gcnew array<Object^>(1) { len });
            return retArr;
          }
        }
        return retArr;
      }

      Object^ Serializable::GetArrayObjectEx(String^ className, int len)
      {
        CreateNewObjectArrayDelegate^ del = nullptr;
        Dictionary<String^, CreateNewObjectArrayDelegate^>^ tmp = ClassNameVsCreateNewObjectArrayDelegate;

        tmp->TryGetValue(className, del);

        if (del != nullptr)
        {
          return del(len);
        }

        Type^ t = GetType(className);
        if (t)
        {
          msclr::lock lockInstance(ClassNameVsTypeLockObj);
          {
            tmp = ClassNameVsCreateNewObjectArrayDelegate;
            tmp->TryGetValue(className, del);
            if (del != nullptr)
              return del(len);
            del = CreateNewObjectArrayDelegateF(t);
            tmp = gcnew Dictionary<String^, CreateNewObjectArrayDelegate^>(ClassNameVsCreateNewObjectArrayDelegate);
            tmp[className] = del;
            ClassNameVsCreateNewObjectArrayDelegate = tmp;
            return del(len);
          }
        }
        return nullptr;
      }

      //delegate Object^ CreateNewObject();
      //static CreateNewObjectDelegate^ CreateNewObjectDelegateF(Type^ type);
      Serializable::CreateNewObjectDelegate^ Serializable::CreateNewObjectDelegateF(Type^ type)
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

        return (Serializable::CreateNewObjectDelegate^)dynam->CreateDelegate(createNewObjectDelegateType);
      }

      //delegate Object^ CreateNewObjectArray(int len);
      Serializable::CreateNewObjectArrayDelegate^ Serializable::CreateNewObjectArrayDelegateF(Type^ type)
      {
        DynamicMethod^ dynam = gcnew DynamicMethod("", Internal::DotNetTypes::ObjectType, singleIntTypeA, type, true);
        ILGenerator^ il = dynam->GetILGenerator();

        il->Emit(OpCodes::Ldarg_0);

        il->Emit(OpCodes::Newarr, type);
        il->Emit(OpCodes::Ret);

        return (Serializable::CreateNewObjectArrayDelegate^)dynam->CreateDelegate(createNewObjectArrayDelegateType);
      }

      Type^ Serializable::getTypeFromRefrencedAssemblies(String^ className, Dictionary<Assembly^, bool>^ referedAssembly, Assembly^ currentAsm)
      {
        Type^ t = currentAsm->GetType(className);
        if (t != nullptr)
        {
          Dictionary<String^, Type^>^ tmp = gcnew Dictionary<String^, Type^>(ClassNameVsType);
          tmp[className] = t;
          ClassNameVsType = tmp;
          return t;
        }
        //already touched
        if (referedAssembly->ContainsKey(currentAsm))
          return nullptr;
        referedAssembly[currentAsm] = true;

        //get all refrenced assembly
        array<AssemblyName^>^ ReferencedAssemblies = currentAsm->GetReferencedAssemblies();
        for each(AssemblyName^ tmpAsm in ReferencedAssemblies)
        {
          try
          {
            Assembly^ la = Assembly::Load(tmpAsm);
            if (la != nullptr && (!referedAssembly->ContainsKey(la)))
            {
              t = getTypeFromRefrencedAssemblies(className, referedAssembly, la);
              if (!t)
                return t;
            }
          }
          catch (System::Exception^){//ignore
          }
        }
        return nullptr;
      }

      Type^ Serializable::GetType(String^ className)
      {
        Type^ retVal = nullptr;
        Dictionary<String^, Type^>^ tmp = ClassNameVsType;
        tmp->TryGetValue(className, retVal);

        if (retVal != nullptr)
          return retVal;
        msclr::lock lockInstance(ClassNameVsTypeLockObj);
        {
          tmp = ClassNameVsType;
          tmp->TryGetValue(className, retVal);

          if (retVal != nullptr)
            return retVal;

          Dictionary<Assembly^, bool>^ referedAssembly = gcnew Dictionary<Assembly^, bool>();
          AppDomain^ MyDomain = AppDomain::CurrentDomain;
          array<Assembly^>^ AssembliesLoaded = MyDomain->GetAssemblies();
          for each(Assembly^ tmpAsm in AssembliesLoaded)
          {
            retVal = getTypeFromRefrencedAssemblies(className, referedAssembly, tmpAsm);
            if (retVal)
              return retVal;
          }
        }
        return retVal;
      }

      IPdxSerializable^ Serializable::GetPdxType(String^ className)
      {
        PdxTypeFactoryMethod^ retVal = nullptr;
        PdxDelegateMap->TryGetValue(className, retVal);

        if (retVal == nullptr){

          if (PdxSerializer != nullptr)
          {
            return gcnew PdxWrapper(className);
          }
          try
          {
            Object^ retObj = CreateObject(className);

            IPdxSerializable^ retPdx = dynamic_cast<IPdxSerializable^>(retObj);
            if (retPdx != nullptr)
              return retPdx;
          }
          catch (System::Exception^ ex)
          {
            Log::Error("Unable to create object usqing reflection for class: " + className + " : " + ex->Message);
          }
          throw gcnew IllegalStateException("Pdx factory method (or PdxSerializer ) not registered (or don't have zero arg constructor)"
                                            " to create default instance for class: " + className);
        }

        return retVal();
      }

      void Serializable::RegisterPDXManagedCacheableKey(Cache^ cache)
      {
        auto cacheImpl = CacheRegionHelper::getCacheImpl(cache->GetNative().get());
        cacheImpl->getSerializationRegistry()->setPdxTypeHandler([](native::DataInput& dataInput){
          auto obj = std::make_shared<native::PdxManagedCacheableKey>();
          obj->fromData(dataInput);
          return obj;
        });
      }

      void Apache::Geode::Client::Serializable::RegisterTypeGeneric(TypeFactoryMethodGeneric^ creationMethod, Cache^ cache)
      {
        if (creationMethod == nullptr) {
          throw gcnew IllegalArgumentException("Serializable.RegisterType(): "
                                               "null TypeFactoryMethod delegate passed");
        }

        //--------------------------------------------------------------

        //adding user type as well in global builtin hashmap
        System::Int64 classId = ((System::Int64)creationMethod()->ClassId);
        if (!ManagedDelegatesGeneric->ContainsKey(classId))
          ManagedDelegatesGeneric->Add(classId, creationMethod);

        DelegateWrapperGeneric^ delegateObj = gcnew DelegateWrapperGeneric(creationMethod);
        TypeFactoryNativeMethodGeneric^ nativeDelegate =
          gcnew TypeFactoryNativeMethodGeneric(delegateObj,
          &DelegateWrapperGeneric::NativeDelegateGeneric);

        // this is avoid object being Gced
        NativeDelegatesGeneric->Add(nativeDelegate);

        // register the type in the DelegateMap, this is pure c# for create domain object 
        IGeodeSerializable^ tmp = creationMethod();
        Log::Fine("Registering serializable class ID " + tmp->ClassId +
                  ", AppDomain ID " + System::Threading::Thread::GetDomainID());
        DelegateMapGeneric[tmp->ClassId] = creationMethod;

        _GF_MG_EXCEPTION_TRY2
          CacheImpl *cacheImpl = CacheRegionHelper::getCacheImpl(cache->GetNative().get());
          cacheImpl->getSerializationRegistry()->addType((native::Serializable*(*)())System::Runtime::InteropServices::Marshal::GetFunctionPointerForDelegate(nativeDelegate).ToPointer());

        _GF_MG_EXCEPTION_CATCH_ALL2
      }

      void Apache::Geode::Client::Serializable::RegisterTypeGeneric(Byte typeId,
                                                                    TypeFactoryMethodGeneric^ creationMethod, Type^ type, Cache^ cache)
      {
        if (creationMethod == nullptr) {
          throw gcnew IllegalArgumentException("Serializable.RegisterType(): "
                                               "null TypeFactoryMethod delegate passed");
        }
        DelegateWrapperGeneric^ delegateObj = gcnew DelegateWrapperGeneric(creationMethod);
        TypeFactoryNativeMethodGeneric^ nativeDelegate =
          gcnew TypeFactoryNativeMethodGeneric(delegateObj,
          &DelegateWrapperGeneric::NativeDelegateGeneric);

        BuiltInDelegatesGeneric[typeId] = nativeDelegate;

        if (type != nullptr)
          ManagedTypeMappingGeneric[type] = typeId;

        //This is hashmap for manged builtin objects
        if (!ManagedDelegatesGeneric->ContainsKey(typeId + 0x80000000))
          ManagedDelegatesGeneric->Add(typeId + 0x80000000, creationMethod);

        // register the type in the DelegateMap
        IGeodeSerializable^ tmp = creationMethod();
        Log::Finer("Registering(,) serializable class ID " + tmp->ClassId +
                   ", AppDomain ID " + System::Threading::Thread::GetDomainID());
        DelegateMapGeneric[tmp->ClassId] = creationMethod;

        try
        {
           CacheImpl *cacheImpl = CacheRegionHelper::getCacheImpl(cache->GetNative().get());
          if (tmp->ClassId < 0xa0000000)
          {
            cacheImpl->getSerializationRegistry()->addType(typeId,
                                                                  (native::Serializable*(*)())System::Runtime::InteropServices::
                                                                  Marshal::GetFunctionPointerForDelegate(
                                                                  nativeDelegate).ToPointer());
          }
          else
          {//special case for CacheableUndefined type
            cacheImpl->getSerializationRegistry()->addType2(typeId,
                                                                   (native::Serializable*(*)())System::Runtime::InteropServices::
                                                                   Marshal::GetFunctionPointerForDelegate(
                                                                   nativeDelegate).ToPointer());
          }

        }
        catch (native::IllegalStateException&)
        {
          //ignore it as this is internal only
        }
      }

      void Apache::Geode::Client::Serializable::UnregisterTypeGeneric(Byte typeId, Cache^ cache)
      {
        BuiltInDelegatesGeneric->Remove(typeId);
        _GF_MG_EXCEPTION_TRY2

          CacheRegionHelper::getCacheImpl(cache->GetNative().get())->getSerializationRegistry()->removeType(typeId);

        _GF_MG_EXCEPTION_CATCH_ALL2
      }

      void Apache::Geode::Client::Serializable::RegisterWrapperGeneric(
        WrapperDelegateGeneric^ wrapperMethod, Byte typeId, System::Type^ type)
      {
        if (typeId < 0 || typeId > WrapperEndGeneric)
        {
          throw gcnew GeodeException("The TypeID (" + typeId + ") being "
                                       "registered is beyond acceptable range of 0-" + WrapperEndGeneric);
        }
        NativeWrappersGeneric[typeId] = wrapperMethod;
        ManagedTypeMappingGeneric[type] = typeId;
      }

      void Apache::Geode::Client::Serializable::UnregisterNativesGeneric()
      {
        BuiltInDelegatesGeneric->Clear();
        for (Byte typeId = 0; typeId <= WrapperEndGeneric; ++typeId) {
          NativeWrappersGeneric[typeId] = nullptr;
        }
        //TODO:: unregister from managed hashmap as well.
        //  ManagedDelegates->Clear();
      }

      generic<class TValue>
      TValue Serializable::GetManagedValueGeneric(native::SerializablePtr val)
      {
        if (val == nullptr)
        {
          return TValue();
        }

        Byte typeId = val->typeId();
        //Log::Debug("Serializable::GetManagedValueGeneric typeid = " + typeId);
        switch (typeId)
        {
        case native::GeodeTypeIds::CacheableByte:
        {
          return (TValue)(int8_t)safe_cast<int8_t>(Serializable::getByte(val));
          /* if (TValue::typeid == System::SByte::typeid) {
              return (TValue)(int8_t)safe_cast<int8_t>(Serializable::getByte(val));
              }
              else {
              return (TValue)(System::Byte)safe_cast<int8_t>(Serializable::getByte(val));
              }
              return safe_cast<TValue>(Serializable::getByte(val));*/
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
          /* if (TValue::typeid == System::Int16::typeid) {
              return (TValue)(System::Int16)safe_cast<System::Int16>(Serializable::getInt16(val));
              }
              else {
              return (TValue)(System::UInt16)safe_cast<System::Int16>(Serializable::getInt16(val));
              }*/
          return safe_cast<TValue>(Serializable::getInt16(val));
        }
        case native::GeodeTypeIds::CacheableInt32:
        {
          /* if (TValue::typeid == System::Int32::typeid) {
              return (TValue)(System::Int32)safe_cast<System::Int32>(Serializable::getInt32(val));
              }
              else {
              return (TValue)(System::UInt32)safe_cast<System::Int32>(Serializable::getInt32(val));
              }  */
          return safe_cast<TValue>(Serializable::getInt32(val));
        }
        case native::GeodeTypeIds::CacheableInt64:
        {
          /*if (TValue::typeid == System::Int64::typeid) {
            return (TValue)(System::Int64)safe_cast<System::Int64>(Serializable::getInt64(val));
            }
            else {
            return (TValue)(System::UInt64)safe_cast<System::Int64>(Serializable::getInt64(val));
            }*/
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

          /* array<String^>^ str = gcnew array<String^>(ret->GetValues()->Length);
            for(int i=0; i<ret->GetValues()->Length; i++ ) {
            str[i] = ret->GetValues()[i];
            }*/

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
        /*  TODO: replace with IDictionary<K, V>
      case native::GeodeTypeIds::Properties:
      {
      Apache::Geode::Client::Properties^ ret = safe_cast<Apache::Geode::Client::Properties^>
      ( SafeGenericUMSerializableConvert<Apache::Geode::Client::Properties^>(val));

      return safe_cast<TValue>(ret);
      }*/

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
        throw gcnew System::Exception("not found typeid");
      }

      generic<class TKey>
      native::CacheableKeyPtr Serializable::GetUnmanagedValueGeneric(TKey key)
      {
        if (key != nullptr) {
          return GetUnmanagedValueGeneric(key->GetType(), key);
        }
        return nullptr;
      }

      generic<class TKey>
      native::CacheableKeyPtr Serializable::GetUnmanagedValueGeneric(TKey key, bool isAciiChar)
      {
        if (key != nullptr) {
          return GetUnmanagedValueGeneric(key->GetType(), key, isAciiChar);
        }
        return nullptr;
      }

      void Serializable::RegisterPdxSerializer(IPdxSerializer^ pdxSerializer)
      {
        /*if(PdxSerializer != nullptr )
        {
        throw gcnew IllegalStateException("IPdxSerializer is already registered: " + PdxSerializer->GetType());
        }*/
        PdxSerializer = pdxSerializer;
      }

      void Serializable::SetPdxTypeMapper(IPdxTypeMapper^ pdxTypeMapper)
      {
        if (pdxTypeMapper != nullptr)
          PdxTypeMapper = pdxTypeMapper;
      }

      String^ Serializable::GetPdxTypeName(String^ localTypeName)
      {
        if (PdxTypeMapper == nullptr)
          return localTypeName;
        IDictionary<String^, String^>^ tmp = LocalTypeNameToPdx;
        String^ pdxTypeName = nullptr;
        tmp->TryGetValue(localTypeName, pdxTypeName);

        if (pdxTypeName != nullptr)
          return pdxTypeName;

        {
          msclr::lock lockInstance(LockObj);
          tmp->TryGetValue(localTypeName, pdxTypeName);

          if (pdxTypeName != nullptr)
            return pdxTypeName;
          if (PdxTypeMapper != nullptr)
          {
            pdxTypeName = PdxTypeMapper->ToPdxTypeName(localTypeName);
            if (pdxTypeName == nullptr)
            {
              throw gcnew IllegalStateException("PdxTypeName should not be null for local type " + localTypeName);
            }

            Dictionary<String^, String^>^ localToPdx = gcnew Dictionary<String^, String^>(LocalTypeNameToPdx);
            localToPdx[localTypeName] = pdxTypeName;
            LocalTypeNameToPdx = localToPdx;
            Dictionary<String^, String^>^ pdxToLocal = gcnew Dictionary<String^, String^>(PdxTypeNameToLocal);
            localToPdx[pdxTypeName] = localTypeName;
            PdxTypeNameToLocal = pdxToLocal;
          }
        }
        return pdxTypeName;
      }

      String^ Serializable::GetLocalTypeName(String^ pdxTypeName)
      {
        if (PdxTypeMapper == nullptr)
          return pdxTypeName;

        IDictionary<String^, String^>^ tmp = PdxTypeNameToLocal;
        String^ localTypeName = nullptr;
        tmp->TryGetValue(pdxTypeName, localTypeName);

        if (localTypeName != nullptr)
          return localTypeName;

        {
          msclr::lock lockInstance(LockObj);
          tmp->TryGetValue(pdxTypeName, localTypeName);

          if (localTypeName != nullptr)
            return localTypeName;
          if (PdxTypeMapper != nullptr)
          {
            localTypeName = PdxTypeMapper->FromPdxTypeName(pdxTypeName);
            if (localTypeName == nullptr)
            {
              throw gcnew IllegalStateException("LocalTypeName should not be null for pdx type " + pdxTypeName);
            }

            Dictionary<String^, String^>^ localToPdx = gcnew Dictionary<String^, String^>(LocalTypeNameToPdx);
            localToPdx[localTypeName] = pdxTypeName;
            LocalTypeNameToPdx = localToPdx;
            Dictionary<String^, String^>^ pdxToLocal = gcnew Dictionary<String^, String^>(PdxTypeNameToLocal);
            localToPdx[pdxTypeName] = localTypeName;
            PdxTypeNameToLocal = pdxToLocal;
          }
        }
        return localTypeName;
      }

      void Serializable::Clear()
      {
        PdxTypeMapper = nullptr;
        LocalTypeNameToPdx->Clear();
        PdxTypeNameToLocal->Clear();
        ClassNameVsCreateNewObjectDelegate->Clear();
        ClassNameVsType->Clear();
        ClassNameVsCreateNewObjectArrayDelegate->Clear();
      }

      IPdxSerializer^ Serializable::GetPdxSerializer()
      {
        return PdxSerializer;
      }

      bool Serializable::IsObjectAndPdxSerializerRegistered(String^ className)
      {
        return PdxSerializer != nullptr;
      }

      generic<class TKey>
      native::CacheableKeyPtr Serializable::GetUnmanagedValueGeneric(
        Type^ managedType, TKey key)
      {
        return GetUnmanagedValueGeneric(managedType, key, false);
      }

      generic<class TKey>
      native::CacheableKeyPtr Serializable::GetUnmanagedValueGeneric(
        Type^ managedType, TKey key, bool isAsciiChar)
      {
        Byte typeId = Apache::Geode::Client::Serializable::GetManagedTypeMappingGeneric(managedType);

        switch (typeId)
        {
        case native::GeodeTypeIds::CacheableByte: {
          return Serializable::getCacheableByte((SByte)key);
        }
        case native::GeodeTypeIds::CacheableBoolean:
          return Serializable::getCacheableBoolean((bool)key);
        case native::GeodeTypeIds::CacheableCharacter:
          return Serializable::getCacheableWideChar((Char)key);
        case native::GeodeTypeIds::CacheableDouble:
          return Serializable::getCacheableDouble((double)key);
        case native::GeodeTypeIds::CacheableASCIIString: {
          if (isAsciiChar)
            return Serializable::getCacheableASCIIString2((String^)key);
          else
            return Serializable::getCacheableASCIIString((String^)key);
        }
        case native::GeodeTypeIds::CacheableFloat:
          return Serializable::getCacheableFloat((float)key);
        case native::GeodeTypeIds::CacheableInt16: {
          return Serializable::getCacheableInt16((System::Int16)key);
        }
        case native::GeodeTypeIds::CacheableInt32: {
          return Serializable::getCacheableInt32((System::Int32)key);
        }
        case native::GeodeTypeIds::CacheableInt64: {
          return Serializable::getCacheableInt64((System::Int64)key);
        }
        case native::GeodeTypeIds::CacheableBytes:
        {
          native::CacheablePtr kPtr(SafeGenericMSerializableConvert((IGeodeSerializable^)Apache::Geode::Client::CacheableBytes::Create((array<Byte>^)key)));
          return std::dynamic_pointer_cast<native::CacheableKey>(kPtr);
        }
        case native::GeodeTypeIds::CacheableDoubleArray:
        {
          native::CacheablePtr kPtr(SafeGenericMSerializableConvert((IGeodeSerializable^)Apache::Geode::Client::CacheableDoubleArray::Create((array<Double>^)key)));
          return std::dynamic_pointer_cast<native::CacheableKey>(kPtr);
        }
        case native::GeodeTypeIds::CacheableFloatArray:
        {
          native::CacheablePtr kPtr(SafeGenericMSerializableConvert((IGeodeSerializable^)Apache::Geode::Client::CacheableFloatArray::Create((array<float>^)key)));
          return std::dynamic_pointer_cast<native::CacheableKey>(kPtr);
        }
        case native::GeodeTypeIds::CacheableInt16Array:
        {
          native::CacheablePtr kPtr(SafeGenericMSerializableConvert((IGeodeSerializable^)Apache::Geode::Client::CacheableInt16Array::Create((array<Int16>^)key)));
          return std::dynamic_pointer_cast<native::CacheableKey>(kPtr);
        }
        case native::GeodeTypeIds::CacheableInt32Array:
        {
          native::CacheablePtr kPtr(SafeGenericMSerializableConvert((IGeodeSerializable^)Apache::Geode::Client::CacheableInt32Array::Create((array<Int32>^)key)));
          return std::dynamic_pointer_cast<native::CacheableKey>(kPtr);
        }
        case native::GeodeTypeIds::CacheableInt64Array:
        {
          native::CacheablePtr kPtr(SafeGenericMSerializableConvert((IGeodeSerializable^)Apache::Geode::Client::CacheableInt64Array::Create((array<Int64>^)key)));
          return std::dynamic_pointer_cast<native::CacheableKey>(kPtr);
        }
        case native::GeodeTypeIds::CacheableStringArray:
        {
          native::CacheablePtr kPtr(SafeGenericMSerializableConvert((IGeodeSerializable^)Apache::Geode::Client::CacheableStringArray::Create((array<String^>^)key)));
          return std::dynamic_pointer_cast<native::CacheableKey>(kPtr);
        }
        case native::GeodeTypeIds::CacheableFileName:
        {
          native::CacheablePtr kPtr(SafeGenericMSerializableConvert((IGeodeSerializable^)(Apache::Geode::Client::CacheableFileName^)key));
          return std::dynamic_pointer_cast<native::CacheableKey>(kPtr);
        }
        case native::GeodeTypeIds::CacheableHashTable://collection::hashtable
        {
          native::CacheablePtr kPtr(SafeGenericMSerializableConvert((IGeodeSerializable^)Apache::Geode::Client::CacheableHashTable::Create((System::Collections::Hashtable^)key)));
          return std::dynamic_pointer_cast<native::CacheableKey>(kPtr);
        }
        case native::GeodeTypeIds::CacheableHashMap://generic dictionary
        {
          native::CacheablePtr kPtr(SafeGenericMSerializableConvert((IGeodeSerializable^)Apache::Geode::Client::CacheableHashMap::Create((System::Collections::IDictionary^)key)));
          return std::dynamic_pointer_cast<native::CacheableKey>(kPtr);
        }
        case native::GeodeTypeIds::CacheableVector://collection::arraylist
        {
          native::CacheablePtr kPtr(SafeGenericMSerializableConvert((IGeodeSerializable^)CacheableVector::Create((System::Collections::IList^)key)));
          return std::dynamic_pointer_cast<native::CacheableKey>(kPtr);
        }
        case native::GeodeTypeIds::CacheableArrayList://generic ilist
        {
          native::CacheablePtr kPtr(SafeGenericMSerializableConvert((IGeodeSerializable^)Apache::Geode::Client::CacheableArrayList::Create((System::Collections::IList^)key)));
          return std::dynamic_pointer_cast<native::CacheableKey>(kPtr);
        }
        case native::GeodeTypeIds::CacheableLinkedList://generic linked list
        {
          native::CacheablePtr kPtr(SafeGenericMSerializableConvert((IGeodeSerializable^)Apache::Geode::Client::CacheableLinkedList::Create((System::Collections::Generic::LinkedList<Object^>^)key)));
          return std::dynamic_pointer_cast<native::CacheableKey>(kPtr);
        }
        case native::GeodeTypeIds::CacheableStack:
        {
          native::CacheablePtr kPtr(SafeGenericMSerializableConvert(Apache::Geode::Client::CacheableStack::Create((System::Collections::ICollection^)key)));
          return std::dynamic_pointer_cast<native::CacheableKey>(kPtr);
        }
        case 7: //GeodeClassIds::CacheableManagedObject
        {
          native::CacheablePtr kPtr(SafeGenericMSerializableConvert((Apache::Geode::Client::CacheableObject^)key));
          return std::dynamic_pointer_cast<native::CacheableKey>(kPtr);
        }
        case 8://GeodeClassIds::CacheableManagedObjectXml
        {
          native::CacheablePtr kPtr(SafeGenericMSerializableConvert((Apache::Geode::Client::CacheableObjectXml^)key));
          return std::dynamic_pointer_cast<native::CacheableKey>(kPtr);
        }
        case native::GeodeTypeIds::CacheableObjectArray:
        {
          native::CacheablePtr kPtr(SafeGenericMSerializableConvert((Apache::Geode::Client::CacheableObjectArray^)key));
          return std::dynamic_pointer_cast<native::CacheableKey>(kPtr);
        }
        case native::GeodeTypeIds::CacheableIdentityHashMap:
        {
          native::CacheablePtr kPtr(SafeGenericMSerializableConvert(Apache::Geode::Client::CacheableIdentityHashMap::Create((System::Collections::IDictionary^)key)));
          return std::dynamic_pointer_cast<native::CacheableKey>(kPtr);
        }
        case native::GeodeTypeIds::CacheableHashSet://no need of it, default case should work
        {
          native::CacheablePtr kPtr(SafeGenericMSerializableConvert((Apache::Geode::Client::CacheableHashSet^)key));
          return std::dynamic_pointer_cast<native::CacheableKey>(kPtr);
        }
        case native::GeodeTypeIds::CacheableLinkedHashSet://no need of it, default case should work
        {
          native::CacheablePtr kPtr(SafeGenericMSerializableConvert((Apache::Geode::Client::CacheableLinkedHashSet^)key));
          return std::dynamic_pointer_cast<native::CacheableKey>(kPtr);
        }
        case native::GeodeTypeIds::CacheableDate:
        {
          native::CacheablePtr kPtr(SafeGenericMSerializableConvert((IGeodeSerializable^)Apache::Geode::Client::CacheableDate::Create((System::DateTime)key)));
          return std::dynamic_pointer_cast<native::CacheableKey>(kPtr);
        }
        case native::GeodeTypeIds::BooleanArray:
        {
          native::CacheablePtr kPtr(SafeGenericMSerializableConvert((IGeodeSerializable^)Apache::Geode::Client::BooleanArray::Create((array<bool>^)key)));
          return std::dynamic_pointer_cast<native::CacheableKey>(kPtr);
        }
        case native::GeodeTypeIds::CharArray:
        {
          native::CacheablePtr kPtr(SafeGenericMSerializableConvert((IGeodeSerializable^)Apache::Geode::Client::CharArray::Create((array<Char>^)key)));
          return std::dynamic_pointer_cast<native::CacheableKey>(kPtr);
        }
        default:
        {
          native::CacheablePtr kPtr(SafeGenericMSerializableConvert(key));
          return std::dynamic_pointer_cast<native::CacheableKey>(kPtr);
        }
        }
      } //

      String^ Serializable::GetString(native::CacheableStringPtr cStr)//native::CacheableString*
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

      // These are the new static methods to get/put data from c++

      //byte
      Byte Serializable::getByte(native::SerializablePtr nativeptr)
      {
        native::CacheableByte* ci = static_cast<native::CacheableByte*>(nativeptr.get());
        return ci->value();
      }

      native::CacheableKeyPtr Serializable::getCacheableByte(SByte val)
      {
        return native::CacheableByte::create(val);
      }

      //boolean
      bool Serializable::getBoolean(native::SerializablePtr nativeptr)
      {
        native::CacheableBoolean* ci = static_cast<native::CacheableBoolean*>(nativeptr.get());
        return ci->value();
      }

      native::CacheableKeyPtr Serializable::getCacheableBoolean(bool val)
      {
        return native::CacheableBoolean::create(val);
      }

      //widechar
      Char Serializable::getChar(native::SerializablePtr nativeptr)
      {
        native::CacheableCharacter* ci = static_cast<native::CacheableCharacter*>(nativeptr.get());
        return ci->value();
      }

      native::CacheableKeyPtr Serializable::getCacheableWideChar(Char val)
      {
        return native::CacheableCharacter::create(val);
      }

      //double
      double Serializable::getDouble(native::SerializablePtr nativeptr)
      {
        native::CacheableDouble* ci = static_cast<native::CacheableDouble*>(nativeptr.get());
        return ci->value();
      }

      native::CacheableKeyPtr Serializable::getCacheableDouble(double val)
      {
        return native::CacheableDouble::create(val);
      }

      //float
      float Serializable::getFloat(native::SerializablePtr nativeptr)
      {
        native::CacheableFloat* ci = static_cast<native::CacheableFloat*>(nativeptr.get());
        return ci->value();
      }

      native::CacheableKeyPtr Serializable::getCacheableFloat(float val)
      {
        return native::CacheableFloat::create(val);
      }

      //int16
      System::Int16 Serializable::getInt16(native::SerializablePtr nativeptr)
      {
        native::CacheableInt16* ci = static_cast<native::CacheableInt16*>(nativeptr.get());
        return ci->value();
      }

      native::CacheableKeyPtr Serializable::getCacheableInt16(int val)
      {
        return native::CacheableInt16::create(val);
      }

      //int32
      System::Int32 Serializable::getInt32(native::SerializablePtr nativeptr)
      {
        native::CacheableInt32* ci = static_cast<native::CacheableInt32*>(nativeptr.get());
        return ci->value();
      }

      native::CacheableKeyPtr Serializable::getCacheableInt32(System::Int32 val)
      {
        return native::CacheableInt32::create(val);
      }

      //int64
      System::Int64 Serializable::getInt64(native::SerializablePtr nativeptr)
      {
        native::CacheableInt64* ci = static_cast<native::CacheableInt64*>(nativeptr.get());
        return ci->value();
      }

      native::CacheableKeyPtr Serializable::getCacheableInt64(System::Int64 val)
      {
        return native::CacheableInt64::create(val);
      }

      //cacheable ascii string
      String^ Serializable::getASCIIString(native::SerializablePtr nativeptr)
      {
        return GetString(nativeptr->toString());
      }

      native::CacheableKeyPtr Serializable::getCacheableASCIIString(String^ val)
      {
        return GetCacheableString(val);
      }

      native::CacheableKeyPtr Serializable::getCacheableASCIIString2(String^ val)
      {
        return GetCacheableString2(val);
      }

      //cacheable ascii string huge
      String^ Serializable::getASCIIStringHuge(native::SerializablePtr nativeptr)
      {
        return GetString(nativeptr->toString());
      }

      native::CacheableKeyPtr Serializable::getCacheableASCIIStringHuge(String^ val)
      {
        return GetCacheableString(val);
      }

      //cacheable string
      String^ Serializable::getUTFString(native::SerializablePtr nativeptr)
      {
        return GetString(nativeptr->toString());
      }

      native::CacheableKeyPtr Serializable::getCacheableUTFString(String^ val)
      {
        return GetCacheableString(val);
      }

      //cacheable string huge
      String^ Serializable::getUTFStringHuge(native::SerializablePtr nativeptr)
      {
        return GetString(nativeptr->toString());
      }

      native::CacheableKeyPtr Serializable::getCacheableUTFStringHuge(String^ val)
      {
        return GetCacheableString(val);
      }

      native::CacheableStringPtr Serializable::GetCacheableString(String^ value)
      {
        native::CacheableStringPtr cStr;
        size_t len = 0;
        if (value != nullptr) {
          len = value->Length;
          pin_ptr<const wchar_t> pin_value = PtrToStringChars(value);
          cStr = native::CacheableString::create(pin_value, Convert::ToInt32(len));
        }
        else {
          cStr.reset(static_cast<native::CacheableString *>(
            native::CacheableString::createDeserializable()));
        }

        return cStr;
      }

      native::CacheableStringPtr Serializable::GetCacheableString2(String^ value)
      {
        native::CacheableStringPtr cStr;
        size_t len = 0;
        if (value != nullptr) {
          len = value->Length;
          const char* chars = (const char*)(Marshal::StringToHGlobalAnsi(value)).ToPointer();
          try
          {
            cStr = native::CacheableString::create(chars, Convert::ToInt32(len));
          }
          finally
          {
            Marshal::FreeHGlobal(IntPtr((void*)chars));
          }
        }
        else {
          cStr.reset(static_cast<native::CacheableString*>(
            native::CacheableString::createDeserializable()));
        }

        return cStr;
      }

      array<Byte>^ Serializable::getSByteArray(array<SByte>^ sArray)
      {
        array<Byte>^ dArray = gcnew array<Byte>(sArray->Length);
        for (int index = 0; index < dArray->Length; index++)
        {
          dArray[index] = sArray[index];
        }
        return dArray;
      }

      array<System::Int16>^ Serializable::getInt16Array(array<System::UInt16>^ sArray)
      {
        array<System::Int16>^ dArray = gcnew array<System::Int16>(sArray->Length);
        for (int index = 0; index < dArray->Length; index++)
        {
          dArray[index] = sArray[index];
        }
        return dArray;
      }

      array<System::Int32>^ Serializable::getInt32Array(array<System::UInt32>^ sArray)
      {
        array<System::Int32>^ dArray = gcnew array<System::Int32>(sArray->Length);
        for (int index = 0; index < dArray->Length; index++)
        {
          dArray[index] = sArray[index];
        }
        return dArray;
      }

      array<System::Int64>^ Serializable::getInt64Array(array<System::UInt64>^ sArray)
      {
        array<System::Int64>^ dArray = gcnew array<System::Int64>(sArray->Length);
        for (int index = 0; index < dArray->Length; index++)
        {
          dArray[index] = sArray[index];
        }
        return dArray;
      }
    }  // namespace Client
  }  // namespace Geode
}  // namespace Apache
