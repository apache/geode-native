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
#include "geode/Region.hpp"
#include "geode/Cache.hpp"
#include "end_native.hpp"

#include "LocalRegion.hpp"
#include "Cache.hpp"
#include "CacheStatistics.hpp"
#include "AttributesMutator.hpp"
#include "RegionEntry.hpp"
#include "impl/AuthenticatedCache.hpp"
#include "impl/SafeConvert.hpp"

using namespace System;

namespace Apache
{
  namespace Geode
  {
    namespace Client
    {

      generic<class TKey, class TValue>
      TValue LocalRegion<TKey, TValue>::Get(TKey key, Object^ callbackArg)
      {
        native::CacheableKeyPtr keyptr = Serializable::GetUnmanagedValueGeneric<TKey>( key, m_nativeptr->get()->getCache().get() );
        auto nativeptr= this->getRegionEntryValue(keyptr);
        if (nativeptr == nullptr)
        {
          throw gcnew KeyNotFoundException("The given key was not present in the region");
        }
        TValue returnVal = Serializable::GetManagedValueGeneric<TValue>( nativeptr );
        return returnVal;        
      }     

      generic<class TKey, class TValue>
      apache::geode::client::SerializablePtr LocalRegion<TKey, TValue>::getRegionEntryValue(apache::geode::client::CacheableKeyPtr& keyptr)
      {
        _GF_MG_EXCEPTION_TRY2/* due to auto replace */
          try
          {
            if (auto entryPtr = m_nativeptr->get()->getEntry(keyptr)) {
              return entryPtr->getValue();
            }
            else {
              return nullptr;
            }
          }
          finally
          {
            GC::KeepAlive(m_nativeptr);
          }
        _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */
      }

      generic<class TKey, class TValue>
      void LocalRegion<TKey, TValue>::Put(TKey key, TValue value, Object^ callbackArg)
      {
        _GF_MG_EXCEPTION_TRY2/* due to auto replace */

        try
        {
          native::CacheableKeyPtr keyptr = Serializable::GetUnmanagedValueGeneric<TKey>( key, m_nativeptr->get()->getCache().get() );
          native::CacheablePtr valueptr = Serializable::GetUnmanagedValueGeneric<TValue>( value, m_nativeptr->get()->getCache().get() );        
          native::UserDataPtr callbackptr = Serializable::GetUnmanagedValueGeneric<Object^>( callbackArg, m_nativeptr->get()->getCache().get() );
          m_nativeptr->get()->localPut( keyptr, valueptr, callbackptr );
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }

        _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */
      }

      generic<class TKey, class TValue>
      TValue LocalRegion<TKey, TValue>::default::get(TKey key)
      { 
        native::CacheableKeyPtr keyptr = Serializable::GetUnmanagedValueGeneric<TKey>( key, m_nativeptr->get()->getCache().get() );
        auto nativeptr = this->getRegionEntryValue(keyptr);
        if (nativeptr == nullptr)
        {
          throw gcnew KeyNotFoundException("The given key was not present in the region");
        }
        TValue returnVal = Serializable::GetManagedValueGeneric<TValue>( nativeptr );
        return returnVal;
      }

      generic<class TKey, class TValue>      
      void LocalRegion<TKey, TValue>::default::set(TKey key, TValue value)
      {
        _GF_MG_EXCEPTION_TRY2/* due to auto replace */

        try
        {
          native::CacheableKeyPtr keyptr = Serializable::GetUnmanagedValueGeneric<TKey>( key, m_nativeptr->get()->getCache().get() );
          native::CacheablePtr valueptr = Serializable::GetUnmanagedValueGeneric<TValue>( value, m_nativeptr->get()->getCache().get() );
          m_nativeptr->get()->localPut( keyptr, valueptr );
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }

        _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */
      }

