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
#include <tuple>
#include "geode/Region.hpp"
#include "geode/Cache.hpp"
#include "end_native.hpp"

#include "Region.hpp"
#include "Cache.hpp"
#include "CacheStatistics.hpp"
#include "RegionAttributes.hpp"
#include "AttributesMutator.hpp"
#include "RegionEntry.hpp"
#include "ISelectResults.hpp"
#include "IGeodeSerializable.hpp"
#include "ResultSet.hpp"
#include "StructSet.hpp"
#include "impl/AuthenticatedView.hpp"
#include "impl/SafeConvert.hpp"
#include "LocalRegion.hpp"
#include "Pool.hpp"
#include "PoolManager.hpp"
#include "SystemProperties.hpp"
#include "impl/CacheResolver.hpp"
#include "TimeUtils.hpp"


namespace Apache
{
  namespace Geode
  {
    namespace Client
    {

      using namespace System;
      using namespace msclr::interop;

      namespace native = apache::geode::client;

      generic<class TKey, class TValue>
      TValue Region<TKey, TValue>::Get(TKey key, Object^ callbackArg)
      {
        std::shared_ptr<native::CacheableKey> keyptr = Serializable::GetUnmanagedValueGeneric<TKey>(key);
        std::shared_ptr<native::Serializable> callbackptr = Serializable::GetUnmanagedValueGeneric<Object^>(callbackArg);
        GC::KeepAlive(m_nativeptr);
        auto nativeptr = this->get(keyptr, callbackptr);
        if (nativeptr == nullptr)
        {
          throw gcnew KeyNotFoundException("The given key was not present in the region.");
        }
        TValue returnVal = TypeRegistry::GetManagedValueGeneric<TValue>(nativeptr);
        return returnVal;
      }

      generic<class TKey, class TValue>
      std::shared_ptr<native::Serializable> Region<TKey, TValue>::get(std::shared_ptr<native::CacheableKey>& keyptr, std::shared_ptr<native::Serializable>& callbackptr)
      {
        _GF_MG_EXCEPTION_TRY2/* due to auto replace */
          try
          {
            return m_nativeptr->get()->get(keyptr, callbackptr);
          }
          finally
          {
            GC::KeepAlive(m_nativeptr);
          }
        _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */
      }

      generic<class TKey, class TValue>
      std::shared_ptr<native::Serializable> Region<TKey, TValue>::get(std::shared_ptr<native::CacheableKey>& keyptr)
      {
        _GF_MG_EXCEPTION_TRY2/* due to auto replace */
          try
          {
            return m_nativeptr->get()->get(keyptr);
          }
          finally
          {
            GC::KeepAlive(m_nativeptr);
          }
        _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */
      }

      generic<class TKey, class TValue>
      bool Region<TKey, TValue>::isPoolInMultiuserMode()
      {
        _GF_MG_EXCEPTION_TRY2/* due to auto replace */
          auto rAttributes = this->Attributes;
          auto poolName = rAttributes->PoolName;
          if (poolName != nullptr) {
            auto poolManager = gcnew PoolManager(m_nativeptr->get()->getCache().getPoolManager());
            auto pool = poolManager->Find(poolName);
            if (pool != nullptr && !pool->Destroyed) {
              return pool->MultiuserAuthentication;
            }
          }
          return false;
        _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */
      }

      generic<class TKey, class TValue>
      void Region<TKey, TValue>::Put(TKey key, TValue value, Object^ callbackArg)
      {
        _GF_MG_EXCEPTION_TRY2/* due to auto replace */

          try
          {
            std::shared_ptr<native::CacheableKey> keyptr = Serializable::GetUnmanagedValueGeneric<TKey>(key);
            std::shared_ptr<native::Cacheable> valueptr = Serializable::GetUnmanagedValueGeneric<TValue>(value);
            std::shared_ptr<native::Serializable> callbackptr = Serializable::GetUnmanagedValueGeneric<Object^>(callbackArg);
            m_nativeptr->get()->put(keyptr, valueptr, callbackptr);
          }
          finally
          {
            GC::KeepAlive(m_nativeptr);
          }
        
        _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */
      }

      generic<class TKey, class TValue>
      TValue Region<TKey, TValue>::default::get(TKey key)
      {
        std::shared_ptr<native::CacheableKey> keyptr = Serializable::GetUnmanagedValueGeneric<TKey>(key);
        GC::KeepAlive(m_nativeptr);
        auto nativeptr = this->get(keyptr);
        if (nativeptr == nullptr)
        {
          throw gcnew KeyNotFoundException("The given key was not present in the region.");
        }
        TValue returnVal = TypeRegistry::GetManagedValueGeneric<TValue>(nativeptr);
        return returnVal;
      }

      generic<class TKey, class TValue>
      void Region<TKey, TValue>::default::set(TKey key, TValue value)
      {
        _GF_MG_EXCEPTION_TRY2/* due to auto replace */

        try
        {
          std::shared_ptr<native::CacheableKey> keyptr = Serializable::GetUnmanagedValueGeneric<TKey>(key);
          std::shared_ptr<native::Cacheable> valueptr = Serializable::GetUnmanagedValueGeneric<TValue>(value);
          m_nativeptr->get()->put(keyptr, valueptr);
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }

        _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */
      }

