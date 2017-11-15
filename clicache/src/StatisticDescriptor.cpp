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



#include "StatisticDescriptor.hpp"


namespace Apache
{
  namespace Geode
  {
    namespace Client
    {
      using namespace msclr::interop;

      System::Int32 StatisticDescriptor::ID::get( )
      {
        return  m_nativeptr->getId();
      }

      String^ StatisticDescriptor::Name::get( )
      {
        return marshal_as<String^>( m_nativeptr->getName() );
      }

      String^ StatisticDescriptor::Description::get( )
      {
        return marshal_as<String^>( m_nativeptr->getDescription() );
      }

      int8_t StatisticDescriptor::IsCounter::get( )
      {
        return m_nativeptr->isCounter();
      }

      int8_t StatisticDescriptor::IsLargerBetter::get( )
      {
        return m_nativeptr->isLargerBetter();
      }

      String^ StatisticDescriptor::Unit::get()
      {
        return marshal_as<String^>(m_nativeptr->getUnit());
      }
    }  // namespace Client
  }  // namespace Geode
}  // namespace Apache

