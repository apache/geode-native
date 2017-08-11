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
#include "Statistics.hpp"
#include "StatisticDescriptor.hpp"
#include "StatisticsType.hpp"

#include "impl/ManagedString.hpp"
#include "ExceptionTypes.hpp"
#include "impl/SafeConvert.hpp"


namespace Apache
{
  namespace Geode
  {
    namespace Client
    {

      void Statistics::Close()
      {
        _GF_MG_EXCEPTION_TRY2/* due to auto replace */
          m_nativeptr->close();
        _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */ 
      }

      System::Int32 Statistics::NameToId(String^ name)
      {
        ManagedString mg_name( name );
        _GF_MG_EXCEPTION_TRY2/* due to auto replace */
          return m_nativeptr->nameToId(mg_name.CharPtr);
        _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */ 
      }

      StatisticDescriptor^ Statistics::NameToDescriptor(String^ name)
      {
        ManagedString mg_name( name );
        _GF_MG_EXCEPTION_TRY2/* due to auto replace */
          return StatisticDescriptor::Create(m_nativeptr->nameToDescriptor(mg_name.CharPtr));
        _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */ 
      }

      System::Int64 Statistics::UniqueId::get( )
      {
        return m_nativeptr->getUniqueId();
      }

      StatisticsType^ Statistics::Type::get( )
      { 
        return StatisticsType::Create(m_nativeptr->getType());
      }

      String^ Statistics::TextId::get()
      {
        return ManagedString::Get(m_nativeptr->getTextId());
      }

      System::Int64 Statistics::NumericId::get()
      {
        return m_nativeptr->getNumericId();
      }
      bool Statistics::IsAtomic::get()
      {
        return m_nativeptr->isAtomic();
      }
      bool Statistics::IsShared::get()
      {
        return m_nativeptr->isShared();
      }
      bool Statistics::IsClosed::get()
      {
        return m_nativeptr->isClosed();
      }
      
      void Statistics::SetInt(System::Int32 id, System::Int32 value)
      {
        _GF_MG_EXCEPTION_TRY2/* due to auto replace */
          m_nativeptr->setInt(id, value);
        _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */ 
      } 

      void Statistics::SetInt(String^ name, System::Int32 value)
      {
        ManagedString mg_name( name );
        _GF_MG_EXCEPTION_TRY2/* due to auto replace */
          m_nativeptr->setInt((char*)mg_name.CharPtr, value);
        _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */ 
      }

      void Statistics::SetInt(StatisticDescriptor^ descriptor, System::Int32 value)
      {
        _GF_MG_EXCEPTION_TRY2/* due to auto replace */
          m_nativeptr->setInt(descriptor->GetNative(), value);
        _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */ 
      }

      void Statistics::SetLong(System::Int32 id, System::Int64 value)
      {
        _GF_MG_EXCEPTION_TRY2/* due to auto replace */
          m_nativeptr->setLong(id, value);
        _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */ 
      }

      void Statistics::SetLong(StatisticDescriptor^ descriptor, System::Int64 value)
      {
        _GF_MG_EXCEPTION_TRY2/* due to auto replace */
          m_nativeptr->setLong(descriptor->GetNative(), value);
        _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */ 
      }

      void Statistics::SetLong(String^ name, System::Int64 value)
      {
        ManagedString mg_name( name );
        _GF_MG_EXCEPTION_TRY2/* due to auto replace */
          m_nativeptr->setLong((char*)mg_name.CharPtr, value);
        _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */ 
      }

      void Statistics::SetDouble(System::Int32 id, double value)
      {
        _GF_MG_EXCEPTION_TRY2/* due to auto replace */
          m_nativeptr->setDouble(id, value);
        _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */
      }

      void Statistics::SetDouble(String^ name, double value)
      {
        ManagedString mg_name( name );
        _GF_MG_EXCEPTION_TRY2/* due to auto replace */
          m_nativeptr->setDouble((char*)mg_name.CharPtr, value);
        _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */
      }

      void Statistics::SetDouble(StatisticDescriptor^ descriptor, double value)
      {
        _GF_MG_EXCEPTION_TRY2/* due to auto replace */
            m_nativeptr->setDouble(descriptor->GetNative(), value);
        _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */
      }

      System::Int32 Statistics::GetInt(System::Int32 id)
      {
        _GF_MG_EXCEPTION_TRY2/* due to auto replace */
          return m_nativeptr->getInt(id);
        _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */
      }

      System::Int32 Statistics::GetInt(StatisticDescriptor^ descriptor)
      {
        _GF_MG_EXCEPTION_TRY2/* due to auto replace */
          return m_nativeptr->getInt(descriptor->GetNative());
        _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */
      }