      generic<class TKey, class TValue>
      System::Collections::Generic::IEnumerator<KeyValuePair<TKey,TValue>>^ 
        LocalRegion<TKey, TValue>::GetEnumerator()
      {
        apache::geode::client::VectorOfRegionEntry vc;

        _GF_MG_EXCEPTION_TRY2/* due to auto replace */

          try
          {
            m_nativeptr->get()->entries( vc, false );
          }
          finally
          {
            GC::KeepAlive(m_nativeptr);
          }

        _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */ 

        auto toArray = gcnew array<KeyValuePair<TKey,TValue>>(static_cast<int>(vc.size()));
        for( System::Int32 index = 0; index < vc.size( ); index++ )
        {
          auto nativeptr = vc[ index ];  
          TKey key = Serializable::GetManagedValueGeneric<TKey> (nativeptr->getKey());
          TValue val = Serializable::GetManagedValueGeneric<TValue> (nativeptr->getValue());
          toArray[ index ] = KeyValuePair<TKey,TValue>(key, val);           
        }                      
        return ((System::Collections::Generic::IEnumerable<KeyValuePair<TKey,TValue>>^)toArray)->GetEnumerator();
      }

      generic<class TKey, class TValue>
      System::Collections::IEnumerator^ 
        LocalRegion<TKey, TValue>::GetEnumeratorOld()
      {
        apache::geode::client::VectorOfRegionEntry vc;

        _GF_MG_EXCEPTION_TRY2/* due to auto replace */

          try
          {
            m_nativeptr->get()->entries( vc, false );
          }
          finally
          {
            GC::KeepAlive(m_nativeptr);
          }

        _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */

        auto toArray = gcnew array<Object^>(static_cast<int>(vc.size()));
        for( System::Int32 index = 0; index < vc.size( ); index++ )
        {
          auto nativeptr = vc[ index ];                       
          TKey key = Serializable::GetManagedValueGeneric<TKey> (nativeptr->getKey());
          TValue val = Serializable::GetManagedValueGeneric<TValue> (nativeptr->getValue());            
          toArray[ index ] = KeyValuePair<TKey,TValue>(key, val);           
        }
        return ((System::Collections::Generic::IEnumerable<Object^>^)toArray)->GetEnumerator();        
      }


      generic<class TKey, class TValue>
      bool LocalRegion<TKey, TValue>::AreValuesEqual(apache::geode::client::CacheablePtr& val1, apache::geode::client::CacheablePtr& val2)
      {
        if ( val1 == nullptr && val2 == nullptr )
        {
          return true;
        }
        else if ((val1 == nullptr && val2 != nullptr) || (val1 != nullptr && val2 == nullptr))
        {
          return false;
        }
        else if( val1 != nullptr && val2 != nullptr )
        {
          if (val1->classId() != val2->classId() || val1->typeId() != val2->typeId())
          {
            return false;
          }
          std::unique_ptr<apache::geode::client::DataOutput> out1 = m_nativeptr->get_shared_ptr()->getCache()->createDataOutput();
          std::unique_ptr<apache::geode::client::DataOutput> out2 = m_nativeptr->get_shared_ptr()->getCache()->createDataOutput();
          val1->toData(*out1);
          val2->toData(*out2);
          if ( out1->getBufferLength() != out2->getBufferLength() )
          {
            return false;
          }
          else if (memcmp(out1->getBuffer(), out2->getBuffer(), out1->getBufferLength()) != 0)
          {
            return false;
          }
          return true;
        }
        return false;
      }

      generic<class TKey, class TValue> 
      bool LocalRegion<TKey, TValue>::Contains(KeyValuePair<TKey,TValue> keyValuePair) 
      { 
        auto keyptr = Serializable::GetUnmanagedValueGeneric<TKey>( keyValuePair.Key, m_nativeptr->get()->getCache().get() ); 
        auto nativeptr = this->getRegionEntryValue(keyptr);
        //This means that key is not present.
        if (nativeptr == nullptr) {
          return false;
        }        
        TValue value = Serializable::GetManagedValueGeneric<TValue>(nativeptr);
        return ((Object^)value)->Equals(keyValuePair.Value);
      } 