      generic<class TKey, class TValue>
      System::Collections::Generic::IEnumerator<KeyValuePair<TKey, TValue>>^
        Region<TKey, TValue>::GetEnumerator()
      {
		std::vector<std::shared_ptr<native::RegionEntry>> vc;

        _GF_MG_EXCEPTION_TRY2/* due to auto replace */

          try
          {
            vc = m_nativeptr->get()->entries(false);
          }
          finally
          {
            GC::KeepAlive(m_nativeptr);
          }

        _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */

          auto toArray = gcnew array<KeyValuePair<TKey, TValue>>(static_cast<int>(vc.size()));

        for (System::Int32 index = 0; index < toArray->Length; index++)
        {
          auto& nativeptr = vc[index];
          auto key = TypeRegistry::GetManagedValueGeneric<TKey>(nativeptr->getKey());
          auto val = TypeRegistry::GetManagedValueGeneric<TValue>(nativeptr->getValue());
          toArray[index] = KeyValuePair<TKey, TValue>(key, val);
        }
        return ((System::Collections::Generic::IEnumerable<KeyValuePair<TKey, TValue>>^)toArray)->GetEnumerator();
      }

      generic<class TKey, class TValue>
      System::Collections::IEnumerator^
        Region<TKey, TValue>::GetEnumeratorOld()
      {
		std::vector<std::shared_ptr<native::RegionEntry>> vc;

        _GF_MG_EXCEPTION_TRY2/* due to auto replace */

          try
          {
            vc = m_nativeptr->get()->entries(false);
          }
          finally
          {
            GC::KeepAlive(m_nativeptr);
          }

        _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */

         auto toArray = gcnew array<Object^>(static_cast<int>(vc.size()));

        for (System::Int32 index = 0; index < toArray->Length; index++)
        {
          auto& nativeptr = vc[index];
          auto key = TypeRegistry::GetManagedValueGeneric<TKey>(nativeptr->getKey());
          auto val = TypeRegistry::GetManagedValueGeneric<TValue>(nativeptr->getValue());
          toArray[index] = KeyValuePair<TKey, TValue>(key, val);
        }
        return ((System::Collections::Generic::IEnumerable<Object^>^)toArray)->GetEnumerator();
      }

      generic<class TKey, class TValue>
      bool Region<TKey, TValue>::AreValuesEqual(std::shared_ptr<native::Cacheable>& val1, std::shared_ptr<native::Cacheable>& val2)
      {
        if (val1 == nullptr && val2 == nullptr)
        {
          return true;
        }
        else if ((val1 == nullptr && val2 != nullptr) || (val1 != nullptr && val2 == nullptr))
        {
          return false;
        }
        else if (val1 != nullptr && val2 != nullptr)
        {
          if (val1->classId() != val2->classId() || val1->typeId() != val2->typeId())
          {
            return false;
          }
          auto out1 = m_nativeptr->get_conditional_shared_ptr()->getCache().createDataOutput();
          auto out2 = m_nativeptr->get_conditional_shared_ptr()->getCache().createDataOutput();
          val1->toData(out1);
          val2->toData(out2);

          GC::KeepAlive(m_nativeptr);
          if (out1.getBufferLength() != out2.getBufferLength())
          {
            return false;
          }
          else if (memcmp(out1.getBuffer(), out2.getBuffer(), out1.getBufferLength()) != 0)
          {
            return false;
          }
          return true;
        }
        return false;
      }

      generic<class TKey, class TValue>
      bool Region<TKey, TValue>::Contains(KeyValuePair<TKey, TValue> keyValuePair)
      {
        std::shared_ptr<native::CacheableKey> keyptr = Serializable::GetUnmanagedValueGeneric<TKey>(keyValuePair.Key);
        GC::KeepAlive(m_nativeptr);
        auto nativeptr = this->get(keyptr);
        //This means that key is not present.
        if (nativeptr == nullptr) {
          return false;
        }
        auto value = TypeRegistry::GetManagedValueGeneric<TValue>(nativeptr);
        return ((Object^)value)->Equals(keyValuePair.Value);
      }

      generic<class TKey, class TValue>
      bool Region<TKey, TValue>::ContainsKey(TKey key)
      {
        _GF_MG_EXCEPTION_TRY2/* due to auto replace */

        try
        {
          std::shared_ptr<native::CacheableKey> keyptr = Serializable::GetUnmanagedValueGeneric<TKey>(key);
          return m_nativeptr->get()->containsKeyOnServer(keyptr);
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }

        _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */
      }

      generic<class TKey, class TValue>
      bool Region<TKey, TValue>::TryGetValue(TKey key, TValue %val)
      {
        _GF_MG_EXCEPTION_TRY2/* due to auto replace */
        std::shared_ptr<native::CacheableKey> keyptr = Serializable::GetUnmanagedValueGeneric<TKey>(key);
        GC::KeepAlive(m_nativeptr);
        auto nativeptr = this->get(keyptr);
        if (nativeptr == nullptr) {
          val = TValue();
          return false;
        }
        else {
          val = TypeRegistry::GetManagedValueGeneric<TValue>(nativeptr);
          return true;
        }

        _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */
      }

      generic<class TKey, class TValue>
      System::Collections::Generic::ICollection<TKey>^ Region<TKey, TValue>::Keys::get()
      {
        _GF_MG_EXCEPTION_TRY2/* due to auto replace */

          std::vector<std::shared_ptr<native::CacheableKey>> vc;
        try
        {
          vc = m_nativeptr->get()->serverKeys();
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }
        auto keyarr = gcnew array<TKey>(static_cast<int>(vc.size()));
        for (System::Int32 index = 0; index < keyarr->Length; index++)
        {
          auto& nativeptr = vc[index];
          keyarr[index] = TypeRegistry::GetManagedValueGeneric<TKey>(nativeptr);
        }
        auto collectionlist = (System::Collections::Generic::ICollection<TKey>^)keyarr;
        return collectionlist;

        _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */
      }

