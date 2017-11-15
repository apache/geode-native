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


#include "StatisticsType.hpp"
#include "StatisticDescriptor.hpp"

#include "ExceptionTypes.hpp"
#include "impl/SafeConvert.hpp"


namespace Apache
{
  namespace Geode
  {
    namespace Client
    {
      using namespace msclr::interop;

      String^ StatisticsType::Name::get()
      {
        return marshal_as<String^>( m_nativeptr->getName() );
      }

      String^ StatisticsType::Description::get()
      {
        return marshal_as<String^>( m_nativeptr->getDescription() );
      }

      array<StatisticDescriptor^>^ StatisticsType::Statistics::get()
      {
        _GF_MG_EXCEPTION_TRY2/* due to auto replace */

          apache::geode::statistics::StatisticDescriptor ** nativedescriptors = m_nativeptr->getStatistics();
          array<StatisticDescriptor^>^ descriptors = gcnew array<StatisticDescriptor^>(m_nativeptr->getDescriptorsCount());
          for (int item = 0; item < m_nativeptr->getDescriptorsCount(); item++)
          {
            descriptors[item] = StatisticDescriptor::Create(nativedescriptors[item]);
          }
          return descriptors;

        _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */
      }

      System::Int32 StatisticsType::NameToId( String^ name )
      {
        _GF_MG_EXCEPTION_TRY2/* due to auto replace */
          return m_nativeptr->nameToId(marshal_as<std::string>(name));
        _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */
      }

      StatisticDescriptor^ StatisticsType::NameToDescriptor( String^ name )
      {
        _GF_MG_EXCEPTION_TRY2/* due to auto replace */
          return StatisticDescriptor::Create(m_nativeptr->nameToDescriptor(marshal_as<std::string>(name)));
        _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */
      }

      System::Int32 StatisticsType::DescriptorsCount::get()
      {
        return m_nativeptr->getDescriptorsCount();
    }  // namespace Client
  }  // namespace Geode
}  // namespace Apache

 } //namespace 