      generic<class TKey, class TValue>
      bool LocalRegion<TKey, TValue>::ContainsKey(TKey key)
      {
        _GF_MG_EXCEPTION_TRY2/* due to auto replace */

        try
        {
          auto keyptr = Serializable::GetUnmanagedValueGeneric<TKey>( key, m_nativeptr->get()->getCache().get() );          
          return m_nativeptr->get()->containsKey(keyptr);
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }

        _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */
      }

      generic<class TKey, class TValue>
      bool LocalRegion<TKey, TValue>::TryGetValue(TKey key, TValue %val)
      {        
        auto keyptr = Serializable::GetUnmanagedValueGeneric<TKey>( key, m_nativeptr->get()->getCache().get() );
        auto nativeptr = this->getRegionEntryValue(keyptr);
        if (nativeptr == nullptr) {            
          val = TValue();
          return false;
        }
        else {
          val = Serializable::GetManagedValueGeneric<TValue>( nativeptr );
          return true;
        }          
      }      

      generic<class TKey, class TValue>
      System::Collections::Generic::ICollection<TKey>^ LocalRegion<TKey, TValue>::Keys::get()
      {
        _GF_MG_EXCEPTION_TRY2/* due to auto replace */

        apache::geode::client::VectorOfCacheableKey vc;
        try
        {
          m_nativeptr->get()->keys(vc);
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }

        auto keyarr =  gcnew array<TKey>( static_cast<int>(vc.size( )) );
        for( System::Int32 index = 0; index < vc.size( ); index++ )
        {            
          auto& nativeptr = vc[ index ];
          keyarr[ index ] = Serializable::GetManagedValueGeneric<TKey>(nativeptr);
        }
        auto collectionlist = (System::Collections::Generic::ICollection<TKey>^)keyarr;
        return collectionlist;

        _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */
      }

      generic<class TKey, class TValue>
      System::Collections::Generic::ICollection<TValue>^ LocalRegion<TKey, TValue>::Values::get()
      {
        _GF_MG_EXCEPTION_TRY2/* due to auto replace */

          apache::geode::client::VectorOfCacheable vc;
          try
          {
            m_nativeptr->get()->values( vc );
          }
          finally
          {
            GC::KeepAlive(m_nativeptr);
          }

          //List<TValue>^ collectionlist = gcnew List<TValue>(vc.size());
          auto valarr = gcnew array<TValue>( static_cast<int>(vc.size( )) );
          for( System::Int32 index = 0; index < vc.size( ); index++ )
          {
            auto& nativeptr = vc[ index ];            
            valarr[ index ] = Serializable::GetManagedValueGeneric<TValue>(nativeptr);
          }
          auto collectionlist = (System::Collections::Generic::ICollection<TValue>^)valarr;
          return collectionlist;

        _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */
      }

      generic<class TKey, class TValue>
      void LocalRegion<TKey, TValue>::Add(TKey key, TValue value)
      {
        _GF_MG_EXCEPTION_TRY2/* due to auto replace */

          try
          {
            native::CacheableKeyPtr keyptr = Serializable::GetUnmanagedValueGeneric<TKey>( key, m_nativeptr->get()->getCache().get() );
            native::CacheablePtr valueptr = Serializable::GetUnmanagedValueGeneric<TValue>( value, m_nativeptr->get()->getCache().get() );
            m_nativeptr->get()->localCreate( keyptr, valueptr );
          }
          finally
          {
            GC::KeepAlive(m_nativeptr);
          }

        _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */
      }

      generic<class TKey, class TValue>
      void LocalRegion<TKey, TValue>::Add(KeyValuePair<TKey, TValue> keyValuePair)
      {
        _GF_MG_EXCEPTION_TRY2/* due to auto replace */

          try
          {
            native::CacheableKeyPtr keyptr = Serializable::GetUnmanagedValueGeneric<TKey>( keyValuePair.Key, m_nativeptr->get()->getCache().get() );
            native::CacheablePtr valueptr = Serializable::GetUnmanagedValueGeneric<TValue>( keyValuePair.Value, m_nativeptr->get()->getCache().get() );
            m_nativeptr->get()->localCreate( keyptr, valueptr );
          }
          finally
          {
            GC::KeepAlive(m_nativeptr);
          }

       _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */
      }