      generic<class TKey, class TValue>
      System::Collections::Generic::ICollection<TValue>^ Region<TKey, TValue>::Values::get()
      {
        _GF_MG_EXCEPTION_TRY2/* due to auto replace */

          std::vector<std::shared_ptr<native::Cacheable>> vc;
        try
        {
          vc = m_nativeptr->get()->values();
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }
        auto valarr = gcnew array<TValue>(static_cast<int>(vc.size()));
        for (System::Int32 index = 0; index < vc.size(); index++)
        {
          auto& nativeptr = vc[index];
          valarr[index] = TypeRegistry::GetManagedValueGeneric<TValue>(nativeptr);
        }
        auto collectionlist = (System::Collections::Generic::ICollection<TValue>^)valarr;
        return collectionlist;

        _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */
      }

      generic<class TKey, class TValue>
      void Region<TKey, TValue>::Add(TKey key, TValue value)
      {
        _GF_MG_EXCEPTION_TRY2/* due to auto replace */

        try
        {
          std::shared_ptr<native::CacheableKey> keyptr = Serializable::GetUnmanagedValueGeneric<TKey>(key);
          std::shared_ptr<native::Cacheable> valueptr = Serializable::GetUnmanagedValueGeneric<TValue>(value);
          m_nativeptr->get()->create(keyptr, valueptr);
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }

        _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */
      }

      generic<class TKey, class TValue>
      void Region<TKey, TValue>::Add(KeyValuePair<TKey, TValue> keyValuePair)
      {
        _GF_MG_EXCEPTION_TRY2/* due to auto replace */

        try
        {
          std::shared_ptr<native::CacheableKey> keyptr = Serializable::GetUnmanagedValueGeneric<TKey>(keyValuePair.Key);
          std::shared_ptr<native::Cacheable> valueptr = Serializable::GetUnmanagedValueGeneric<TValue>(keyValuePair.Value);
          m_nativeptr->get()->create(keyptr, valueptr);
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }

        _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */
      }

      generic<class TKey, class TValue>
      void Region<TKey, TValue>::Add(TKey key, TValue value, Object^ callbackArg)
      {
        _GF_MG_EXCEPTION_TRY2/* due to auto replace */

        try
        {
          std::shared_ptr<native::CacheableKey> keyptr = Serializable::GetUnmanagedValueGeneric<TKey>(key);
          std::shared_ptr<native::Cacheable> valueptr = Serializable::GetUnmanagedValueGeneric<TValue>(value);
          std::shared_ptr<native::Serializable> callbackptr = Serializable::GetUnmanagedValueGeneric<Object^>(callbackArg);
          m_nativeptr->get()->create(keyptr, valueptr, callbackptr);
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }

        _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */
      }

      generic<class TKey, class TValue>
      bool Region<TKey, TValue>::Remove(TKey key)
      {
        _GF_MG_EXCEPTION_TRY2/* due to auto replace */

        try
        {
          std::shared_ptr<native::CacheableKey> keyptr = Serializable::GetUnmanagedValueGeneric<TKey>(key);
          return m_nativeptr->get()->removeEx(keyptr);
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }

        _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */

      }

      generic<class TKey, class TValue>
      bool Region<TKey, TValue>::Remove(TKey key, Object^ callbackArg)
      {
        _GF_MG_EXCEPTION_TRY2/* due to auto replace */

        try
        {
          std::shared_ptr<native::CacheableKey> keyptr = Serializable::GetUnmanagedValueGeneric<TKey>(key);
          std::shared_ptr<native::Serializable> callbackptr = Serializable::GetUnmanagedValueGeneric<Object^>(callbackArg);
          return m_nativeptr->get()->removeEx(keyptr, callbackptr);
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }

        _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */
      }

      generic<class TKey, class TValue>
      bool Region<TKey, TValue>::Remove(KeyValuePair<TKey, TValue> keyValuePair)
      {
        _GF_MG_EXCEPTION_TRY2/* due to auto replace */

        try
        {
          std::shared_ptr<native::CacheableKey> keyptr = Serializable::GetUnmanagedValueGeneric<TKey>(keyValuePair.Key);
          std::shared_ptr<native::Cacheable> valueptr = Serializable::GetUnmanagedValueGeneric<TValue>(keyValuePair.Value);
          return m_nativeptr->get()->remove(keyptr, valueptr);
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }

        _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */
      }

      generic<class TKey, class TValue>
      bool Region<TKey, TValue>::Remove(TKey key, TValue value, Object^ callbackArg)
      {
        _GF_MG_EXCEPTION_TRY2/* due to auto replace */

        try
        {
          std::shared_ptr<native::CacheableKey> keyptr = Serializable::GetUnmanagedValueGeneric<TKey>(key);
          std::shared_ptr<native::Cacheable> valueptr = Serializable::GetUnmanagedValueGeneric<TValue>(value);
          std::shared_ptr<native::Serializable> callbackptr = Serializable::GetUnmanagedValueGeneric<Object^>(callbackArg);
          GC::KeepAlive(m_nativeptr);
          return m_nativeptr->get()->remove(keyptr, valueptr, callbackptr);
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }

        _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */
      }

      generic<class TKey, class TValue>
      void Region<TKey, TValue>::InvalidateRegion()
      {
        _GF_MG_EXCEPTION_TRY2/* due to auto replace */

          InvalidateRegion(nullptr);

        _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */
      }

