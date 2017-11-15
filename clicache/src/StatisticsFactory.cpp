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


#include "begin_native.hpp"
#include "statistics/StatisticsManager.hpp"
#include "end_native.hpp"

#include "StatisticsFactory.hpp"
#include "StatisticsType.hpp"
#include "StatisticDescriptor.hpp"
#include "Statistics.hpp"
#include "ExceptionTypes.hpp"
#include "impl/SafeConvert.hpp"


namespace Apache
{
  namespace Geode
  {
    namespace Client
    {
      using namespace msclr::interop;

      StatisticDescriptor^ StatisticsFactory::CreateIntCounter( String^ name, String^ description,String^ units )
      {
        return CreateIntCounter(name,description,units,true);
      }

      StatisticDescriptor^ StatisticsFactory::CreateIntCounter(String^ name, String^ description,String^ units, bool largerBetter)
      {
        _GF_MG_EXCEPTION_TRY2/* due to auto replace */

          return StatisticDescriptor::Create(m_nativeptr->createIntCounter(
              marshal_as<std::string>(name),
              marshal_as<std::string>(description),
              marshal_as<std::string>(units),
              largerBetter));

        _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */
      }

      StatisticDescriptor^ StatisticsFactory::CreateLongCounter( String^ name, String^ description,String^ units )
      {
        return CreateLongCounter(name,description,units,true);
      }

      StatisticDescriptor^ StatisticsFactory::CreateLongCounter( String^ name, String^ description,String^ units, bool largerBetter )
      {
        _GF_MG_EXCEPTION_TRY2/* due to auto replace */

          return StatisticDescriptor::Create(m_nativeptr->createLongCounter(
              marshal_as<std::string>(name),
              marshal_as<std::string>(description),
              marshal_as<std::string>(units),
              largerBetter));

        _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */
      }      
        
      StatisticDescriptor^ StatisticsFactory::CreateDoubleCounter( String^ name, String^ description, String^ units )
      {
        return CreateDoubleCounter(name,description,units,true);
      }

      StatisticDescriptor^ StatisticsFactory::CreateDoubleCounter( String^ name, String^ description, String^ units, bool largerBetter )
      {
        _GF_MG_EXCEPTION_TRY2/* due to auto replace */

          return StatisticDescriptor::Create(m_nativeptr->createDoubleCounter(
              marshal_as<std::string>(name),
              marshal_as<std::string>(description),
              marshal_as<std::string>(units),
              largerBetter));

        _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */
      }

      
      StatisticDescriptor^ StatisticsFactory::CreateIntGauge( String^ name, String^ description, String^ units )
      {
        return CreateIntGauge(name,description,units,false);
      }

      StatisticDescriptor^ StatisticsFactory::CreateIntGauge( String^ name, String^ description, String^ units, bool largerBetter )
      {
        _GF_MG_EXCEPTION_TRY2/* due to auto replace */

          return StatisticDescriptor::Create(m_nativeptr->createIntGauge(
              marshal_as<std::string>(name),
              marshal_as<std::string>(description),
              marshal_as<std::string>(units),
              largerBetter));

        _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */      
      }

      StatisticDescriptor^ StatisticsFactory::CreateLongGauge( String^ name, String^ description, String^ units )
      {
        return CreateLongGauge(name,description,units,false);
      }

      StatisticDescriptor^ StatisticsFactory::CreateLongGauge( String^ name, String^ description, String^ units, bool largerBetter )
      {
        _GF_MG_EXCEPTION_TRY2/* due to auto replace */

          return StatisticDescriptor::Create(m_nativeptr->createLongGauge(
              marshal_as<std::string>(name),
              marshal_as<std::string>(description),
              marshal_as<std::string>(units),
              largerBetter));

        _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */      
      }
      
      StatisticDescriptor^ StatisticsFactory::CreateDoubleGauge( String^ name, String^ description, String^ units )
      {
        return CreateDoubleGauge(name,description,units,false);
      }

