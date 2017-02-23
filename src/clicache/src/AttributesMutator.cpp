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

//#include "geode_includes.hpp"
#include "AttributesMutator.hpp"
//#include "Region.hpp"
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

      generic<class TKey, class TValue>
      System::Int32 AttributesMutator<TKey, TValue>::SetEntryIdleTimeout( System::Int32 idleTimeout )
      {
        return NativePtr->setEntryIdleTimeout( idleTimeout );
      }

      generic<class TKey, class TValue>
      ExpirationAction AttributesMutator<TKey, TValue>::SetEntryIdleTimeoutAction(
        ExpirationAction action )
      {
        return static_cast<ExpirationAction>(
          NativePtr->setEntryIdleTimeoutAction(
          static_cast<apache::geode::client::ExpirationAction::Action>( action ) ) );
      }

      generic<class TKey, class TValue>
      System::Int32 AttributesMutator<TKey, TValue>::SetEntryTimeToLive( System::Int32 timeToLive )
      {
        return NativePtr->setEntryTimeToLive( timeToLive );
      }

      generic<class TKey, class TValue>
      ExpirationAction AttributesMutator<TKey, TValue>::SetEntryTimeToLiveAction(
        ExpirationAction action )
      {
        return static_cast<ExpirationAction>(
          NativePtr->setEntryTimeToLiveAction(
          static_cast<apache::geode::client::ExpirationAction::Action>( action ) ) );
      }

      generic<class TKey, class TValue>
      System::Int32 AttributesMutator<TKey, TValue>::SetRegionIdleTimeout( System::Int32 idleTimeout )
      {
        return NativePtr->setRegionIdleTimeout( idleTimeout );
      }

      generic<class TKey, class TValue>
      ExpirationAction AttributesMutator<TKey, TValue>::SetRegionIdleTimeoutAction(
        ExpirationAction action )
      {
        return static_cast<ExpirationAction>(
          NativePtr->setRegionIdleTimeoutAction(
          static_cast<apache::geode::client::ExpirationAction::Action>( action ) ) );
      }

      generic<class TKey, class TValue>
      System::Int32 AttributesMutator<TKey, TValue>::SetRegionTimeToLive( System::Int32 timeToLive )
      {
        return NativePtr->setRegionTimeToLive( timeToLive );
      }

      generic<class TKey, class TValue>
      ExpirationAction AttributesMutator<TKey, TValue>::SetRegionTimeToLiveAction(
        ExpirationAction action )
      {
        return static_cast<ExpirationAction>(
          NativePtr->setRegionTimeToLiveAction(
          static_cast<apache::geode::client::ExpirationAction::Action>( action ) ) );
      }

      generic<class TKey, class TValue>
      System::UInt32 AttributesMutator<TKey, TValue>::SetLruEntriesLimit( System::UInt32 entriesLimit )
      {
        return NativePtr->setLruEntriesLimit( entriesLimit );
      }

      generic<class TKey, class TValue>
      void AttributesMutator<TKey, TValue>::SetCacheListener( ICacheListener<TKey, TValue>^ cacheListener )
      {
        apache::geode::client::CacheListenerPtr listenerptr;
        if (cacheListener != nullptr)
        {
          CacheListenerGeneric<TKey, TValue>^ clg = gcnew CacheListenerGeneric<TKey, TValue>();
          clg->SetCacheListener(cacheListener);
          listenerptr = new apache::geode::client::ManagedCacheListenerGeneric( /*clg,*/ cacheListener );
          ((apache::geode::client::ManagedCacheListenerGeneric*)listenerptr.ptr())->setptr(clg);
        }
        NativePtr->setCacheListener( listenerptr );
      }

      generic<class TKey, class TValue>
      void AttributesMutator<TKey, TValue>::SetCacheListener( String^ libPath,
        String^ factoryFunctionName )
      {
        throw gcnew System::NotSupportedException;
        ManagedString mg_libpath( libPath );
        ManagedString mg_factoryFunctionName( factoryFunctionName );

        NativePtr->setCacheListener( mg_libpath.CharPtr,
          mg_factoryFunctionName.CharPtr );
      }

      generic<class TKey, class TValue>
      void AttributesMutator<TKey, TValue>::SetCacheLoader( ICacheLoader<TKey, TValue>^ cacheLoader )
      {
        apache::geode::client::CacheLoaderPtr loaderptr;
        if (cacheLoader != nullptr)
        {
          CacheLoaderGeneric<TKey, TValue>^ clg = gcnew CacheLoaderGeneric<TKey, TValue>();
          clg->SetCacheLoader(cacheLoader);
          loaderptr = new apache::geode::client::ManagedCacheLoaderGeneric( /*clg,*/ cacheLoader );
          ((apache::geode::client::ManagedCacheLoaderGeneric*)loaderptr.ptr())->setptr(clg);
        }
        NativePtr->setCacheLoader( loaderptr );
      }

      generic<class TKey, class TValue>
      void AttributesMutator<TKey, TValue>::SetCacheLoader( String^ libPath,
        String^ factoryFunctionName )
      {
        throw gcnew System::NotSupportedException;
        ManagedString mg_libpath( libPath );
        ManagedString mg_factoryFunctionName( factoryFunctionName );

        NativePtr->setCacheLoader( mg_libpath.CharPtr,
          mg_factoryFunctionName.CharPtr );
      }

      generic<class TKey, class TValue>
      void AttributesMutator<TKey, TValue>::SetCacheWriter( ICacheWriter<TKey, TValue>^ cacheWriter )
      {
        apache::geode::client::CacheWriterPtr writerptr;
        if (cacheWriter != nullptr)
        {
          CacheWriterGeneric<TKey, TValue>^ cwg = gcnew CacheWriterGeneric<TKey, TValue>();
          cwg->SetCacheWriter(cacheWriter);
          writerptr = new apache::geode::client::ManagedCacheWriterGeneric( /*cwg,*/ cacheWriter );
          ((apache::geode::client::ManagedCacheWriterGeneric*)writerptr.ptr())->setptr(cwg);
        }
        NativePtr->setCacheWriter( writerptr );
      }

      generic<class TKey, class TValue>
      void AttributesMutator<TKey, TValue>::SetCacheWriter( String^ libPath,
        String^ factoryFunctionName )
      {
        throw gcnew System::NotSupportedException;
        ManagedString mg_libpath( libPath );
        ManagedString mg_factoryFunctionName( factoryFunctionName );

        NativePtr->setCacheWriter( mg_libpath.CharPtr,
          mg_factoryFunctionName.CharPtr );
      }
    }  // namespace Client
  }  // namespace Geode
}  // namespace Apache