      generic<class TKey, class TValue>
      void Region<TKey, TValue>::InvalidateRegion(Object^ callbackArg)
      {
        _GF_MG_EXCEPTION_TRY2/* due to auto replace */

        try
        {
          std::shared_ptr<native::Serializable> callbackptr = Serializable::GetUnmanagedValueGeneric<Object^>(callbackArg);
          m_nativeptr->get()->invalidateRegion(callbackptr);
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }

        _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */
      }

      generic<class TKey, class TValue>
      void Region<TKey, TValue>::DestroyRegion()
      {
        _GF_MG_EXCEPTION_TRY2/* due to auto replace */

          DestroyRegion(nullptr);

        _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */
      }

      generic<class TKey, class TValue>
      void Region<TKey, TValue>::DestroyRegion(Object^ callbackArg)
      {
        _GF_MG_EXCEPTION_TRY2/* due to auto replace */

        try
        {
          std::shared_ptr<native::Serializable> callbackptr = Serializable::GetUnmanagedValueGeneric<Object^>(callbackArg);
          m_nativeptr->get()->destroyRegion(callbackptr);
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }

        _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */
      }

      generic<class TKey, class TValue>
      void Region<TKey, TValue>::Invalidate(TKey key)
      {
        _GF_MG_EXCEPTION_TRY2/* due to auto replace */

          Invalidate(key, nullptr);

        _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */

      }

      generic<class TKey, class TValue>
      void Region<TKey, TValue>::Invalidate(TKey key, Object^ callbackArg)
      {
        _GF_MG_EXCEPTION_TRY2/* due to auto replace */

        try
        {
          std::shared_ptr<native::CacheableKey> keyptr = Serializable::GetUnmanagedValueGeneric<TKey>(key);
          std::shared_ptr<native::Serializable> callbackptr = Serializable::GetUnmanagedValueGeneric<Object^>(callbackArg);
          m_nativeptr->get()->invalidate(keyptr, callbackptr);
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }

        _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */
      }

      generic<class TKey, class TValue>
      void Region<TKey, TValue>::PutAll(System::Collections::Generic::IDictionary<TKey, TValue>^ map)
      {
        _GF_MG_EXCEPTION_TRY2/* due to auto replace */

          return PutAll(map, TimeUtils::DurationToTimeSpan(native::DEFAULT_RESPONSE_TIMEOUT));

        _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */

      }

      generic<class TKey, class TValue>
      void Region<TKey, TValue>::PutAll(System::Collections::Generic::IDictionary<TKey, TValue>^ map, TimeSpan timeout)
      {
        _GF_MG_EXCEPTION_TRY2/* due to auto replace */

          native::HashMapOfCacheable nativeMap;
        for each (KeyValuePair<TKey, TValue> keyValPair in map)
        {
          std::shared_ptr<native::CacheableKey> keyptr = Serializable::GetUnmanagedValueGeneric<TKey>(keyValPair.Key);
          std::shared_ptr<native::Cacheable> valueptr = Serializable::GetUnmanagedValueGeneric<TValue>(keyValPair.Value);
          nativeMap.emplace(keyptr, valueptr);
        }
        try
        {
          m_nativeptr->get()->putAll(nativeMap, TimeUtils::TimeSpanToDurationCeil<std::chrono::seconds>(timeout));
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }

        _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */

      }

      generic<class TKey, class TValue>
      void Region<TKey, TValue>::PutAll(System::Collections::Generic::IDictionary<TKey, TValue>^ map, TimeSpan timeout, Object^ callbackArg)
      {
        _GF_MG_EXCEPTION_TRY2/* due to auto replace */

          native::HashMapOfCacheable nativeMap;
        for each (KeyValuePair<TKey, TValue> keyValPair in map)
        {
          std::shared_ptr<native::CacheableKey> keyptr = Serializable::GetUnmanagedValueGeneric<TKey>(keyValPair.Key);
          std::shared_ptr<native::Cacheable> valueptr = Serializable::GetUnmanagedValueGeneric<TValue>(keyValPair.Value);
          nativeMap.emplace(keyptr, valueptr);
        }
        std::shared_ptr<native::Serializable> callbackptr = Serializable::GetUnmanagedValueGeneric<Object^>(callbackArg);
        try
        {
          m_nativeptr->get()->putAll(nativeMap, TimeUtils::TimeSpanToDurationCeil<std::chrono::seconds>(timeout), callbackptr);
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }

        _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */

      }

      generic<class TKey, class TValue>
      void Region<TKey, TValue>::GetAll(System::Collections::Generic::ICollection<TKey>^ keys,
                                        System::Collections::Generic::IDictionary<TKey, TValue>^ values,
                                        System::Collections::Generic::IDictionary<TKey, System::Exception^>^ exceptions)
      {
        _GF_MG_EXCEPTION_TRY2/* due to auto replace */

          GetAll(keys, values, exceptions, false);

        _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */

      }