      generic<class TKey, class TValue>
      void LocalRegion<TKey, TValue>::Add(TKey key, TValue value, Object^ callbackArg)
      {
          _GF_MG_EXCEPTION_TRY2/* due to auto replace */

          try
          {
            native::CacheableKeyPtr keyptr = Serializable::GetUnmanagedValueGeneric<TKey>( key, m_nativeptr->get()->getCache().get() );
            native::CacheablePtr valueptr = Serializable::GetUnmanagedValueGeneric<TValue>( value, m_nativeptr->get()->getCache().get() );          
            native::UserDataPtr callbackptr = Serializable::GetUnmanagedValueGeneric<Object^>( callbackArg, m_nativeptr->get()->getCache().get() );
            m_nativeptr->get()->localCreate( keyptr, valueptr, callbackptr );
          }
          finally
          {
            GC::KeepAlive(m_nativeptr);
          }

         _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */
      }

      generic<class TKey, class TValue>
      bool LocalRegion<TKey, TValue>::Remove(TKey key)
      {
        _GF_MG_EXCEPTION_TRY2/* due to auto replace */
    
          try
          {
            native::CacheableKeyPtr keyptr = Serializable::GetUnmanagedValueGeneric<TKey>(key, m_nativeptr->get()->getCache().get());
            m_nativeptr->get()->localDestroy(keyptr);
            return true;
          }
          catch (apache::geode::client::EntryNotFoundException /*ex*/)
          {
            return false;
          }
          finally
          {
            GC::KeepAlive(m_nativeptr);
          }

        _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */
        
      }

      generic<class TKey, class TValue>
      bool LocalRegion<TKey, TValue>::Remove( TKey key, Object^ callbackArg )
      {
         _GF_MG_EXCEPTION_TRY2/* due to auto replace */
           try
           {
             native::CacheableKeyPtr keyptr = Serializable::GetUnmanagedValueGeneric<TKey>(key, m_nativeptr->get()->getCache().get());
             native::UserDataPtr callbackptr = Serializable::GetUnmanagedValueGeneric<Object^>(callbackArg, m_nativeptr->get()->getCache().get());
             m_nativeptr->get()->localDestroy(keyptr, callbackptr);
             return true;
           }
           catch (apache::geode::client::EntryNotFoundException /*ex*/)
           {
             return false;
           }
           finally
           {
             GC::KeepAlive(m_nativeptr);
           }
        _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */
      }

      generic<class TKey, class TValue>
      bool LocalRegion<TKey, TValue>::Remove(KeyValuePair<TKey,TValue> keyValuePair)
      {
        _GF_MG_EXCEPTION_TRY2/* due to auto replace */

          try
          {
            native::CacheableKeyPtr keyptr = Serializable::GetUnmanagedValueGeneric<TKey>( keyValuePair.Key, m_nativeptr->get()->getCache().get() );
            native::CacheablePtr valueptr = Serializable::GetUnmanagedValueGeneric<TValue>( keyValuePair.Value, m_nativeptr->get()->getCache().get() );
            return m_nativeptr->get()->localRemove(keyptr, valueptr);
          }
          finally
          {
            GC::KeepAlive(m_nativeptr);
          }

         _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */
      }

      generic<class TKey, class TValue>
      bool LocalRegion<TKey, TValue>::Remove(TKey key, TValue value, Object^ callbackArg)
      {
        _GF_MG_EXCEPTION_TRY2/* due to auto replace */

          try
          {
            native::CacheableKeyPtr keyptr = Serializable::GetUnmanagedValueGeneric<TKey>(key, m_nativeptr->get()->getCache().get());
            native::CacheablePtr valueptr = Serializable::GetUnmanagedValueGeneric<TValue>(value, m_nativeptr->get()->getCache().get());
            native::UserDataPtr callbackptr = Serializable::GetUnmanagedValueGeneric<Object^>(callbackArg, m_nativeptr->get()->getCache().get());
            return m_nativeptr->get()->localRemove(keyptr, valueptr, callbackptr);
          }
          finally
          {
            GC::KeepAlive(m_nativeptr);
          }

         _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */
      }

