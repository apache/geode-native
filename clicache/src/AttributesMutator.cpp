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

#include "AttributesMutator.hpp"

#include "impl/ManagedCacheListener.hpp"
#include "impl/ManagedCacheLoader.hpp"
#include "impl/ManagedCacheWriter.hpp"
#include "impl/CacheLoader.hpp"
#include "impl/CacheWriter.hpp"
#include "impl/CacheListener.hpp"

using namespace System;

namespace Apache
{
  namespace Geode
  {
    namespace Client
    {
      namespace native = apache::geode::client;

      generic<class TKey, class TValue>
      System::Int32 AttributesMutator<TKey, TValue>::SetEntryIdleTimeout( System::Int32 idleTimeout )
      {
        try
        {
          return m_nativeptr->get()->setEntryIdleTimeout( idleTimeout );
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }
      }

      generic<class TKey, class TValue>
      ExpirationAction AttributesMutator<TKey, TValue>::SetEntryIdleTimeoutAction(
        ExpirationAction action )
      {
        try
        {
          return static_cast<ExpirationAction>(
            m_nativeptr->get()->setEntryIdleTimeoutAction(
              static_cast<native::ExpirationAction::Action>(action)));
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }
      }

      generic<class TKey, class TValue>
      System::Int32 AttributesMutator<TKey, TValue>::SetEntryTimeToLive( System::Int32 timeToLive )
      {
        try
        {
          return m_nativeptr->get()->setEntryTimeToLive( timeToLive );
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }
      }

      generic<class TKey, class TValue>
      ExpirationAction AttributesMutator<TKey, TValue>::SetEntryTimeToLiveAction(
        ExpirationAction action )
      {
        try
        {
          return static_cast<ExpirationAction>(
            m_nativeptr->get()->setEntryTimeToLiveAction(
              static_cast<native::ExpirationAction::Action>(action)));
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }
     }

      generic<class TKey, class TValue>
      System::Int32 AttributesMutator<TKey, TValue>::SetRegionIdleTimeout( System::Int32 idleTimeout )
      {
        try
        {
          return m_nativeptr->get()->setRegionIdleTimeout( idleTimeout );
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }
      }

      generic<class TKey, class TValue>
      ExpirationAction AttributesMutator<TKey, TValue>::SetRegionIdleTimeoutAction(
        ExpirationAction action )
      {
        try
        {
          return static_cast<ExpirationAction>(
            m_nativeptr->get()->setRegionIdleTimeoutAction(
              static_cast<native::ExpirationAction::Action>(action)));
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }
      }

      generic<class TKey, class TValue>
      System::Int32 AttributesMutator<TKey, TValue>::SetRegionTimeToLive( System::Int32 timeToLive )
      {
        try
        {
          return m_nativeptr->get()->setRegionTimeToLive( timeToLive );
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }
      }

      generic<class TKey, class TValue>
      ExpirationAction AttributesMutator<TKey, TValue>::SetRegionTimeToLiveAction(
        ExpirationAction action )
      {
        try
        {
          return static_cast<ExpirationAction>(
            m_nativeptr->get()->setRegionTimeToLiveAction(
              static_cast<native::ExpirationAction::Action>(action)));
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }
      }

      generic<class TKey, class TValue>
      System::UInt32 AttributesMutator<TKey, TValue>::SetLruEntriesLimit( System::UInt32 entriesLimit )
      {
        try
        {
          return m_nativeptr->get()->setLruEntriesLimit( entriesLimit );
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }
      }

      generic<class TKey, class TValue>
      void AttributesMutator<TKey, TValue>::SetCacheListener( ICacheListener<TKey, TValue>^ cacheListener )
      {
        std::shared_ptr<native::CacheListener> listenerptr;
        if (cacheListener != nullptr)
        {
          auto clg = gcnew CacheListenerGeneric<TKey, TValue>();
          clg->SetCacheListener(cacheListener);
          listenerptr = std::shared_ptr<native::ManagedCacheListenerGeneric>( new native::ManagedCacheListenerGeneric(cacheListener) );
          ((native::ManagedCacheListenerGeneric*)listenerptr.get())->setptr(clg);
        }
        try
        {
          m_nativeptr->get()->setCacheListener( listenerptr );
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }
      }

      generic<class TKey, class TValue>
      void AttributesMutator<TKey, TValue>::SetCacheListener( String^ libPath,
        String^ factoryFunctionName )
      {
        throw gcnew System::NotSupportedException;
      }

      generic<class TKey, class TValue>
      void AttributesMutator<TKey, TValue>::SetCacheLoader( ICacheLoader<TKey, TValue>^ cacheLoader )
      {
        std::shared_ptr<native::CacheLoader> loaderptr;
        if (cacheLoader != nullptr)
        {
          auto clg = gcnew CacheLoaderGeneric<TKey, TValue>();
          clg->SetCacheLoader(cacheLoader);
          loaderptr = std::shared_ptr<native::ManagedCacheLoaderGeneric>( new native::ManagedCacheLoaderGeneric(cacheLoader) );
          ((native::ManagedCacheLoaderGeneric*)loaderptr.get())->setptr(clg);
        }
        try
        {
          m_nativeptr->get()->setCacheLoader( loaderptr );
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }
      }

      generic<class TKey, class TValue>
      void AttributesMutator<TKey, TValue>::SetCacheLoader( String^ libPath,
        String^ factoryFunctionName )
      {
        throw gcnew System::NotSupportedException;
      }

      generic<class TKey, class TValue>
      void AttributesMutator<TKey, TValue>::SetCacheWriter( ICacheWriter<TKey, TValue>^ cacheWriter )
      {
        std::shared_ptr<native::CacheWriter> writerptr;
        if (cacheWriter != nullptr)
        {
          auto cwg = gcnew CacheWriterGeneric<TKey, TValue>();
          cwg->SetCacheWriter(cacheWriter);
          writerptr = std::shared_ptr<native::ManagedCacheWriterGeneric>( new native::ManagedCacheWriterGeneric(cacheWriter) );
          ((native::ManagedCacheWriterGeneric*)writerptr.get())->setptr(cwg);
        }
        try
        {
          m_nativeptr->get()->setCacheWriter( writerptr );
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }
      }

      generic<class TKey, class TValue>
      void AttributesMutator<TKey, TValue>::SetCacheWriter( String^ libPath,
        String^ factoryFunctionName )
      {
        throw gcnew System::NotSupportedException;
      }
    }  // namespace Client
  }  // namespace Geode
}  // namespace Apache
