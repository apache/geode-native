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
      size_t StructSet<TResult>::Size::get( )
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
          return TypeRegistry::GetManagedValueGeneric<TResult>(m_nativeptr->get()->operator[](index));
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }
      }

      generic<class TResult>
      System::Collections::Generic::IEnumerator<TResult>^ StructSet<TResult>::GetEnumerator( )
      {
        return SelectResultsIterator<TResult>::Create(this);
      }

      generic<class TResult>
      System::Collections::IEnumerator^ StructSet<TResult>::GetIEnumerator( )
      {
        return SelectResultsIterator<TResult>::Create(this);
      }

      generic<class TResult>
      int32_t StructSet<TResult>::GetFieldIndex( String^ fieldName )
      {
        try {/* due to auto replace */

          try
          {
            return m_nativeptr->get()->getFieldIndex( marshal_as<std::string>(fieldName) );
          }
          finally
          {
            GC::KeepAlive(m_nativeptr);
          }

        }
        catch (const apache::geode::client::Exception& ex) {
          throw Apache::Geode::Client::GeodeException::Get(ex);
        }
        catch (System::AccessViolationException^ ex) {
          throw ex;
        }/* due to auto replace */
      }

      generic<class TResult>
      String^ StructSet<TResult>::GetFieldName(int32_t index)
      {
        try
        {
          return marshal_as<String^>(m_nativeptr->get()->getFieldName(index));
        }
        finally
        {
          GC::KeepAlive(m_nativeptr);
        }
      }
    }  // namespace Client
  }  // namespace Geode
}  // namespace Apache