      generic<class TKey, class TValue>
      void LocalRegion<TKey, TValue>::InvalidateRegion()
      {
        _GF_MG_EXCEPTION_TRY2/* due to auto replace */

          InvalidateRegion( nullptr );

        _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */
      }

      generic<class TKey, class TValue>
      void LocalRegion<TKey, TValue>::InvalidateRegion(Object^ callbackArg)
      {
        _GF_MG_EXCEPTION_TRY2/* due to auto replace */
                    
          try
          {
            native::UserDataPtr callbackptr = Serializable::GetUnmanagedValueGeneric<Object^>( callbackArg, m_nativeptr->get()->getCache().get() );
            m_nativeptr->get()->localInvalidateRegion( callbackptr );
          }
          finally
          {
            GC::KeepAlive(m_nativeptr);
          }
      
        _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */
      }

      generic<class TKey, class TValue>
      void LocalRegion<TKey, TValue>::DestroyRegion()
      {
        _GF_MG_EXCEPTION_TRY2/* due to auto replace */

          DestroyRegion( nullptr );

        _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */
      }

      generic<class TKey, class TValue>
      void LocalRegion<TKey, TValue>::DestroyRegion(Object^ callbackArg)
      {
        _GF_MG_EXCEPTION_TRY2/* due to auto replace */          
          try
          {
            native::UserDataPtr callbackptr = Serializable::GetUnmanagedValueGeneric<Object^>( callbackArg, m_nativeptr->get()->getCache().get() );
            m_nativeptr->get()->localDestroyRegion( callbackptr );
          }
          finally
          {
            GC::KeepAlive(m_nativeptr);
          }
        _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */
      }

      generic<class TKey, class TValue>
      void LocalRegion<TKey, TValue>::Invalidate(TKey key)
      {
        _GF_MG_EXCEPTION_TRY2/* due to auto replace */

         Invalidate(key, nullptr);

        _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */

      }

      generic<class TKey, class TValue>
      void LocalRegion<TKey, TValue>::Invalidate(TKey key, Object^ callbackArg)
      {
        _GF_MG_EXCEPTION_TRY2/* due to auto replace */

          try
          {
            native::CacheableKeyPtr keyptr = Serializable::GetUnmanagedValueGeneric<TKey>( key, m_nativeptr->get()->getCache().get() );          
            native::UserDataPtr callbackptr = Serializable::GetUnmanagedValueGeneric<Object^>( callbackArg, m_nativeptr->get()->getCache().get() );            
            m_nativeptr->get()->localInvalidate( keyptr, callbackptr );
          }
          finally
          {
            GC::KeepAlive(m_nativeptr);
          }

        _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */
      }

      generic<class TKey, class TValue>
      void LocalRegion<TKey, TValue>::PutAll(System::Collections::Generic::IDictionary<TKey, TValue>^ map)
      {
        throw gcnew System::NotSupportedException;
      }

      generic<class TKey, class TValue>
      void LocalRegion<TKey, TValue>::PutAll(System::Collections::Generic::IDictionary<TKey, TValue>^ map, int timeout)
      {
        throw gcnew System::NotSupportedException;
      }

      generic<class TKey, class TValue>
      void LocalRegion<TKey, TValue>::PutAll(System::Collections::Generic::IDictionary<TKey, TValue>^ map, int timeout, Object^ callbackArg)
      {
        throw gcnew System::NotSupportedException;
      }

      generic<class TKey, class TValue>
      void LocalRegion<TKey, TValue>::GetAll(System::Collections::Generic::ICollection<TKey>^ keys, 
        System::Collections::Generic::IDictionary<TKey, TValue>^ values, 
        System::Collections::Generic::IDictionary<TKey, System::Exception^>^ exceptions)
      {
        throw gcnew System::NotSupportedException;      
      }

