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



#include "ResultSet.hpp"
#include "SelectResultsIterator.hpp"
#include "ExceptionTypes.hpp"
#include "impl/SafeConvert.hpp"

using namespace System;

namespace Apache
{
  namespace Geode
  {
    namespace Client
    {

      generic<class TResult>
      size_t ResultSet<TResult>::Size::get( )
      {
        try
        {
          return static_cast<int>(m_nativeptr->get()->size( ));
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }
      }

      generic<class TResult>
      TResult ResultSet<TResult>::default::get( size_t index )
      {
        try
        {
          return TypeRegistry::GetManagedValueGeneric<TResult>(m_nativeptr->get()->operator[](index));
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }
      }

      generic<class TResult>
      System::Collections::Generic::IEnumerator<TResult>^ ResultSet<TResult>::GetEnumerator( )
      {
        return SelectResultsIterator<TResult>::Create(this);
      }

      generic<class TResult>
      System::Collections::IEnumerator^ ResultSet<TResult>::GetIEnumerator()
      {
        return SelectResultsIterator<TResult>::Create(this);
      }
    }  // namespace Client
  }  // namespace Geode
}  // namespace Apache
