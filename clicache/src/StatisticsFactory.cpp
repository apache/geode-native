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
#include "impl/ManagedString.hpp"
#include "ExceptionTypes.hpp"
#include "impl/SafeConvert.hpp"


namespace Apache
{
  namespace Geode
  {
    namespace Client
    {

      // TODO globals - pass in distributed system
      //StatisticsFactory^ StatisticsFactory::GetExistingInstance(DistributedSystem^ distributedSystem)
      //{
      //  _GF_MG_EXCEPTION_TRY2/* due to auto replace */


      //    return StatisticsFactory::Create(distributedSystem->getStatisticsManager()->getStatisticsFactory());

      //  _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */
      //}

      StatisticDescriptor^ StatisticsFactory::CreateIntCounter( String^ name, String^ description,String^ units )
      {
        return CreateIntCounter(name,description,units,true);
      }

      StatisticDescriptor^ StatisticsFactory::CreateIntCounter(String^ name, String^ description,String^ units, bool largerBetter)
      {
        ManagedString mg_name( name );
        ManagedString mg_description( description );
        ManagedString mg_units( units );
        _GF_MG_EXCEPTION_TRY2/* due to auto replace */

          return StatisticDescriptor::Create(m_nativeptr->createIntCounter(mg_name.CharPtr, mg_description.CharPtr, mg_units.CharPtr, largerBetter));

        _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */
      }

      StatisticDescriptor^ StatisticsFactory::CreateLongCounter( String^ name, String^ description,String^ units )
      {
        return CreateLongCounter(name,description,units,true);
      }

      StatisticDescriptor^ StatisticsFactory::CreateLongCounter( String^ name, String^ description,String^ units, bool largerBetter )
      {
        ManagedString mg_name( name );
        ManagedString mg_description( description );
        ManagedString mg_units( units );
        _GF_MG_EXCEPTION_TRY2/* due to auto replace */

          return StatisticDescriptor::Create(m_nativeptr->createLongCounter(mg_name.CharPtr, mg_description.CharPtr, mg_units.CharPtr, largerBetter));

        _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */
      }      
        
      StatisticDescriptor^ StatisticsFactory::CreateDoubleCounter( String^ name, String^ description, String^ units )
      {
        return CreateDoubleCounter(name,description,units,true);
      }

      StatisticDescriptor^ StatisticsFactory::CreateDoubleCounter( String^ name, String^ description, String^ units, bool largerBetter )
      {
        ManagedString mg_name( name );
        ManagedString mg_description( description );
        ManagedString mg_units( units );
        _GF_MG_EXCEPTION_TRY2/* due to auto replace */

          return StatisticDescriptor::Create(m_nativeptr->createDoubleCounter(mg_name.CharPtr, mg_description.CharPtr, mg_units.CharPtr, largerBetter));

        _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */
      }

      
      StatisticDescriptor^ StatisticsFactory::CreateIntGauge( String^ name, String^ description, String^ units )
      {
        return CreateIntGauge(name,description,units,false);
      }

      StatisticDescriptor^ StatisticsFactory::CreateIntGauge( String^ name, String^ description, String^ units, bool largerBetter )
      {
        ManagedString mg_name( name );
        ManagedString mg_description( description );
        ManagedString mg_units( units );
        _GF_MG_EXCEPTION_TRY2/* due to auto replace */

          return StatisticDescriptor::Create(m_nativeptr->createIntGauge(mg_name.CharPtr, mg_description.CharPtr, mg_units.CharPtr, largerBetter));

        _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */      
      }

      StatisticDescriptor^ StatisticsFactory::CreateLongGauge( String^ name, String^ description, String^ units )
      {
        return CreateLongGauge(name,description,units,false);
      }

      StatisticDescriptor^ StatisticsFactory::CreateLongGauge( String^ name, String^ description, String^ units, bool largerBetter )
      {
        ManagedString mg_name( name );
        ManagedString mg_description( description );
        ManagedString mg_units( units );
        _GF_MG_EXCEPTION_TRY2/* due to auto replace */

          return StatisticDescriptor::Create(m_nativeptr->createLongGauge(mg_name.CharPtr, mg_description.CharPtr, mg_units.CharPtr, largerBetter));

        _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */      
      }
      
      StatisticDescriptor^ StatisticsFactory::CreateDoubleGauge( String^ name, String^ description, String^ units )
      {
        return CreateDoubleGauge(name,description,units,false);
      }

      StatisticDescriptor^ StatisticsFactory::CreateDoubleGauge( String^ name, String^ description, String^ units, bool largerBetter )
      {
        ManagedString mg_name( name );
        ManagedString mg_description( description );
        ManagedString mg_units( units );
        _GF_MG_EXCEPTION_TRY2/* due to auto replace */

          return StatisticDescriptor::Create(m_nativeptr->createDoubleGauge(mg_name.CharPtr, mg_description.CharPtr, mg_units.CharPtr, largerBetter));

        _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */      
      }

      StatisticsType^ StatisticsFactory::CreateType( String^ name, String^ description,
                                   array<StatisticDescriptor^>^ stats, System::Int32 statsLength)
      {
        ManagedString mg_name( name );
        ManagedString mg_description( description );
        _GF_MG_EXCEPTION_TRY2/* due to auto replace */
                
          apache::geode::statistics::StatisticDescriptor ** nativedescriptors = new apache::geode::statistics::StatisticDescriptor*[statsLength];
          for (System::Int32 index = 0; index < statsLength; index++)
          {
            nativedescriptors[index] = stats[index]->GetNative();
          }
          return StatisticsType::Create(m_nativeptr->createType(mg_name.CharPtr, mg_description.CharPtr, nativedescriptors, statsLength));
          
        _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */     
      }

      StatisticsType^ StatisticsFactory::FindType(String^ name)
      {
        ManagedString mg_name( name );
        _GF_MG_EXCEPTION_TRY2/* due to auto replace */

          return StatisticsType::Create(m_nativeptr->findType(mg_name.CharPtr));

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
        ManagedString mg_text( textId );
        _GF_MG_EXCEPTION_TRY2/* due to auto replace */

          return Statistics::Create(m_nativeptr->createStatistics(type->GetNative(),(char*)mg_text.CharPtr));

        _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */
      }

      Statistics^ StatisticsFactory::CreateStatistics(StatisticsType^ type, String^ textId, System::Int64 numericId)
      {
        ManagedString mg_text( textId );
        _GF_MG_EXCEPTION_TRY2/* due to auto replace */

          return Statistics::Create(m_nativeptr->createStatistics(type->GetNative(),(char*)mg_text.CharPtr, numericId));

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
        ManagedString mg_text( textId );
        _GF_MG_EXCEPTION_TRY2/* due to auto replace */

          return Statistics::Create(m_nativeptr->createAtomicStatistics(type->GetNative(),(char*)mg_text.CharPtr));

        _GF_MG_EXCEPTION_CATCH_ALL2/* due to auto replace */
      }

      Statistics^ StatisticsFactory::CreateAtomicStatistics(StatisticsType^ type, String^ textId, System::Int64 numericId)
      {
        ManagedString mg_text( textId );
        _GF_MG_EXCEPTION_TRY2/* due to auto replace */

          return Statistics::Create(m_nativeptr->createAtomicStatistics(type->GetNative(),(char*)mg_text.CharPtr, numericId));

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
        return ManagedString::Get( m_nativeptr->getName() );
      }

      System::Int64 StatisticsFactory::ID::get()
      {
        return  m_nativeptr->getId();
      }
    }  // namespace Client
  }  // namespace Geode
}  // namespace Apache