      generic<class TKey, class TValue>
      void LocalRegion<TKey, TValue>::GetAll(System::Collections::Generic::ICollection<TKey>^ keys, 
        System::Collections::Generic::IDictionary<TKey, TValue>^ values, 
        System::Collections::Generic::IDictionary<TKey, System::Exception^>^ exceptions, 
        bool addToLocalCache)
      {    
        throw gcnew System::NotSupportedException;
      }

      generic<class TKey, class TValue>
      void LocalRegion<TKey, TValue>::GetAll(System::Collections::Generic::ICollection<TKey>^ keys, 
        System::Collections::Generic::IDictionary<TKey, TValue>^ values, 
        System::Collections::Generic::IDictionary<TKey, System::Exception^>^ exceptions, 
        bool addToLocalCache, Object^ callbackArg)
      {    
        throw gcnew System::NotSupportedException;
      }
      
      generic<class TKey, class TValue>
      void LocalRegion<TKey, TValue>::RemoveAll(System::Collections::Generic::ICollection<TKey>^ keys)
      {
        throw gcnew System::NotSupportedException;
      }
      
      generic<class TKey, class TValue>
      void LocalRegion<TKey, TValue>::RemoveAll(System::Collections::Generic::ICollection<TKey>^ keys,
            Object^ callbackArg)
      {
        throw gcnew System::NotSupportedException;
      }

      generic<class TKey, class TValue>
      String^ LocalRegion<TKey, TValue>::Name::get()
      { 
        try
        {
          return ManagedString::Get( m_nativeptr->get()->getName( ) );
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        } 
      } 

      generic<class TKey, class TValue>
      String^ LocalRegion<TKey, TValue>::FullPath::get()
      { 
        try
        {
          return ManagedString::Get( m_nativeptr->get()->getFullPath( ) );
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        } 
      } 

      generic<class TKey, class TValue>
      IRegion<TKey, TValue>^ LocalRegion<TKey, TValue>::ParentRegion::get()
      {
        _GF_MG_EXCEPTION_TRY2/* due to auto replace */

          try
          {
            auto parentRegion = m_nativeptr->get()->getParentRegion( );
            auto region = Region<TKey, TValue>::Create( parentRegion );
            if (region == nullptr) {
              return nullptr;
            }
            return region->GetLocalView();
          }
          finally
          {
            GC::KeepAlive(m_nativeptr);
          }
        _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */
      }

      generic<class TKey, class TValue>
      Apache::Geode::Client::RegionAttributes<TKey, TValue>^ LocalRegion<TKey, TValue>::Attributes::get()
      { 
        _GF_MG_EXCEPTION_TRY2/* due to auto replace */

        try
        {
          return Apache::Geode::Client::RegionAttributes<TKey, TValue>::Create(m_nativeptr->get()->getAttributes());
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }

        _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */
      } 

      generic<class TKey, class TValue>      
      AttributesMutator<TKey, TValue>^ LocalRegion<TKey, TValue>::AttributesMutator::get()
      {
        _GF_MG_EXCEPTION_TRY2/* due to auto replace */

          try
          {
            return Apache::Geode::Client::AttributesMutator<TKey, TValue>::Create(m_nativeptr->get()->getAttributesMutator());
          }
          finally
          {
            GC::KeepAlive(m_nativeptr);
          }

        _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */
      }

      generic<class TKey, class TValue>
			Apache::Geode::Client::CacheStatistics^ LocalRegion<TKey, TValue>::Statistics::get()
      {
        _GF_MG_EXCEPTION_TRY2/* due to auto replace */

          try
          {
            return Apache::Geode::Client::CacheStatistics::Create(m_nativeptr->get()->getStatistics());
          }
          finally
          {
            GC::KeepAlive(m_nativeptr);
          }

        _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */
      }

      generic<class TKey, class TValue>
      IRegion<TKey, TValue>^ LocalRegion<TKey, TValue>::GetSubRegion( String^ path )
      {
        _GF_MG_EXCEPTION_TRY2/* due to auto replace */

          try
          {
            ManagedString mg_path(path);
            auto nativeptr = m_nativeptr->get()->getSubregion(mg_path.CharPtr);
            auto region = Region<TKey, TValue>::Create(nativeptr);
            if (region == nullptr) {
              return nullptr;
            }
            return region->GetLocalView();
          }
          finally
          {
            GC::KeepAlive(m_nativeptr);
          }

        _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */
      }

