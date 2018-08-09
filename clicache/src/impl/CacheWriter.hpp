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

//#include "../../../ICacheWriter.hpp"
#include "../CacheWriterAdapter.hpp"
#include "../ICacheWriter.hpp"
//#include "../Region.hpp"
//#include "../../../Region.hpp"
//#include "../../../Cache.hpp"

using namespace System;

using namespace Apache::Geode::Client;

namespace Apache
{
  namespace Geode
  {
    namespace Client
    {

      generic<class TKey, class TValue>
      public ref class CacheWriterGeneric : Apache::Geode::Client::CacheWriterAdapter<Object^, Object^>
      {
        private:

          ICacheWriter<TKey, TValue>^ m_writer;

        public:

          void SetCacheWriter(ICacheWriter<TKey, TValue>^ writer)
          {
            m_writer = writer;
          }

          bool BeforeUpdate( Apache::Geode::Client::EntryEvent<Object^, Object^>^ ev ) override
          {
            EntryEvent<TKey, TValue> gevent(ev->GetNative());
            return m_writer->BeforeUpdate(%gevent);
          }

          bool BeforeCreate(Apache::Geode::Client::EntryEvent<Object^, Object^>^ ev) override
          {
            EntryEvent<TKey, TValue> gevent(ev->GetNative());
            return m_writer->BeforeCreate(%gevent);
          }

          bool BeforeDestroy(Apache::Geode::Client::EntryEvent<Object^, Object^>^ ev) override
          {
            EntryEvent<TKey, TValue> gevent(ev->GetNative());
            return m_writer->BeforeDestroy(%gevent);
          }

          bool BeforeRegionClear( Apache::Geode::Client::RegionEvent<Object^, Object^>^ ev ) override
          {
            RegionEvent<TKey, TValue> gevent(ev->GetNative());
            return m_writer->BeforeRegionClear(%gevent);
          }

          bool BeforeRegionDestroy(Apache::Geode::Client::RegionEvent<Object^, Object^>^ ev) override
          {
            RegionEvent<TKey, TValue> gevent(ev->GetNative());
            return m_writer->BeforeRegionDestroy(%gevent);
          }
          
          void Close(Apache::Geode::Client::IRegion<Object^, Object^>^ region) override
          {
            m_writer->Close((IRegion<TKey, TValue>^) region);
          }
      };
    }  // namespace Client
  }  // namespace Geode
}  // namespace Apache