      generic<class TKey, class TValue>
      void Region<TKey, TValue>::GetAll(System::Collections::Generic::ICollection<TKey>^ keys,
                                        System::Collections::Generic::IDictionary<TKey, TValue>^ values,
                                        System::Collections::Generic::IDictionary<TKey, System::Exception^>^ exceptions,
                                        bool addToLocalCache)
      {
        if (keys != nullptr) {
          _GF_MG_EXCEPTION_TRY2/* due to auto replace */

            std::vector<std::shared_ptr<native::CacheableKey>> vecKeys;

          for each(TKey item in keys)
          {
            vecKeys.push_back(
              Serializable::GetUnmanagedValueGeneric<TKey>(item));
          }

         native::HashMapOfCacheable native_value;
        
          try
          {
            native_value = m_nativeptr->get()->getAll(vecKeys);
          }
          finally
          {
            GC::KeepAlive(m_nativeptr);
          }
          if (values != nullptr) {
            for (const auto& iter : native_value) {
              TKey key = TypeRegistry::GetManagedValueGeneric<TKey>(iter.first);
              TValue val = TypeRegistry::GetManagedValueGeneric<TValue>(iter.second);
              values->Add(key, val);
            }
          }

          _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */
        }
        else {
          throw gcnew IllegalArgumentException("GetAll: null keys provided");
        }
      }

      generic<class TKey, class TValue>
      void Region<TKey, TValue>::GetAll(System::Collections::Generic::ICollection<TKey>^ keys,
                                        System::Collections::Generic::IDictionary<TKey, TValue>^ values,
                                        System::Collections::Generic::IDictionary<TKey, System::Exception^>^ exceptions,
                                        bool addToLocalCache, Object^ callbackArg)
      {
        if (keys != nullptr) {
          _GF_MG_EXCEPTION_TRY2/* due to auto replace */

            std::vector<std::shared_ptr<native::CacheableKey>> vecKeys;

          for each(TKey item in keys)
          {
            vecKeys.push_back(
              Serializable::GetUnmanagedValueGeneric<TKey>(item));
          }

          std::shared_ptr<native::HashMapOfCacheable> valuesPtr;
          if (values != nullptr) {
            valuesPtr = std::make_shared<native::HashMapOfCacheable>();
          }
          auto callbackptr = Serializable::GetUnmanagedValueGeneric<Object^>(callbackArg);
          native::HashMapOfCacheable native_value;
          try
          {
            native_value = m_nativeptr->get()->getAll(vecKeys, callbackptr);
          }
          finally
          {
            GC::KeepAlive(m_nativeptr);
          }
          if (values != nullptr) {
            for (const auto& iter : native_value) {
              TKey key = TypeRegistry::GetManagedValueGeneric<TKey>(iter.first);
              TValue val = TypeRegistry::GetManagedValueGeneric<TValue>(iter.second);
              values->Add(key, val);
            }
          }
 
          _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */
        }
        else {
          throw gcnew IllegalArgumentException("GetAll: null keys provided");
        }
      }

      generic<class TKey, class TValue>
      void Region<TKey, TValue>::RemoveAll(System::Collections::Generic::ICollection<TKey>^ keys)
      {
        _GF_MG_EXCEPTION_TRY2/* due to auto replace */

          RemoveAll(keys, nullptr);

        _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */

      }

      generic<class TKey, class TValue>
      void Region<TKey, TValue>::RemoveAll(System::Collections::Generic::ICollection<TKey>^ keys,
                                            Object^ callbackArg)
      {
        _GF_MG_EXCEPTION_TRY2/* due to auto replace */

          std::vector<std::shared_ptr<native::CacheableKey>> vecKeys;
        for each(TKey item in keys)
          vecKeys.push_back(Serializable::GetUnmanagedValueGeneric<TKey>(item));

        std::shared_ptr<native::Serializable> callbackptr = Serializable::GetUnmanagedValueGeneric<Object^>(callbackArg);

        try
        {
          m_nativeptr->get()->removeAll(vecKeys, callbackptr);
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }

        _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */

      }
      generic<class TKey, class TValue>
      String^ Region<TKey, TValue>::Name::get()
      {
        try
        {
          return marshal_as<String^>(m_nativeptr->get()->getName());
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }
      }

      generic<class TKey, class TValue>
      String^ Region<TKey, TValue>::FullPath::get()
      {
        try
        {
          return marshal_as<String^>(m_nativeptr->get()->getFullPath());
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }
      }

      generic<class TKey, class TValue>
      IRegion<TKey, TValue>^ Region<TKey, TValue>::ParentRegion::get()
      {
        _GF_MG_EXCEPTION_TRY2/* due to auto replace */

          try
          {
            auto parentRegion = m_nativeptr->get()->getParentRegion();
            return Region::Create(parentRegion);
          }
          finally
          {
            GC::KeepAlive(m_nativeptr);
          }

        _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */
      }

      generic<class TKey, class TValue>
      Apache::Geode::Client::RegionAttributes<TKey, TValue>^ Region<TKey, TValue>::Attributes::get()
      {
        _GF_MG_EXCEPTION_TRY2/* due to auto replace */

          try
          {
            auto nativeptr = m_nativeptr->get()->getAttributes();
            return Apache::Geode::Client::RegionAttributes<TKey, TValue>::Create(nativeptr);
          }
          finally
          {
            GC::KeepAlive(m_nativeptr);
          }

        _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */
      }

      generic<class TKey, class TValue>
      AttributesMutator<TKey, TValue>^ Region<TKey, TValue>::AttributesMutator::get()
      {
        _GF_MG_EXCEPTION_TRY2/* due to auto replace */

          try
          {
            auto am = m_nativeptr->get()->getAttributesMutator();
            return Apache::Geode::Client::AttributesMutator<TKey, TValue>::Create( am );
          }
          finally
          {
            GC::KeepAlive(m_nativeptr);
          }

        _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */
      }