      StatisticDescriptor^ StatisticsFactory::CreateDoubleGauge( String^ name, String^ description, String^ units, bool largerBetter )
      {
        _GF_MG_EXCEPTION_TRY2/* due to auto replace */

          return StatisticDescriptor::Create(m_nativeptr->createDoubleGauge(
              marshal_as<std::string>(name),
              marshal_as<std::string>(description),
              marshal_as<std::string>(units),
              largerBetter));

        _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */      
      }

      StatisticsType^ StatisticsFactory::CreateType( String^ name, String^ description,
                                   array<StatisticDescriptor^>^ stats, System::Int32 statsLength)
      {
        _GF_MG_EXCEPTION_TRY2/* due to auto replace */
                
          apache::geode::statistics::StatisticDescriptor ** nativedescriptors = new apache::geode::statistics::StatisticDescriptor*[statsLength];
          for (System::Int32 index = 0; index < statsLength; index++)
          {
            nativedescriptors[index] = stats[index]->GetNative();
          }
          return StatisticsType::Create(m_nativeptr->createType(
              marshal_as<std::string>(name),
              marshal_as<std::string>(description),
              nativedescriptors,
              statsLength));
          
        _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */     
      }

      StatisticsType^ StatisticsFactory::FindType(String^ name)
      {
        _GF_MG_EXCEPTION_TRY2/* due to auto replace */

          return StatisticsType::Create(m_nativeptr->findType(marshal_as<std::string>(name)));

        _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */     
      }

      Statistics^ StatisticsFactory::CreateStatistics(StatisticsType^ type)
      {
        _GF_MG_EXCEPTION_TRY2/* due to auto replace */
         
          return Statistics::Create(m_nativeptr->createStatistics(type->GetNative()));

        _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */
      }

      Statistics^ StatisticsFactory::CreateStatistics(StatisticsType^ type, String^ textId)
      {
        _GF_MG_EXCEPTION_TRY2/* due to auto replace */

          return Statistics::Create(m_nativeptr->createStatistics(type->GetNative(), marshal_as<std::string>(textId)));

        _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */
      }

      Statistics^ StatisticsFactory::CreateStatistics(StatisticsType^ type, String^ textId, System::Int64 numericId)
      {
        _GF_MG_EXCEPTION_TRY2/* due to auto replace */

          return Statistics::Create(m_nativeptr->createStatistics(type->GetNative(), marshal_as<std::string>(textId), numericId));

        _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */
      }

      Statistics^ StatisticsFactory::CreateAtomicStatistics(StatisticsType^ type)
      {
        _GF_MG_EXCEPTION_TRY2/* due to auto replace */
         
          return Statistics::Create(m_nativeptr->createAtomicStatistics(type->GetNative()));

        _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */
      }

      Statistics^ StatisticsFactory::CreateAtomicStatistics(StatisticsType^ type, String^ textId)
      {
        _GF_MG_EXCEPTION_TRY2/* due to auto replace */

          return Statistics::Create(m_nativeptr->createAtomicStatistics(type->GetNative(), marshal_as<std::string>(textId)));

        _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */
      }

      Statistics^ StatisticsFactory::CreateAtomicStatistics(StatisticsType^ type, String^ textId, System::Int64 numericId)
      {
        _GF_MG_EXCEPTION_TRY2/* due to auto replace */

          return Statistics::Create(m_nativeptr->createAtomicStatistics(type->GetNative(), marshal_as<std::string>(textId), numericId));

        _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */
      }
      Statistics^ StatisticsFactory::FindFirstStatisticsByType( StatisticsType^ type )
      {
        _GF_MG_EXCEPTION_TRY2/* due to auto replace */
         
          return Statistics::Create(m_nativeptr->findFirstStatisticsByType(type->GetNative()));

        _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */
      }

      String^ StatisticsFactory::Name::get( )
      {
        return marshal_as<String^>( m_nativeptr->getName() );
      }

      System::Int64 StatisticsFactory::ID::get()
      {
        return  m_nativeptr->getId();
      }
    }  // namespace Client
  }  // namespace Geode
}  // namespace Apache