      System::Int32 Statistics::GetInt(String^ name)
      {
        ManagedString mg_name( name );
        _GF_MG_EXCEPTION_TRY2/* due to auto replace */
          return m_nativeptr->getInt((char*)mg_name.CharPtr);
        _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */
      }

      System::Int64 Statistics::GetLong(System::Int32 id)
      {
        _GF_MG_EXCEPTION_TRY2/* due to auto replace */
           return m_nativeptr->getLong(id);
        _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */
      }
       System::Int64 Statistics::GetLong(StatisticDescriptor^ descriptor)
       {
          _GF_MG_EXCEPTION_TRY2/* due to auto replace */
            return m_nativeptr->getLong(descriptor->GetNative());
          _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */
       }

      System::Int64 Statistics::GetLong(String^ name)
      {
        ManagedString mg_name( name );
        _GF_MG_EXCEPTION_TRY2/* due to auto replace */
         return m_nativeptr->getLong((char*)mg_name.CharPtr);
        _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */
      }

      double Statistics::GetDouble(System::Int32 id)
      {
         _GF_MG_EXCEPTION_TRY2/* due to auto replace */
           return m_nativeptr->getDouble(id);
         _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */
      }

      double Statistics::GetDouble(StatisticDescriptor^ descriptor)
      {
        _GF_MG_EXCEPTION_TRY2/* due to auto replace */
           return m_nativeptr->getDouble(descriptor->GetNative());
        _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */
      }

      double Statistics::GetDouble(String^ name)
      {
        ManagedString mg_name( name );
        _GF_MG_EXCEPTION_TRY2/* due to auto replace */
          return m_nativeptr->getDouble((char*)mg_name.CharPtr);
        _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */
      }

      System::Int64 Statistics::GetRawBits(StatisticDescriptor^ descriptor)
      {
         _GF_MG_EXCEPTION_TRY2/* due to auto replace */
           return m_nativeptr->getRawBits(descriptor->GetNative());
         _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */
      }

      System::Int32 Statistics::IncInt(System::Int32 id, System::Int32 delta)
      {
        _GF_MG_EXCEPTION_TRY2/* due to auto replace */
          return m_nativeptr->incInt(id,delta);
        _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */
      }

      System::Int32 Statistics::IncInt(StatisticDescriptor^ descriptor, System::Int32 delta)
      {
        _GF_MG_EXCEPTION_TRY2/* due to auto replace */
          return m_nativeptr->incInt(descriptor->GetNative(),delta);
        _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */
      }

      System::Int32 Statistics::IncInt(String^ name, System::Int32 delta)
      {
         ManagedString mg_name( name );
        _GF_MG_EXCEPTION_TRY2/* due to auto replace */
          return m_nativeptr->incInt((char*)mg_name.CharPtr,delta);
        _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */
      }

      System::Int64 Statistics::IncLong(System::Int32 id, System::Int64 delta)
      {
        _GF_MG_EXCEPTION_TRY2/* due to auto replace */
          return m_nativeptr->incLong(id,delta);
        _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */
      }

      System::Int64 Statistics::IncLong(StatisticDescriptor^ descriptor, System::Int64 delta)
      {
        _GF_MG_EXCEPTION_TRY2/* due to auto replace */
          return m_nativeptr->incLong(descriptor->GetNative(),delta);
        _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */
      }

      System::Int64 Statistics::IncLong(String^ name, System::Int64 delta)
      {
         ManagedString mg_name( name );
         _GF_MG_EXCEPTION_TRY2/* due to auto replace */
           return m_nativeptr->incLong((char*)mg_name.CharPtr,delta);
         _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */
      }

      double Statistics::IncDouble(System::Int32 id, double delta)
      {
        _GF_MG_EXCEPTION_TRY2/* due to auto replace */
          return m_nativeptr->incDouble(id,delta);
        _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */
      }

      double Statistics::IncDouble(StatisticDescriptor^ descriptor, double delta)
      {
        _GF_MG_EXCEPTION_TRY2/* due to auto replace */
          return m_nativeptr->incDouble(descriptor->GetNative(),delta);
        _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */
      }

      double Statistics::IncDouble(String^ name, double delta)
      {
        ManagedString mg_name( name );
        _GF_MG_EXCEPTION_TRY2/* due to auto replace */
          return m_nativeptr->incDouble((char*)mg_name.CharPtr,delta);
        _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */
    }  // namespace Client
  }  // namespace Geode
}  // namespace Apache
 } //namespace 