      generic<class TKey, class TValue>
      Apache::Geode::Client::CacheStatistics^ Region<TKey, TValue>::Statistics::get()
      {
        _GF_MG_EXCEPTION_TRY2/* due to auto replace */

          try
          {
            auto nativeptr = m_nativeptr->get()->getStatistics();
            return Apache::Geode::Client::CacheStatistics::Create(nativeptr);
          }
          finally
          {
            GC::KeepAlive(m_nativeptr);
          }

        _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */
      }

      generic<class TKey, class TValue>
      IRegion<TKey, TValue>^ Region<TKey, TValue>::GetSubRegion(String^ path)
      {
        _GF_MG_EXCEPTION_TRY2/* due to auto replace */

          try
          {
            auto subRegion = m_nativeptr->get()->getSubregion(marshal_as<std::string>(path));
            return Region::Create(subRegion);
          }
          finally
          {
            GC::KeepAlive(m_nativeptr);
          }

        _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */
      }

      generic<class TKey, class TValue>
      IRegion<TKey, TValue>^ Region<TKey, TValue>::CreateSubRegion(String^ subRegionName,
                                                                    Apache::Geode::Client::RegionAttributes<TKey, TValue>^ attributes)
      {
        _GF_MG_EXCEPTION_TRY2/* due to auto replace */

          try
          {
            auto p_attrs = attributes->GetNative();
            return Region::Create(m_nativeptr->get()->createSubregion(marshal_as<std::string>(subRegionName), *p_attrs));
          }
          finally
          {
            GC::KeepAlive(m_nativeptr);
          }

        _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */

      }

      generic<class TKey, class TValue>
      System::Collections::Generic::ICollection<IRegion<TKey, TValue>^>^ Region<TKey, TValue>::SubRegions(bool recursive)
      {
        _GF_MG_EXCEPTION_TRY2/* due to auto replace */

          std::vector<std::shared_ptr<native::Region>> vsr;
        try
        {
          vsr = m_nativeptr->get()->subregions(recursive);
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }
        auto subRegions =
          gcnew array<IRegion<TKey, TValue>^>(static_cast<int>(vsr.size()));

        for (System::Int32 index = 0; index < subRegions->Length; index++)
        {
          auto subRegion = vsr[index];
          subRegions[index] = Region<TKey, TValue>::Create(subRegion);
        }
        auto collection = (System::Collections::Generic::ICollection<IRegion<TKey, TValue>^>^)subRegions;
        return collection;

        _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */
      }

      generic<class TKey, class TValue>
      RegionEntry<TKey, TValue>^ Region<TKey, TValue>::GetEntry(TKey key)
      {
        _GF_MG_EXCEPTION_TRY2/* due to auto replace */

        try
        {
          std::shared_ptr<native::CacheableKey> keyptr = Serializable::GetUnmanagedValueGeneric<TKey>(key);
          auto nativeptr = m_nativeptr->get()->getEntry(keyptr);
          return RegionEntry<TKey, TValue>::Create(nativeptr);
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }

        _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */
      }

      generic<class TKey, class TValue>
      System::Collections::Generic::ICollection<RegionEntry<TKey, TValue>^>^ Region<TKey, TValue>::GetEntries(bool recursive)
      {
        _GF_MG_EXCEPTION_TRY2/* due to auto replace */

		std::vector<std::shared_ptr<native::RegionEntry>> vc;
        try
        {
          vc = m_nativeptr->get()->entries(recursive);
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }
        auto entryarr = gcnew array<RegionEntry<TKey, TValue>^>(static_cast<int>(vc.size()));

        for (System::Int32 index = 0; index < entryarr->Length; index++)
        {
          auto& nativeptr = vc[index];
          entryarr[index] = RegionEntry<TKey, TValue>::Create(nativeptr);
        }
        auto collection = (System::Collections::Generic::ICollection<RegionEntry<TKey, TValue>^>^)entryarr;

        return collection;

        _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */

      }

      generic<class TKey, class TValue>
      IRegionService^ Region<TKey, TValue>::RegionService::get()
      {
        return CacheResolver::Lookup(&m_nativeptr->get()->getCache());
      }

      generic<class TKey, class TValue>
      bool Region<TKey, TValue>::ContainsValueForKey(TKey key)
      {
        _GF_MG_EXCEPTION_TRY2/* due to auto replace */

        try
        {
          std::shared_ptr<native::CacheableKey> keyptr = Serializable::GetUnmanagedValueGeneric<TKey>(key);
          return m_nativeptr->get()->containsValueForKey(keyptr);
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }

        _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */
      }

      generic<class TKey, class TValue>
      int Region<TKey, TValue>::Count::get()
      {
        _GF_MG_EXCEPTION_TRY2/* due to auto replace */
          try
          {
            return m_nativeptr->get()->size();
          }
          finally
          {
            GC::KeepAlive(m_nativeptr);
          }
        _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */
      }

      generic<class TKey, class TValue>
      void Region<TKey, TValue>::Clear()
      {
        Clear(nullptr);
      }

      generic<class TKey, class TValue>
      void Region<TKey, TValue>::Clear(Object^ callbackArg)
      {
        _GF_MG_EXCEPTION_TRY2/* due to auto replace */
        try
        {
          std::shared_ptr<native::Serializable> callbackptr = Serializable::GetUnmanagedValueGeneric<Object^>(callbackArg);
          m_nativeptr->get()->clear(callbackptr);
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }
        _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */
      }

