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
#include "StructSet.hpp"
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
      bool StructSet<TResult>::IsModifiable::get( )
      {
        try
        {
          return m_nativeptr->get()->isModifiable( );
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }
      }

      generic<class TResult>
      System::Int32 StructSet<TResult>::Size::get( )
      {
        try
        {
          return m_nativeptr->get()->size( );
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }
      }

      generic<class TResult>
      TResult StructSet<TResult>::default::get( size_t index )
      {
        try
        {
          return Serializable::GetManagedValueGeneric<TResult>((m_nativeptr->get()->operator[](static_cast<System::Int32>(index))));
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }
      }

      generic<class TResult>
      SelectResultsIterator<TResult>^ StructSet<TResult>::GetIterator( )
      {
        try
        {
          return SelectResultsIterator<TResult>::Create(std::make_unique<apache::geode::client::SelectResultsIterator>(m_nativeptr->get()->getIterator()));
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }
      }

      generic<class TResult>
      System::Collections::Generic::IEnumerator<TResult>^ StructSet<TResult>::GetEnumerator( )
      {
        return GetIterator( );
      }

      generic<class TResult>
      System::Collections::IEnumerator^ StructSet<TResult>::GetIEnumerator( )
      {
        return GetIterator( );
      }

      generic<class TResult>
      size_t StructSet<TResult>::GetFieldIndex( String^ fieldName )
      {
        ManagedString mg_fieldName( fieldName );

        _GF_MG_EXCEPTION_TRY2/* due to auto replace */

          try
          {
            return m_nativeptr->get()->getFieldIndex( mg_fieldName.CharPtr );
          }
          finally
          {
            GC::KeepAlive(m_nativeptr);
          }

        _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */
      }

      generic<class TResult>
      String^ StructSet<TResult>::GetFieldName(size_t index)
      {
        try
        {
          return ManagedString::Get(m_nativeptr->get()->getFieldName(static_cast<System::Int32> (index)).c_str());
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }
      }
    }  // namespace Client
  }  // namespace Geode
}  // namespace Apache
