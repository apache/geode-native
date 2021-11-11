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
#include "String.hpp"

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
      using namespace msclr::interop;
      namespace native = apache::geode::client;

      System::UInt64 Apache::Geode::Client::Serializable::ObjectSize::get()
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

      String^ Apache::Geode::Client::Serializable::ToString()
      {
        try
        {
          return to_String(m_nativeptr->get()->toString());
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }
      }

      System::Int32 Serializable::GetPDXIdForType(native::Pool* pool, ISerializable^ pdxType, Cache^ cache)
      {
        std::shared_ptr<native::Cacheable> kPtr(GetNativeWrapperForManagedObject(pdxType));
        return CacheRegionHelper::getCacheImpl(cache->GetNative().get())
            ->getSerializationRegistry()
            ->GetPDXIdForType(pool, kPtr);
      }

      ISerializable^ Serializable::GetPDXTypeById(native::Pool* pool, System::Int32 typeId, Cache^ cache)
      {        
        auto sPtr =  CacheRegionHelper::getCacheImpl(cache->GetNative().get())
            ->getSerializationRegistry()
            ->GetPDXTypeById(pool, typeId);
        return SafeUMSerializableConvertGeneric(sPtr);
      }

      int Serializable::GetEnumValue(Internal::EnumInfo^ ei, Cache^ cache)
      {
        std::shared_ptr<native::Cacheable> kPtr(GetNativeWrapperForManagedObject(ei));
        auto&& cacheImpl = CacheRegionHelper::getCacheImpl(cache->GetNative().get());
        return cacheImpl->getSerializationRegistry()->GetEnumValue(cacheImpl->getDefaultPool(), kPtr);
      }

      Internal::EnumInfo^ Serializable::GetEnum(int val, Cache^ cache)
      {
        auto cacheImpl = CacheRegionHelper::getCacheImpl(cache->GetNative().get());
        auto sPtr = cacheImpl->getSerializationRegistry()->GetEnum(cacheImpl->getDefaultPool(), val);
        return (Internal::EnumInfo^)SafeUMSerializableConvertGeneric(sPtr);
      }

      generic<class TKey>
      std::shared_ptr<native::CacheableKey> Serializable::GetUnmanagedValueGeneric(TKey key)
      {
        if (key != nullptr) {
          return GetUnmanagedValueGeneric(key->GetType(), key);
        }
        return nullptr;
      }

      generic<class TKey>
      std::shared_ptr<native::CacheableKey> Serializable::GetUnmanagedValueGeneric(TKey key, bool isAciiChar)
      {
        if (key != nullptr) {
          return GetUnmanagedValueGeneric(key->GetType(), key, isAciiChar);
        }
        return nullptr;
      }

      generic<class TKey>
      std::shared_ptr<native::CacheableKey> Serializable::GetUnmanagedValueGeneric(
        Type^ managedType, TKey key)
      {
        return GetUnmanagedValueGeneric(managedType, key, false);
      }

      generic<class TKey>
      std::shared_ptr<native::CacheableKey> Serializable::GetUnmanagedValueGeneric(
        Type^ managedType, TKey key, bool isAsciiChar)
      {
        Byte typeId = Apache::Geode::Client::TypeRegistry::GetDsCodeForManagedType(managedType);

        switch (typeId)
        {
        case native::internal::DSCode::CacheableByte: {
          return Serializable::getCacheableByte((Byte)key);
        }
        case native::internal::DSCode::CacheableBoolean:
          return Serializable::getCacheableBoolean((bool)key);
        case native::internal::DSCode::CacheableCharacter:
          return Serializable::getCacheableWideChar((Char)key);
        case native::internal::DSCode::CacheableDouble:
          return Serializable::getCacheableDouble((double)key);
        case native::internal::DSCode::CacheableASCIIString:
          return Serializable::GetCacheableString((String^)key);
        case native::internal::DSCode::CacheableFloat:
          return Serializable::getCacheableFloat((float)key);
        case native::internal::DSCode::CacheableInt16: {
          return Serializable::getCacheableInt16((System::Int16)key);
        }
        case native::internal::DSCode::CacheableInt32: {
          return Serializable::getCacheableInt32((System::Int32)key);
        }
        case native::internal::DSCode::CacheableInt64: {
          return Serializable::getCacheableInt64((System::Int64)key);
        }
        case native::internal::DSCode::CacheableBytes:
        {
          return GetNativeCacheableKeyWrapperForManagedISerializable(Apache::Geode::Client::CacheableBytes::Create((array<Byte>^)key));
        }
        case native::internal::DSCode::CacheableDoubleArray:
        {
          return GetNativeCacheableKeyWrapperForManagedISerializable(Apache::Geode::Client::CacheableDoubleArray::Create((array<Double>^)key));
        }
        case native::internal::DSCode::CacheableFloatArray:
        {
          return GetNativeCacheableKeyWrapperForManagedISerializable(Apache::Geode::Client::CacheableFloatArray::Create((array<float>^)key));
        }
        case native::internal::DSCode::CacheableInt16Array:
        {
          return GetNativeCacheableKeyWrapperForManagedISerializable(Apache::Geode::Client::CacheableInt16Array::Create((array<Int16>^)key));
        }
        case native::internal::DSCode::CacheableInt32Array:
        {
          return GetNativeCacheableKeyWrapperForManagedISerializable(Apache::Geode::Client::CacheableInt32Array::Create((array<Int32>^)key));
        }
        case native::internal::DSCode::CacheableInt64Array:
        {
          return GetNativeCacheableKeyWrapperForManagedISerializable(Apache::Geode::Client::CacheableInt64Array::Create((array<Int64>^)key));
        }
        case native::internal::DSCode::CacheableStringArray:
        {
          return GetNativeCacheableKeyWrapperForManagedISerializable(Apache::Geode::Client::CacheableStringArray::Create((array<String^>^)key));
        }
        case native::internal::DSCode::CacheableFileName:
        {
          return GetNativeCacheableKeyWrapperForManagedISerializable((Apache::Geode::Client::CacheableFileName^)key);
        }
        case native::internal::DSCode::CacheableHashTable://collection::hashtable
        {
          return GetNativeCacheableKeyWrapperForManagedISerializable(Apache::Geode::Client::CacheableHashTable::Create((System::Collections::Hashtable^)key));
        }
        case native::internal::DSCode::CacheableHashMap://generic dictionary
        {
          return GetNativeCacheableKeyWrapperForManagedISerializable(Apache::Geode::Client::CacheableHashMap::Create((System::Collections::IDictionary^)key));
        }
        case native::internal::DSCode::CacheableVector://collection::arraylist
        {
          return GetNativeCacheableKeyWrapperForManagedISerializable(CacheableVector::Create((System::Collections::IList^)key));
        }
        case native::internal::DSCode::CacheableArrayList://generic ilist
        {
          return GetNativeCacheableKeyWrapperForManagedISerializable(Apache::Geode::Client::CacheableArrayList::Create((System::Collections::IList^)key));
        }
        case native::internal::DSCode::CacheableLinkedList://generic linked list
        {
          return GetNativeCacheableKeyWrapperForManagedISerializable(Apache::Geode::Client::CacheableLinkedList::Create((System::Collections::Generic::LinkedList<Object^>^)key));
        }
        case native::internal::DSCode::CacheableStack:
        {
          return GetNativeCacheableKeyWrapperForManagedISerializable(Apache::Geode::Client::CacheableStack::Create((System::Collections::ICollection^)key));
        }
        case native::internal::InternalId::CacheableManagedObject:
        {
          return GetNativeCacheableKeyWrapperForManagedISerializable((Apache::Geode::Client::CacheableObject^)key);
        }
        case native::internal::InternalId::CacheableManagedObjectXml:
        {
          return GetNativeCacheableKeyWrapperForManagedISerializable((Apache::Geode::Client::CacheableObjectXml^)key);
        }
        case native::internal::DSCode::CacheableObjectArray:
        {
          return GetNativeCacheableKeyWrapperForManagedISerializable((Apache::Geode::Client::CacheableObjectArray^)key);
        }
        case native::internal::DSCode::CacheableIdentityHashMap:
        {
          return GetNativeCacheableKeyWrapperForManagedISerializable(Apache::Geode::Client::CacheableIdentityHashMap::Create((System::Collections::IDictionary^)key));
        }
        case native::internal::DSCode::CacheableHashSet://no need of it, default case should work
        {
          return GetNativeCacheableKeyWrapperForManagedISerializable((Apache::Geode::Client::CacheableHashSet^)key);
        }
        case native::internal::DSCode::CacheableLinkedHashSet://no need of it, default case should work
        {
          return GetNativeCacheableKeyWrapperForManagedISerializable((Apache::Geode::Client::CacheableLinkedHashSet^)key);
        }
        case native::internal::DSCode::CacheableDate:
        {
          return GetNativeCacheableKeyWrapperForManagedISerializable(Apache::Geode::Client::CacheableDate::Create((System::DateTime)key));
        }
        case native::internal::DSCode::BooleanArray:
        {
          return GetNativeCacheableKeyWrapperForManagedISerializable(Apache::Geode::Client::BooleanArray::Create((array<bool>^)key));
        }
        case native::internal::DSCode::CharArray:
        {
          return GetNativeCacheableKeyWrapperForManagedISerializable(Apache::Geode::Client::CharArray::Create((array<Char>^)key));
        }
        default:
        {
          std::shared_ptr<native::Cacheable> kPtr(GetNativeWrapperForManagedObject(key));
          return std::dynamic_pointer_cast<native::CacheableKey>(kPtr);
        }
        }
      } //
      
      std::shared_ptr<native::CacheableKey> Serializable::GetNativeCacheableKeyWrapperForManagedISerializable(ISerializable^ managedObject) {
        if (nullptr == managedObject) {
          return __nullptr;
        }
        auto wrappedObject = std::shared_ptr<native::Serializable>(GetNativeWrapperForManagedObject(managedObject));
        return std::dynamic_pointer_cast<native::CacheableKey>(wrappedObject);
      }

      // These are the new static methods to get/put data from c++

      //byte
      Byte Serializable::getByte(std::shared_ptr<native::Serializable> nativeptr)
      {
        auto ci = dynamic_cast<native::CacheableByte*>(nativeptr.get());
        return ci->value();
      }

      std::shared_ptr<native::CacheableKey> Serializable::getCacheableByte(Byte val)
      {
        return native::CacheableByte::create(val);
      }

      //boolean
      bool Serializable::getBoolean(std::shared_ptr<native::Serializable> nativeptr)
      {
        auto ci = dynamic_cast<native::CacheableBoolean*>(nativeptr.get());
        return ci->value();
      }

      std::shared_ptr<native::CacheableKey> Serializable::getCacheableBoolean(bool val)
      {
        return native::CacheableBoolean::create(val);
      }

      //widechar
      Char Serializable::getChar(std::shared_ptr<native::Serializable> nativeptr)
      {
        auto ci = dynamic_cast<native::CacheableCharacter*>(nativeptr.get());
        return ci->value();
      }

      std::shared_ptr<native::CacheableKey> Serializable::getCacheableWideChar(Char val)
      {
        return native::CacheableCharacter::create(val);
      }

      //double
      double Serializable::getDouble(std::shared_ptr<native::Serializable> nativeptr)
      {
        auto ci = dynamic_cast<native::CacheableDouble*>(nativeptr.get());
        return ci->value();
      }

      std::shared_ptr<native::CacheableKey> Serializable::getCacheableDouble(double val)
      {
        return native::CacheableDouble::create(val);
      }

      //float
      float Serializable::getFloat(std::shared_ptr<native::Serializable> nativeptr)
      {
        auto ci = dynamic_cast<native::CacheableFloat*>(nativeptr.get());
        return ci->value();
      }

      std::shared_ptr<native::CacheableKey> Serializable::getCacheableFloat(float val)
      {
        return native::CacheableFloat::create(val);
      }

      //int16
      System::Int16 Serializable::getInt16(std::shared_ptr<native::Serializable> nativeptr)
      {
        auto ci = dynamic_cast<native::CacheableInt16*>(nativeptr.get());
        return ci->value();
      }

      std::shared_ptr<native::CacheableKey> Serializable::getCacheableInt16(int val)
      {
        return native::CacheableInt16::create(val);
      }

      //int32
      System::Int32 Serializable::getInt32(std::shared_ptr<native::Serializable> nativeptr)
      {
        auto ci = dynamic_cast<native::CacheableInt32*>(nativeptr.get());
        return ci->value();
      }

      std::shared_ptr<native::CacheableKey> Serializable::getCacheableInt32(System::Int32 val)
      {
        return native::CacheableInt32::create(val);
      }

      //int64
      System::Int64 Serializable::getInt64(std::shared_ptr<native::Serializable> nativeptr)
      {
        auto ci = dynamic_cast<native::CacheableInt64*>(nativeptr.get());
        return ci->value();
      }

      std::shared_ptr<native::CacheableKey> Serializable::getCacheableInt64(System::Int64 val)
      {
        return native::CacheableInt64::create(val);
      }

      //cacheable ascii string
      String^ Serializable::getString(std::shared_ptr<native::Serializable> nativeptr)
      {
        if (auto cacheableString = std::dynamic_pointer_cast<native::CacheableString>(nativeptr))
        {
          return to_String(cacheableString->value());
        }

        return to_String(nativeptr->toString());
      }

      std::shared_ptr<native::CacheableString> Serializable::GetCacheableString(String^ value)
      {
        std::shared_ptr<native::CacheableString> cStr;
        if (value) {
          cStr = native::CacheableString::create(to_utf8(value));
        }
        else {
          cStr = std::dynamic_pointer_cast<native::CacheableString>(native::CacheableString::createDeserializable());
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