      generic<class TKey, class TValue>
      IRegion<TKey, TValue>^ LocalRegion<TKey, TValue>::CreateSubRegion( String^ subRegionName, 
        Apache::Geode::Client::RegionAttributes<TKey, TValue>^ attributes)
      {
        _GF_MG_EXCEPTION_TRY2/* due to auto replace */

          try
          {
            ManagedString mg_subregionName(subRegionName);
            return Region<TKey, TValue>::Create(m_nativeptr->get()->createSubregion(
              mg_subregionName.CharPtr, __nullptr))->GetLocalView();
          }
          finally
          {
            GC::KeepAlive(m_nativeptr);
          }

        _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */

      }

      generic<class TKey, class TValue>
      System::Collections::Generic::ICollection<IRegion<TKey, TValue>^>^ LocalRegion<TKey, TValue>::SubRegions( bool recursive )
      {
        _GF_MG_EXCEPTION_TRY2/* due to auto replace */

          apache::geode::client::VectorOfRegion vsr;
          try
          {
            m_nativeptr->get()->subregions( recursive, vsr );
          }
          finally
          {
            GC::KeepAlive(m_nativeptr);
          }
          array<IRegion<TKey, TValue>^>^ subRegions =
            gcnew array<IRegion<TKey, TValue>^>( static_cast<int>(vsr.size( )) );

          for( System::Int32 index = 0; index < vsr.size( ); index++ )
          {
            auto nativeptr = vsr[ index ];
            subRegions[ index ] = Region<TKey, TValue>::Create( nativeptr )->GetLocalView();
          }
          auto collection = (System::Collections::Generic::ICollection<IRegion<TKey, TValue>^>^)subRegions;
          return collection;

        _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */
      }

