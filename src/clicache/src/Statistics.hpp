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
#include <geode/statistics/Statistics.hpp>
#include <geode/statistics/StatisticDescriptor.hpp>
#include <geode/statistics/StatisticsType.hpp>
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

      /// <summary>
      /// An instantiation of an existing <c>StatisticsType</c> object with methods for
      /// setting, incrementing and getting individual <c>StatisticDescriptor</c> values.
      /// </summary>
      public ref class Statistics sealed
       {
       public:

         /// <summary>
         /// Closes these statistics.  After statistics have been closed, they
         /// are no longer archived.
         /// A value access on a closed statistics always results in zero.
         /// A value modification on a closed statistics is ignored.
         /// </summary>
         virtual void Close();

         /// <summary>
         /// Returns the id of the statistic with the given name in this
         /// statistics instance.
         /// </summary>
         /// <param name="name">the statistic name</param>
         /// <returns>the id of the statistic with the given name</returns>
         /// <exception cref="IllegalArgumentException">
         /// if no statistic named <c>name</c> exists in this
         /// statistics instance.
         /// </exception>
         /// <see cref="StatisticsType#nameToDescriptor" />        
         virtual System::Int32 NameToId(String^ name);

         /// <summary>
         /// Returns the descriptor of the statistic with the given name in this
         /// statistics instance.
         /// </summary>
         /// <param name="name">the statistic name</param>
         /// <returns>the descriptor of the statistic with the given name</returns>
         /// <exception cref="IllegalArgumentException">
         /// if no statistic named <c>name</c> exists in this
         /// statistics instance.
         /// </exception>
         /// <see cref="StatisticsType#nameToId" />
         virtual StatisticDescriptor^ NameToDescriptor(String^ name);

         /// <summary>
         /// Gets a value that uniquely identifies this statistics.
         /// </summary>
         virtual property System::Int64 UniqueId
         {
           virtual System::Int64 get( );
         }

         /// <summary>
         /// Gets the <see cref="StatisticsType" /> of this instance.
         /// </summary>
         virtual property StatisticsType^ Type
         {
           virtual StatisticsType^ get( );
         }

         /// <summary>
         /// Gets the text associated with this instance that helps identify it.
         /// </summary>
         virtual property String^ TextId
         {
           virtual String^ get( );
         }

         /// <summary>
         /// Gets the number associated with this instance that helps identify it.
         /// </summary>
         virtual property System::Int64 NumericId 
         {
           virtual System::Int64 get( );
         }

         /// <summary>
         /// Returns true if modifications are atomic. This means that multiple threads
         /// can safely modify this instance without additional synchronization.
         /// </summary>
         /// <para>
         /// Returns false if modifications are not atomic. This means that modifications
         /// to this instance are cheaper but not thread safe.
         /// </para>
         /// <para>
         /// Note that all instances that are <see cref="#isShared" /> shared are also atomic.
         /// </para>
         virtual property bool IsAtomic
         {
           virtual bool get( );
         }

         /// <summary>
         /// Returns true if the data for this instance is stored in shared memory.
         /// Returns false if the data is store in local memory.
         /// </summary>
         /// <para>
         /// Note that all instances that are <see cref="#isShared" /> shared are also atomic.
         /// </para>
         virtual property bool IsShared
         {
           virtual bool get( );
         }

         /// <summary>
         /// Returns true if the instance has been <see cref="#close" /> closed.
         /// </summary>
         virtual property bool IsClosed
         {
           virtual bool get( );
         }

         /// <summary>
         /// Sets the value of a statistic with the given <c>id</c>
         /// whose type is <c>int</c>.
         /// </summary>
         /// <param name="id">a statistic id obtained with <see cref="#nameToId" />
         /// or <see cref="#StatisticsType#nameToId" /> </param>
         /// <param name="value">value to set</param>
         /// <exception cref="IllegalArgumentException">
         /// If the id is invalid.
         /// </exception>
         virtual void SetInt(System::Int32 id, System::Int32 value);

         /// <summary>
         /// Sets the value of a named statistic of type <c>int</c>
         /// </summary>
         /// <param name="name">statistic name</param>
         /// <param name="value">value to set</param>
         /// <exception cref="IllegalArgumentException">
         /// If no statistic exists named <c>name</c> or
         /// if the statistic with name <c>name</c> is not of
         /// type <c>int</c>.
         /// </exception>
         virtual void SetInt(String^ name, System::Int32 value);

         /// <summary>
         /// Sets the value of a described statistic of type <c>int</c>
         /// </summary>
         /// <param name="descriptor">a statistic descriptor obtained with <see cref="#nameToDescriptor" />
         /// or <see cref="#StatisticsType#nameToDescriptor" /> </param>
         /// <param name="value">value to set</param>
         /// <exception cref="IllegalArgumentException">
         /// If no statistic exists for the given <c>descriptor</c> or
         /// if the described statistic is not of
         /// type <c>int</c>.
         /// </exception>
         virtual void SetInt(StatisticDescriptor^ descriptor, System::Int32 value);

         /// <summary>
         /// Sets the value of a statistic with the given <c>id</c>
         /// whose type is <c>long</c>.
         /// </summary>
         /// <param name="id">a statistic id obtained with <see cref="#nameToId" /> 
         /// or <see cref="#StatisticsType#nameToId" />. </param>
         /// <param name="value">value to set</param>
         /// <exception cref="IllegalArgumentException">
         /// If the id is invalid.
         /// </exception>
         virtual void SetLong(System::Int32 id, System::Int64 value); 

         /// <summary>
         /// Sets the value of a described statistic of type <c>long</c>
         /// </summary>
         /// <param name="descriptor">a statistic descriptor obtained with <see cref="#nameToDescriptor" />
         /// or <see cref="StatisticsType#nameToDescriptor" /> </param>
         /// <param name="value">value to set</param>
         /// <exception cref="IllegalArgumentException">
         /// If no statistic exists for the given <c>descriptor</c> or
         /// if the described statistic is not of
         /// type <c>long</c>.
         /// </exception>
         virtual void SetLong(StatisticDescriptor^ descriptor, System::Int64 value);

         /// <summary>
         /// Sets the value of a named statistic of type <c>long</c>.
         /// </summary>
         /// <param name="name">statistic name</param>
         /// <param name="value">value to set</param>
         /// <exception cref="IllegalArgumentException">
         /// If no statistic exists named <c>name</c> or
         /// if the statistic with name <c>name</c> is not of
         /// type <c>long</c>.
         /// </exception>
         virtual void SetLong(String^ name, System::Int64 value);


         /// <summary>
         /// Sets the value of a statistic with the given <c>id</c>
         /// whose type is <c>double</c>.
         /// </summary>
         /// <param name="id">a statistic id obtained with <see cref="#nameToId" />
         /// or <see cref="#StatisticsType#nameToId" /> </param>
         /// <param name="value">value to set</param>
         /// <exception cref="IllegalArgumentException">
         /// If the id is invalid.
         /// </exception>
         virtual void SetDouble(System::Int32 id, double value);

         /// <summary>
         /// Sets the value of a named statistic of type <c>double</c>
         /// </summary>
         /// <param name="name">statistic name</param>
         /// <param name="value">value to set</param>
         /// <exception cref="IllegalArgumentException">
         /// If no statistic exists named <c>name</c> or
         /// if the statistic with name <c>name</c> is not of
         /// type <c>double</c>.
         /// </exception>
         virtual void SetDouble(String^ name, double value);

         /// <summary>
         /// Sets the value of a described statistic of type <c>double</c>
         /// </summary>
         /// <param name="descriptor">a statistic descriptor obtained with <see cref="#nameToDescriptor" />
         /// or <see cref="StatisticsType#nameToDescriptor" /> </param>
         /// <param name="value">value to set</param>
         /// <exception cref="IllegalArgumentException">
         /// If no statistic exists for the given <c>descriptor</c> or
         /// if the described statistic is not of
         /// type <c>double</c>.
         /// </exception>
         virtual void SetDouble(StatisticDescriptor^ descriptor, double value);

         /// <summary>
         /// Returns the value of the identified statistic of type <c>int</c>.
         /// whose type is <c>double</c>.
         /// </summary>
         /// <param name="id">a statistic id obtained with <see cref="#nameToId" />
         /// or <see cref="#StatisticsType#nameToId" /> </param>
         /// <exception cref="IllegalArgumentException">
         /// If the id is invalid.
         /// </exception>
         virtual System::Int32 GetInt(System::Int32 id);

         /// <summary>
         /// Returns the value of the described statistic of type <code>int</code>.
         /// </summary>
         /// <param name="descriptor">a statistic descriptor obtained with <see cref="#nameToDescriptor" />
         /// or <see cref="StatisticsType#nameToDescriptor" /> </param>
         /// <exception cref="IllegalArgumentException">
         /// If no statistic exists with the specified <c>descriptor</c> or
         /// if the described statistic is not of
         /// type <c>int</c>.
         /// </exception>
         virtual System::Int32 GetInt(StatisticDescriptor^ descriptor);


         /// <summary>
         /// Returns the value of the statistic of type <code>int</code> at
         /// the given name.
         /// </summary>
         /// <param name="name">statistic name</param>
         /// <exception cref="IllegalArgumentException">
         /// If no statistic exists named <c>name</c> or
         /// if the statistic with name <c>name</c> is not of
         /// type <c>int</c>.
         /// </exception>
         virtual System::Int32 GetInt(String^ name);

         /// <summary>
         /// Returns the value of the identified statistic of type <c>long</c>.
         /// </summary>
         /// <param name="id">a statistic id obtained with <see cref="#nameToId" />
         /// or <see cref="#StatisticsType#nameToId" /> </param>
         /// <exception cref="IllegalArgumentException">
         /// If the id is invalid.
         /// </exception>
         virtual System::Int64 GetLong(System::Int32 id);

         
         /// <summary>
         /// Returns the value of the described statistic of type <c>long</c>.
         /// </summary>
         /// <param name="descriptor">a statistic descriptor obtained with <see cref="#nameToDescriptor" />
         /// or <see cref="StatisticsType#nameToDescriptor" /> </param>
         /// <exception cref="IllegalArgumentException">
         /// If no statistic exists with the specified <c>descriptor</c> or
         /// if the described statistic is not of
         /// type <c>long</c>.
         /// </exception>
         virtual System::Int64 GetLong(StatisticDescriptor^ descriptor);

         
         /// <summary>
         /// Returns the value of the statistic of type <c>long</c> at
         /// the given name.
         /// </summary>
         /// <param name="name">statistic name</param>
         /// <exception cref="IllegalArgumentException">
         /// If no statistic exists named <c>name</c> or
         /// if the statistic with name <c>name</c> is not of
         /// type <c>long</c>.
         /// </exception>
         virtual System::Int64 GetLong(String^ name);


         /// <summary>
         /// Returns the value of the identified statistic of type <c>double</c>.
         /// </summary>
         /// <param name="id">a statistic id obtained with <see cref="#nameToId" />
         /// or <see cref="#StatisticsType#nameToId" /> </param>
         /// <exception cref="IllegalArgumentException">
         /// If the id is invalid.
         /// </exception>
         virtual double GetDouble(System::Int32 id);
                  
         /// <summary>
         /// Returns the value of the described statistic of type <c>double</c>.
         /// </summary>
         /// <param name="descriptor">a statistic descriptor obtained with <see cref="#nameToDescriptor" />
         /// or <see cref="StatisticsType#nameToDescriptor" /> </param>
         /// <exception cref="IllegalArgumentException">
         /// If no statistic exists with the specified <c>descriptor</c> or
         /// if the described statistic is not of
         /// type <c>double</c>.
         /// </exception>
         virtual double GetDouble(StatisticDescriptor^ descriptor);

         /// <summary>
         /// Returns the value of the statistic of type <c>double</c> at
         /// the given name.
         /// </summary>
         /// <param name="name">statistic name</param>
         /// <exception cref="IllegalArgumentException">
         /// If no statistic exists named <c>name</c> or
         /// if the statistic with name <c>name</c> is not of
         /// type <c>double</c>.
         /// </exception>
         virtual double GetDouble(String^ name);

         /// <summary>
         /// Returns the bits that represent the raw value of the described statistic.
         /// </summary>
         /// <param name="descriptor">a statistic descriptor obtained with <see cref="#nameToDescriptor" />
         /// or <see cref="StatisticsType#nameToDescriptor" /> </param>
         /// <exception cref="IllegalArgumentException">
         /// If the described statistic does not exist
         /// </exception>
         virtual System::Int64 GetRawBits(StatisticDescriptor^ descriptor);

         /// <summary>
         /// Increments the value of the identified statistic of type <c>int</c>
         /// by the given amount.
         /// </summary>
         /// <param name="id">a statistic id obtained with <see cref="#nameToId" />
         /// or <see cref="#StatisticsType#nameToId" /> </param>
         /// <param name="delta">the value of the statistic after it has been incremented</param>
         /// <returns>the value of the statistic after it has been incremented</returns>
         /// <exception cref="IllegalArgumentException">
         /// If the id is invalid.
         /// </exception>
         virtual System::Int32 IncInt(System::Int32 id, System::Int32 delta);

         /// <summary>
         /// Increments the value of the described statistic of type <c>int</c>
         /// by the given amount.
         /// </summary>
         /// <param name="descriptor">a statistic descriptor obtained with <see cref="#nameToDescriptor" />
         /// or <see cref="StatisticsType#nameToDescriptor" /> </param>
         /// <param name="delta">change value to be added</param>
         /// <returns>the value of the statistic after it has been incremented</returns>
         /// <exception cref="IllegalArgumentException">
         /// If no statistic exists for the given <c>descriptor</c> or
         /// if the described statistic is not of
         /// type <c>int</c>.
         /// </exception>
         virtual System::Int32 IncInt(StatisticDescriptor^ descriptor, System::Int32 delta);

         /// <summary>
         /// Increments the value of the statistic of type <c>int</c> with
         /// the given name by a given amount.
         /// </summary>
         /// <param name="name">statistic name</param>
         /// <param name="delta">change value to be added</param>
         /// <returns>the value of the statistic after it has been incremented</returns>
         /// <exception cref="IllegalArgumentException">
         /// If no statistic exists named <c>name</c> or
         /// if the statistic with name <c>name</c> is not of
         /// type <c>int</c>.
         /// </exception>
         virtual System::Int32 IncInt(String^ name, System::Int32 delta);

         /// <summary>
         /// Increments the value of the identified statistic of type <c>long</c>
         /// by the given amount.
         /// </summary>
         /// <param name="id">a statistic id obtained with <see cref="#nameToId" />
         /// or <see cref="#StatisticsType#nameToId" /> </param>
         /// <param name="delta">the value of the statistic after it has been incremented</param>
         /// <returns>the value of the statistic after it has been incremented</returns>
         /// <exception cref="IllegalArgumentException">
         /// If the id is invalid.
         /// </exception>
         virtual System::Int64 IncLong(System::Int32 id, System::Int64 delta);


         /// <summary>
         /// Increments the value of the described statistic of type <c>long</c>
         /// by the given amount.
         /// </summary>
         /// <param name="descriptor">a statistic descriptor obtained with <see cref="#nameToDescriptor" />
         /// or <see cref="StatisticsType#nameToDescriptor" /> </param>
         /// <param name="delta">change value to be added</param>
         /// <returns>the value of the statistic after it has been incremented</returns>
         /// <exception cref="IllegalArgumentException">
         /// If no statistic exists for the given <c>descriptor</c> or
         /// if the described statistic is not of
         /// type <c>long</c>.
         /// </exception>
         virtual System::Int64 IncLong(StatisticDescriptor^ descriptor, System::Int64 delta);

         /// <summary>
         /// Increments the value of the statistic of type <c>long</c> with
         /// the given name by a given amount.
         /// </summary>
         /// <param name="name">statistic name</param>
         /// <param name="delta">change value to be added</param>
         /// <returns>the value of the statistic after it has been incremented</returns>
         /// <exception cref="IllegalArgumentException">
         /// If no statistic exists named <c>name</c> or
         /// if the statistic with name <c>name</c> is not of
         /// type <c>long</c>.
         /// </exception>
         virtual System::Int64 IncLong(String^ name, System::Int64 delta);


         /// <summary>
         /// Increments the value of the identified statistic of type <c>double</c>
         /// by the given amount.
         /// </summary>
         /// <param name="id">a statistic id obtained with <see cref="#nameToId" />
         /// or <see cref="#StatisticsType#nameToId" /> </param>
         /// <param name="delta">the value of the statistic after it has been incremented</param>
         /// <returns>the value of the statistic after it has been incremented</returns>
         /// <exception cref="IllegalArgumentException">
         /// If the id is invalid.
         /// </exception>
         virtual double IncDouble(System::Int32 id, double delta);

         /// <summary>
         /// Increments the value of the described statistic of type <c>double</c>
         /// by the given amount.
         /// </summary>
         /// <param name="descriptor">a statistic descriptor obtained with <see cref="#nameToDescriptor" />
         /// or <see cref="StatisticsType#nameToDescriptor" /> </param>
         /// <param name="delta">change value to be added</param>
         /// <returns>the value of the statistic after it has been incremented</returns>
         /// <exception cref="IllegalArgumentException">
         /// If no statistic exists for the given <c>descriptor</c> or
         /// if the described statistic is not of
         /// type <c>double</c>.
         /// </exception>
         virtual double IncDouble(StatisticDescriptor^ descriptor, double delta);

         /// <summary>
         /// Increments the value of the statistic of type <c>double</c> with
         /// the given name by a given amount.
         /// </summary>
         /// <param name="name">statistic name</param>
         /// <param name="delta">change value to be added</param>
         /// <returns>the value of the statistic after it has been incremented</returns>
         /// <exception cref="IllegalArgumentException">
         /// If no statistic exists named <c>name</c> or
         /// if the statistic with name <c>name</c> is not of
         /// type <c>double</c>.
         /// </exception>
         virtual double IncDouble(String^ name, double delta);

         internal:
           /// <summary>
           /// Internal factory function to wrap a native object pointer inside
           /// this managed class, with null pointer check.
           /// </summary>
           /// <param name="nativeptr">native object pointer</param>
           /// <returns>
           /// the managed wrapper object, or null if the native pointer is null.
           /// </returns>
          inline static Statistics^ Create(
          apache::geode::statistics::Statistics* nativeptr )
          {
          return __nullptr == nativeptr ? nullptr :
            gcnew Statistics( nativeptr );
          }

         private:
           /// <summary>
           /// Private constructor to wrap a native object pointer
           /// </summary>
           /// <param name="nativeptr">The native object pointer</param>
          inline Statistics( apache::geode::statistics::Statistics* nativeptr )
          : m_nativeptr( nativeptr )
          {
          }
        private:
          apache::geode::statistics::Statistics* m_nativeptr;

      };
    }  // namespace Client
  }  // namespace Geode
}  // namespace Apache


