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
#include "CacheableKey.hpp"
#include "CacheableString.hpp"
#include "CacheableBuiltins.hpp"

using namespace System;

namespace Apache
{
  namespace Geode
  {
    namespace Client
    {

      namespace native = apache::geode::client;

     // generic<class TKey>
      System::Int32 CacheableKey::GetHashCode()
      {
        try
        {
          return static_cast<native::CacheableKey*>(m_nativeptr->get())->hashcode();
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }
      }

     // generic<class TKey>
      bool CacheableKey::Equals(Client::ICacheableKey^ other)
      {
        if (other == nullptr || other->ClassId != ClassId) {
          return false;
        }
        try
        {
          return static_cast<native::CacheableKey*>(m_nativeptr->get())->operator==(
            *static_cast<native::CacheableKey*>(
            ((Client::CacheableKey^)other)->m_nativeptr->get()));
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
          GC::KeepAlive(((Client::CacheableKey^)other)->m_nativeptr);
        }
      }

      //generic<class TKey>
      bool CacheableKey::Equals(Object^ obj)
      {
        return Equals(dynamic_cast<CacheableKey^>(obj));
      }

      //generic<class TKey>
      CacheableKey::operator CacheableKey^ (Byte value)
      {
        return (CacheableKey^) CacheableByte::Create(value);
      }

      //generic<class TKey>
      CacheableKey::operator CacheableKey^ (bool value)
      {
        return (CacheableKey^) CacheableBoolean::Create(value);
      }

      //generic<class TKey>
      CacheableKey::operator CacheableKey^ (Char value)
      {
        return (CacheableKey^) CacheableCharacter::Create(value);
      }

      //generic<class TKey>
      CacheableKey::operator CacheableKey^ (Double value)
      {
        return (CacheableKey^) CacheableDouble::Create(value);
      }

      //generic<class TKey>
      CacheableKey::operator CacheableKey^ (Single value)
      {
        return (CacheableKey^) CacheableFloat::Create(value);
      }

      //generic<class TKey>
      CacheableKey::operator CacheableKey^ (System::Int16 value)
      {
        return (CacheableKey^) CacheableInt16::Create(value);
      }

      //generic<class TKey>
      CacheableKey::operator CacheableKey^ (System::Int32 value)
      {
        return (CacheableKey^) CacheableInt32::Create(value);
      }

     // generic<class TKey>
      CacheableKey::operator CacheableKey^ (System::Int64 value)
      {
        return (CacheableKey^) CacheableInt64::Create(value);
      }

      //generic<class TKey>
      CacheableKey::operator CacheableKey ^ (String^ value)
      {
        return dynamic_cast<CacheableKey^>(CacheableString::Create(value));
      }
    }  // namespace Client
  }  // namespace Geode
}  // namespace Apache