      generic<class TKey, class TValue>
      void Region<TKey, TValue>::CopyTo(array<KeyValuePair<TKey, TValue>>^ toArray,
                                        int startIdx)
      {
        if (toArray == nullptr)
        {
          throw gcnew System::ArgumentNullException;
        }
        if (startIdx < 0)
        {
          throw gcnew System::ArgumentOutOfRangeException;
        }

        _GF_MG_EXCEPTION_TRY2/* due to auto replace */

		std::vector<std::shared_ptr<native::RegionEntry>> vc;
        try
        {
          vc = m_nativeptr->get()->entries(false);
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }

        if (toArray->Rank > 1 || (vc.size() > (toArray->Length - startIdx)))
        {
          throw gcnew System::ArgumentException;
        }

        for (System::Int32 index = 0; index < vc.size(); index++)
        {
          auto& nativeptr = vc[index];
          auto key = TypeRegistry::GetManagedValueGeneric<TKey>(nativeptr->getKey());
          auto val = TypeRegistry::GetManagedValueGeneric<TValue>(nativeptr->getValue());
          toArray[startIdx] = KeyValuePair<TKey, TValue>(key, val);
          ++startIdx;
        }

        _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */
      }

      generic<class TKey, class TValue>
      bool Region<TKey, TValue>::IsDestroyed::get()
      {
        try
        {
          return m_nativeptr->get()->isDestroyed();
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }
      }

      generic<class TKey, class TValue>
      void Region<TKey, TValue>::RegisterKeys(System::Collections::Generic::ICollection<TKey>^ keys)
      {
        RegisterKeys(keys, false, false);
      }

      generic<class TKey, class TValue>
      void Region<TKey, TValue>::RegisterKeys(System::Collections::Generic::ICollection<TKey>^ keys, bool isDurable, bool getInitialValues)
      {
        RegisterKeys(keys, isDurable, getInitialValues, true);
      }

      generic<class TKey, class TValue>
      void Region<TKey, TValue>::RegisterKeys(System::Collections::Generic::ICollection<TKey>^ keys,
                                              bool isDurable,
                                              bool getInitialValues,
                                              bool receiveValues)
      {
        if (keys != nullptr)
        {
          _GF_MG_EXCEPTION_TRY2/* due to auto replace */

            std::vector<std::shared_ptr<native::CacheableKey>> vecKeys;

          for each(TKey item in keys)
          {
            vecKeys.push_back(Serializable::GetUnmanagedValueGeneric<TKey>(item));
          }
          try
          {
            m_nativeptr->get()->registerKeys(vecKeys, isDurable, getInitialValues, receiveValues);
          }
          finally
          {
            GC::KeepAlive(m_nativeptr);
          }

          _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */
        }
      }

      generic<class TKey, class TValue>
      void Region<TKey, TValue>::UnregisterKeys(System::Collections::Generic::ICollection<TKey>^ keys)
      {
        if (keys != nullptr)
        {
          _GF_MG_EXCEPTION_TRY2/* due to auto replace */

            std::vector<std::shared_ptr<native::CacheableKey>> vecKeys;

          for each(TKey item in keys)
          {
            vecKeys.push_back(
              Serializable::GetUnmanagedValueGeneric<TKey>(item));
          }

          try
          {
            m_nativeptr->get()->unregisterKeys(vecKeys);
          }
          finally
          {
            GC::KeepAlive(m_nativeptr);
          }

          _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */
        }
      }

      generic<class TKey, class TValue>
      void Region<TKey, TValue>::RegisterAllKeys()
      {
        RegisterAllKeys(false, false);
      }

      generic<class TKey, class TValue>
      void Region<TKey, TValue>::RegisterAllKeys(bool isDurable)
      {
        RegisterAllKeys(isDurable, false);
      }

      generic<class TKey, class TValue>
      void Region<TKey, TValue>::RegisterAllKeys(bool isDurable,
                                                  bool getInitialValues)
      {
        RegisterAllKeys(isDurable, getInitialValues, true);
      }

      generic<class TKey, class TValue>
      void Region<TKey, TValue>::RegisterAllKeys(bool isDurable,
                                                  bool getInitialValues,
                                                  bool receiveValues)
      {
        _GF_MG_EXCEPTION_TRY2/* due to auto replace */
          
        try
        {
            m_nativeptr->get()->registerAllKeys(isDurable, getInitialValues, receiveValues);
        }
        finally
        {
            GC::KeepAlive(m_nativeptr);
        }

        _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */
      }

      generic<class TKey, class TValue>
      System::Collections::Generic::ICollection<TKey>^ Region<TKey, TValue>::GetInterestList()
      {
        _GF_MG_EXCEPTION_TRY2/* due to auto replace */

          std::vector<std::shared_ptr<native::CacheableKey>> vc;
        try
        {
          vc = m_nativeptr->get()->getInterestList();
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }
        auto keyarr = gcnew array<TKey>(static_cast<int>(vc.size()));
        for (System::Int32 index = 0; index < keyarr->Length; index++)
        {
          auto& nativeptr = vc[index];
          keyarr[index] = TypeRegistry::GetManagedValueGeneric<TKey>(nativeptr);
        }

        auto collectionlist = (System::Collections::Generic::ICollection<TKey>^)keyarr;
        return collectionlist;

        _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */
      }