      generic<class TKey, class TValue>
      RegionEntry<TKey, TValue>^ LocalRegion<TKey, TValue>::GetEntry( TKey key )
      {
        _GF_MG_EXCEPTION_TRY2/* due to auto replace */

          try
          {
            native::CacheableKeyPtr keyptr = Serializable::GetUnmanagedValueGeneric<TKey>(key, m_nativeptr->get()->getCache().get());
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
      System::Collections::Generic::ICollection<RegionEntry<TKey, TValue>^>^ LocalRegion<TKey, TValue>::GetEntries(bool recursive)
      {
         _GF_MG_EXCEPTION_TRY2/* due to auto replace */

          apache::geode::client::VectorOfRegionEntry vc;
          try
          {
            m_nativeptr->get()->entries( vc, recursive );
          }
          finally
          {
            GC::KeepAlive(m_nativeptr);
          }          
          auto entryarr = gcnew array<RegionEntry<TKey, TValue>^>( static_cast<int>(vc.size( )) );

          for( System::Int32 index = 0; index < vc.size( ); index++ )
          {
            auto nativeptr = vc[ index ] ;
            entryarr[ index ] = RegionEntry<TKey, TValue>::Create( nativeptr );
          }
          auto collection = (System::Collections::Generic::ICollection<RegionEntry<TKey, TValue>^>^)entryarr;
          return collection;

         _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */
        
      }

      generic<class TKey, class TValue>
      IRegionService^ LocalRegion<TKey, TValue>::RegionService::get()
      {        
        _GF_MG_EXCEPTION_TRY2/* due to auto replace */

          try
          {
            auto regionService = m_nativeptr->get()->getRegionService();
            if (auto realCache = std::dynamic_pointer_cast<apache::geode::client::Cache>(regionService))
            {
              return Apache::Geode::Client::Cache::Create(realCache);
            }
            else
            {
              return Apache::Geode::Client::AuthenticatedCache::Create(regionService);
            }
          }
          finally
          {
            GC::KeepAlive(m_nativeptr);
          }
          
        _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */
      }

      generic<class TKey, class TValue>
      bool LocalRegion<TKey, TValue>::ContainsValueForKey( TKey key )
      {
         _GF_MG_EXCEPTION_TRY2/* due to auto replace */

           try
           {
             return m_nativeptr->get()->containsValueForKey(Serializable::GetUnmanagedValueGeneric<TKey>(key, m_nativeptr->get()->getCache().get()));
           }
           finally
           {
             GC::KeepAlive(m_nativeptr);
           }

         _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */
      }

      generic<class TKey, class TValue>
      int LocalRegion<TKey, TValue>::Count::get()
      {
        try
        {
          return m_nativeptr->get()->size();
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }
      }

      generic<class TKey, class TValue>
      void LocalRegion<TKey, TValue>::Clear()
      {
        Clear(nullptr);
      }

      generic<class TKey, class TValue>
      void LocalRegion<TKey, TValue>::Clear(Object^ callbackArg)
      {
        _GF_MG_EXCEPTION_TRY2/* due to auto replace */        
          try
          {
            m_nativeptr->get()->localClear(Serializable::GetUnmanagedValueGeneric<Object^>( callbackArg, m_nativeptr->get()->getCache().get() ) );
          }
          finally
          {
            GC::KeepAlive(m_nativeptr);
          }
        _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */
      }


      generic<class TKey, class TValue>
      void LocalRegion<TKey, TValue>::CopyTo(array<KeyValuePair<TKey,TValue>>^ toArray,
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

        apache::geode::client::VectorOfRegionEntry vc;
        try
        {
          m_nativeptr->get()->entries( vc, false );
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }        

        if (toArray->Rank > 1 || (vc.size() > (toArray->Length - startIdx)))
        {
          throw gcnew System::ArgumentException;
        }          

        for( System::Int32 index = 0; index < vc.size( ); index++ )
        {
          apache::geode::client::RegionEntryPtr nativeptr =  vc[ index ];                       
          TKey key = Serializable::GetManagedValueGeneric<TKey> (nativeptr->getKey());
          TValue val = Serializable::GetManagedValueGeneric<TValue> (nativeptr->getValue());            
          toArray[ startIdx ] = KeyValuePair<TKey,TValue>(key, val);
          ++startIdx;
        }               

        _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */
      }

      generic<class TKey, class TValue>
      bool LocalRegion<TKey, TValue>::IsDestroyed::get()
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
      generic<class TResult>
      ISelectResults<TResult>^ LocalRegion<TKey, TValue>::Query( String^ predicate )
      {
        throw gcnew System::NotSupportedException;
      }

      generic<class TKey, class TValue>
      generic<class TResult>
      ISelectResults<TResult>^ LocalRegion<TKey, TValue>::Query( String^ predicate, System::UInt32 timeout )
      {
        throw gcnew System::NotSupportedException;
      }

      generic<class TKey, class TValue>
      bool LocalRegion<TKey, TValue>::ExistsValue( String^ predicate )
      {
        throw gcnew System::NotSupportedException;
      }

      generic<class TKey, class TValue>
      bool LocalRegion<TKey, TValue>::ExistsValue( String^ predicate, System::UInt32 timeout )
      {
        throw gcnew System::NotSupportedException;
      }

      generic<class TKey, class TValue>
      Object^ LocalRegion<TKey, TValue>::SelectValue( String^ predicate )
      {
        throw gcnew System::NotSupportedException;
      }

      generic<class TKey, class TValue>
      Object^ LocalRegion<TKey, TValue>::SelectValue( String^ predicate, System::UInt32 timeout )
      {
        throw gcnew System::NotSupportedException;
      }

      generic<class TKey, class TValue>
      ISubscriptionService<TKey>^ LocalRegion<TKey, TValue>::GetSubscriptionService()
      {
        throw gcnew System::NotSupportedException;
      }

      generic<class TKey, class TValue>
      IRegion<TKey, TValue>^ LocalRegion<TKey, TValue>::GetLocalView()
      {
        throw gcnew System::NotSupportedException;
      }
    }  // namespace Client
  }  // namespace Geode
}  // namespace Apache
