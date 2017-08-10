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

#include "geode_defs.hpp"
#include "begin_native.hpp"
#include <geode/statistics/StatisticsFactory.hpp>
#include <geode/statistics/StatisticsType.hpp>
#include <geode/statistics/StatisticDescriptor.hpp>
#include <geode/statistics/Statistics.hpp>
#include "end_native.hpp"

using namespace System;

namespace Apache
{
  namespace Geode
  {
    namespace Client
    {

      ref class StatisticDescriptor;
      ref class StatisticsType;
      ref class Statistics;

      /// <summary>
      /// Instances of this interface provide methods that create instances
      /// of <see cref="StatisticDescriptor" /> and <see cref="StatisticsType" />.
      /// Every <see cref="StatisticsFactory" /> is also a type factory.
      /// </summary>
      /// <para>
      /// A <c>StatisticsFactory</c> can create a <see cref="StatisticDescriptor" />
      /// statistic of three numeric types:
      /// <c>int</c>, <c>long</c>, and <c>double</c>.  A
      /// statistic (<c>StatisticDescriptor</c>) can either be a
      /// <I>gauge</I> meaning that its value can increase and decrease or a
      /// <I>counter</I> meaning that its value is strictly increasing.
      /// Marking a statistic as a counter allows the Geode Manager Console
      /// to properly display a statistics whose value "wraps around" (that
      /// is, exceeds its maximum value).
      /// </para>
      public ref class StatisticsFactory sealed
      {
      protected:
        StatisticsFactory(){}
        StatisticsFactory(StatisticsFactory^){}
      public:
        /// <summary>
        /// Return a pre-existing statistics factory. Typically configured through
        /// creation of a distributed system.
        /// </summary>
        //static StatisticsFactory^ GetExistingInstance();

        /// <summary>
        /// Creates and returns an int counter  <see cref="StatisticDescriptor" />
        /// with the given <c>name</c>, <c>description</c>,
        /// <c>units</c>, and with larger values indicating better performance.
        /// </summary>
        virtual StatisticDescriptor^ CreateIntCounter(String^ name, String^ description, String^ units, bool largerBetter);

        /// <summary>
        /// Creates and returns an int counter  <see cref="StatisticDescriptor" />
        /// with the given <c>name</c>, <c>description</c>,
        /// <c>units</c>.
        /// </summary>
        virtual StatisticDescriptor^ CreateIntCounter(String^ name, String^ description, String^ units);

        /// <summary>
        /// Creates and returns an long counter  <see cref="StatisticDescriptor" />
        /// with the given <c>name</c>, <c>description</c>,
        /// <c>units</c>, and with larger values indicating better performance.
        /// </summary>
        virtual StatisticDescriptor^ CreateLongCounter(String^ name, String^ description, String^ units, bool largerBetter);

        /// <summary>
        /// Creates and returns an long counter  <see cref="StatisticDescriptor" />
        /// with the given <c>name</c>, <c>description</c>,
        /// <c>units</c>.
        /// </summary>
        virtual StatisticDescriptor^ CreateLongCounter(String^ name, String^ description, String^ units);

        /// <summary>
        /// Creates and returns an double counter  <see cref="StatisticDescriptor" />
        /// with the given <c>name</c>, <c>description</c>,
        /// <c>units</c>, and with larger values indicating better performance.
        /// </summary>

        virtual StatisticDescriptor^ CreateDoubleCounter(String^ name, String^ description, String^ units, bool largerBetter);

        /// <summary>
        /// Creates and returns an double counter  <see cref="StatisticDescriptor" />
        /// with the given <c>name</c>, <c>description</c>,
        /// <c>units</c>.
        /// </summary>
        virtual StatisticDescriptor^ CreateDoubleCounter(String^ name, String^ description, String^ units);

        /// <summary>
        /// Creates and returns an int gauge  <see cref="StatisticDescriptor" />
        /// with the given <c>name</c>, <c>description</c>,
        /// <c>units</c>, and with smaller values indicating better performance.
        /// </summary>
        virtual StatisticDescriptor^ CreateIntGauge(String^ name, String^ description, String^ units, bool largerBetter);

        /// <summary>
        /// Creates and returns an int gauge  <see cref="StatisticDescriptor" />
        /// with the given <c>name</c>, <c>description</c>,
        /// <c>units</c>.
        /// </summary>
        virtual StatisticDescriptor^ CreateIntGauge(String^ name, String^ description, String^ units);

        /// <summary>
        /// Creates and returns an long gauge <see cref="StatisticDescriptor" />
        /// with the given <c>name</c>, <c>description</c>,
        /// <c>units</c>, and with smaller values indicating better performance.
        /// </summary>
        virtual StatisticDescriptor^ CreateLongGauge(String^ name, String^ description, String^ units, bool largerBetter);

        /// <summary>
        /// Creates and returns an long gauge <see cref="StatisticDescriptor" />
        /// with the given <c>name</c>, <c>description</c>,
        /// <c>units</c>.
        /// </summary>
        virtual StatisticDescriptor^ CreateLongGauge(String^ name, String^ description, String^ units);

        /// <summary>
        /// Creates and returns an double gauge <see cref="StatisticDescriptor" />
        /// with the given <c>name</c>, <c>description</c>,
        /// <c>units</c>, and with smaller values indicating better performance.
        /// </summary>
        virtual StatisticDescriptor^ CreateDoubleGauge(String^ name, String^ description, String^ units, bool largerBetter);

        /// <summary>
        /// Creates and returns an double gauge <see cref="StatisticDescriptor" />
        /// with the given <c>name</c>, <c>description</c>,
        /// <c>units</c>.
        /// </summary>
        virtual StatisticDescriptor^ CreateDoubleGauge(String^ name, String^ description, String^ units);

        /// <summary>
        /// Creates and returns a <see cref="StatisticsType" /> 
        /// with the given <c>name</c>, <c>description</c>,and <see cref="StatisticDescriptor" />
        /// </summary>
        /// <exception cref="IllegalArgumentException">
        /// if a type with the given <c>name</c> already exists.
        /// </exception>
        virtual StatisticsType^ CreateType(String^ name, String^ description,
                                           array<StatisticDescriptor^>^ stats, System::Int32 statsLength);

        /// <summary>
        /// Finds and returns an already created <see cref="StatisticsType" /> 
        /// with the given <c>name</c>. Returns <c>null</c> if the type does not exist.
        /// </summary>
        virtual StatisticsType^ FindType(String^ name);

        /// <summary>
        /// Creates and returns a <see cref="Statistics" /> instance of the given <see cref="StatisticsType" /> type, <c>textId</c>, and with default ids.
        /// </summary>
        /// <para>
        /// The created instance may not be <see cref="Statistics#isAtomic" /> atomic.
        /// </para>
        virtual Statistics^ CreateStatistics(StatisticsType^ type);

        /// <summary>
        /// Creates and returns a <see cref="Statistics" /> instance of the given <see cref="StatisticsType" /> type, <c>textId</c>, and with a default numeric id.
        /// </summary>
        /// <para>
        /// The created instance may not be <see cref="Statistics#isAtomic" /> atomic.
        /// </para>
        virtual Statistics^ CreateStatistics(StatisticsType^ type, String^ textId);

        /// <summary>
        /// Creates and returns a <see cref="Statistics" /> instance of the given <see cref="StatisticsType" /> type, <c>textId</c>, and <c>numericId</c>.
        /// </summary>
        /// <para>
        /// The created instance may not be <see cref="Statistics#isAtomic" /> atomic.
        /// </para>
        virtual Statistics^ CreateStatistics(StatisticsType^ type, String^ textId, System::Int64 numericId);

        /// <summary>
        /// Creates and returns a <see cref="Statistics" /> instance of the given <see cref="StatisticsType" /> type, <c>textId</c>, and with default ids.
        /// </summary>
        /// <para>
        /// The created instance will be <see cref="Statistics#isAtomic" /> atomic.
        /// </para>
        virtual Statistics^ CreateAtomicStatistics(StatisticsType^ type);

        /// <summary>
        /// Creates and returns a <see cref="Statistics" /> instance of the given <see cref="StatisticsType" /> type, <c>textId</c>, and with a default numeric id.
        /// </summary>
        /// <para>
        /// The created instance will be <see cref="Statistics#isAtomic" /> atomic.
        /// </para>
        virtual Statistics^ CreateAtomicStatistics(StatisticsType^ type, String^ textId);

        /// <summary>
        /// Creates and returns a <see cref="Statistics" /> instance of the given <see cref="StatisticsType" /> type, <c>textId</c>, and <c>numericId</c>.
        /// </summary>
        /// <para>
        /// The created instance will be <see cref="Statistics#isAtomic" /> atomic.
        /// </para>
        virtual Statistics^ CreateAtomicStatistics(StatisticsType^ type, String^ textId, System::Int64 numericId);

        /// <summary>
        /// Return the first instance that matches the type, or NULL
        /// </summary>
        virtual Statistics^ FindFirstStatisticsByType(StatisticsType^ type);

        /// <summary>
        /// Returns a name that can be used to identify the manager
        /// </summary>
        virtual property String^ Name
        {
          virtual String^ get();
        }

        /// <summary>
        /// Returns a numeric id that can be used to identify the manager
        /// </summary>
        virtual property System::Int64 ID
        {
          virtual System::Int64 get();
        }

      internal:
        /// <summary>
        /// Internal factory function to wrap a native object pointer inside
        /// this managed class, with null pointer check.
        /// </summary>
        /// <param name="nativeptr">native object pointer</param>
        /// <returns>
        /// the managed wrapper object, or null if the native pointer is null.
        /// </returns>
        inline static StatisticsFactory^ Create(
          apache::geode::statistics::StatisticsFactory* nativeptr)
        {
          return __nullptr == nativeptr ? nullptr :
            gcnew StatisticsFactory( nativeptr );
        }

      private:
        /// <summary>
        /// Private constructor to wrap a native object pointer
        /// </summary>
        /// <param name="nativeptr">The native object pointer</param>
        inline StatisticsFactory(apache::geode::statistics::StatisticsFactory* nativeptr)
          : m_nativeptr( nativeptr )
        {
        }

        apache::geode::statistics::StatisticsFactory* m_nativeptr;
      };
    }  // namespace Client
  }  // namespace Geode
}  // namespace Apache