      generic<class TKey, class TValue>
      System::Collections::Generic::ICollection<String^>^ Region<TKey, TValue>::GetInterestListRegex()
      {
        _GF_MG_EXCEPTION_TRY2/* due to auto replace */

          std::vector<std::shared_ptr<native::CacheableString>> vc;
        try
        {
          vc = m_nativeptr->get()->getInterestListRegex();
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }
        auto strarr = gcnew array<String^>(static_cast<int>(vc.size()));
        for (System::Int32 index = 0; index < strarr->Length; index++)
        {
          strarr[index] = marshal_as<String^>(vc[index]->value());
          //collectionlist[ index ] = Serializable::GetManagedValue<TValue>(nativeptr);
        }
        auto collectionlist = (System::Collections::Generic::ICollection<String^>^)strarr;
        return collectionlist;

        _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */
      }

      generic<class TKey, class TValue>
      void Region<TKey, TValue>::UnregisterAllKeys()
      {
        _GF_MG_EXCEPTION_TRY2/* due to auto replace */

          try
          {
            m_nativeptr->get()->unregisterAllKeys();
          }
          finally
          {
            GC::KeepAlive(m_nativeptr);
          }

        _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */

      }

      generic<class TKey, class TValue>
      void Region<TKey, TValue>::RegisterRegex(String^ regex)
      {
        RegisterRegex(regex, false, false);
      }

      generic<class TKey, class TValue>
      void Region<TKey, TValue>::RegisterRegex(String^ regex, bool isDurable)
      {
        RegisterRegex(regex, isDurable, false);
      }

      generic<class TKey, class TValue>
      void Region<TKey, TValue>::RegisterRegex(String^ regex, bool isDurable, bool getInitialValues)
      {
        RegisterRegex(regex, isDurable, getInitialValues, true);
      }

      generic<class TKey, class TValue>
      void Region<TKey, TValue>::RegisterRegex(String^ regex, bool isDurable,
          bool getInitialValues, bool receiveValues)
      {
        _GF_MG_EXCEPTION_TRY2/* due to auto replace */

        try
        {
          m_nativeptr->get()->registerRegex(marshal_as<std::string>(regex), isDurable,
            getInitialValues, receiveValues);
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }

        _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */
      }

      generic<class TKey, class TValue>
      void Region<TKey, TValue>::UnregisterRegex(String^ regex)
      {
        _GF_MG_EXCEPTION_TRY2/* due to auto replace */

        try
        {
          m_nativeptr->get()->unregisterRegex(marshal_as<std::string>(regex));
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }

        _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */

      }

      generic<class TKey, class TValue>
      generic<class TResult>
      ISelectResults<TResult>^ Region<TKey, TValue>::Query(String^ predicate)
      {
        return Query<TResult>( predicate, TimeUtils::DurationToTimeSpan(native::DEFAULT_QUERY_RESPONSE_TIMEOUT ));
      }

      generic<class TKey, class TValue>
      generic<class TResult>
      ISelectResults<TResult>^ Region<TKey, TValue>::Query(String^ predicate, TimeSpan timeout)
      {
        _GF_MG_EXCEPTION_TRY2/* due to auto replace */

          try
          {
            auto selectResults = m_nativeptr->get()->query(marshal_as<std::string>(predicate), TimeUtils::TimeSpanToDurationCeil<std::chrono::seconds>(timeout));
            if (auto resultptr = std::dynamic_pointer_cast<native::ResultSet>(selectResults))
            {
              return ResultSet<TResult>::Create(resultptr);
            }
            else if (auto structptr = std::dynamic_pointer_cast<native::StructSet>(selectResults))
            {
              return StructSet<TResult>::Create(structptr);
            }
            return nullptr;
          }
          finally
          {
            GC::KeepAlive(m_nativeptr);
          }

        _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */
      }

      generic<class TKey, class TValue>
      bool Region<TKey, TValue>::ExistsValue(String^ predicate)
      {
        return ExistsValue(predicate, TimeUtils::DurationToTimeSpan(native::DEFAULT_QUERY_RESPONSE_TIMEOUT));
      }

      generic<class TKey, class TValue>
      bool Region<TKey, TValue>::ExistsValue(String^ predicate, TimeSpan timeout)
      {
        _GF_MG_EXCEPTION_TRY2/* due to auto replace */

          try
          {
            return m_nativeptr->get()->existsValue(marshal_as<std::string>(predicate), TimeUtils::TimeSpanToDurationCeil<std::chrono::seconds>(timeout));
          }
          finally
          {
            GC::KeepAlive(m_nativeptr);
          }

        _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */
      }

      generic<class TKey, class TValue>
      Object^ Region<TKey, TValue>::SelectValue(String^ predicate)
      {
        return SelectValue(predicate, TimeUtils::DurationToTimeSpan(native::DEFAULT_QUERY_RESPONSE_TIMEOUT));
      }

      generic<class TKey, class TValue>
      Object^ Region<TKey, TValue>::SelectValue(String^ predicate, TimeSpan timeout)
      {
        _GF_MG_EXCEPTION_TRY2/* due to auto replace */

        try
        {
          auto nativeptr = m_nativeptr->get()->selectValue(marshal_as<std::string>(predicate), TimeUtils::TimeSpanToDurationCeil<std::chrono::seconds>(timeout));
          return TypeRegistry::GetManagedValueGeneric<Object^>(nativeptr);
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }

        _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */
      }

      generic<class TKey, class TValue>
      ISubscriptionService<TKey>^ Region<TKey, TValue>::GetSubscriptionService()
      {
        return (ISubscriptionService<TKey>^) this;
      }

      generic<class TKey, class TValue>
      IRegion<TKey, TValue>^ Region<TKey, TValue>::GetLocalView()
      {
        return gcnew LocalRegion<TKey, TValue>(GetNative());
      }
    }  // namespace Client
  }  // namespace Geode
}  // namespace Apache
