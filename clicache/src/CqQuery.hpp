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
#include "CqState.hpp"
#include "begin_native.hpp"
#include <geode/CqQuery.hpp>
#include "end_native.hpp"
#include "native_shared_ptr.hpp"

namespace Apache
{
  namespace Geode
  {
    namespace Client
    {
      using namespace System;

      namespace native = apache::geode::client;

      generic<class TResult>
      interface class ICqResults;

      generic<class TKey, class TResult>
      ref class CqAttributes;

      ref class CqStatistics;

      generic<class TKey, class TResult>
      ref class CqAttributesMutator;

      generic<class TResult>
      ref class Query;

      /// <summary>
      /// Class to encapsulate a continuous query (CQ).
      /// </summary>
      /// <remarks>
      /// A CqQuery is obtained from a QueryService which in turn is obtained
      /// from the Cache.
      /// This can be executed to return SelectResults which can be either
      /// a ResultSet or a StructSet, or it can be just registered on the
      /// java server without returning results immediately rather only
      /// the incremental results.
      ///
      /// This class is intentionally not thread-safe. So multiple threads
      /// should not operate on the same <c>CqQuery</c> object concurrently
      /// rather should have their own <c>CqQuery</c> objects.
      /// </remarks>
      generic<class TKey, class TResult>
      public ref class CqQuery sealed
      {
      public:

        /// <summary>
        /// Executes the Cq  Query on the cache server
        /// </summary>
        void Execute( );

        /// <summary>
        /// Executes the Cq Query on the cache server
        /// and returns the Cqresults.
        /// </summary>
        ICqResults<TResult>^ ExecuteWithInitialResults();

        /// <summary>
        /// Executes the Cq Query on the cache server
        /// with the specified timeout and returns the results.
        /// </summary>
        /// <param name="timeout">The time to wait for query response.
        /// </param>
        ICqResults<TResult>^ ExecuteWithInitialResults(TimeSpan timeout);

        /// <summary>
        /// Get the string for this cq query.
        /// </summary>
        property String^ QueryString
        {
          String^ get( );
        }

        /// <summary>
        /// Get the name for this cq query.
        /// </summary>
        property String^ Name
        {
          String^ get( );
        }

        /// <summary>
        /// Get the Attributes for this cq query.
        /// </summary>
        CqAttributes<TKey, TResult>^ GetCqAttributes();

        /// <summary>
        /// Get the Attributes Mutator for this cq query.
        /// </summary>
        CqAttributesMutator<TKey, TResult>^ GetCqAttributesMutator();

        /// <summary>
        /// Get the stats for this cq query.
        /// </summary>
        CqStatistics^ GetStatistics();

        /// <summary>
        /// Get the Query for this cq query.
        /// </summary>
        Query<TResult>^ GetQuery();

        /// <summary>
        /// stop the cq query
        /// </summary>
        void Stop( );

        /// <summary>
        /// stop the cq query
        /// </summary>
        void Close( );

        /// <summary>
        /// get the state of this cq query
        /// </summary>
        CqState GetState();

        /// <summary>
        /// Is this Cq in running state?
        /// </summary>
        bool IsRunning();

        /// <summary>
        /// Is this Cq in stopped state?
        /// </summary>
        bool IsStopped();

        /// <summary>
        /// Is this Cq in closed state?
        /// </summary>
        bool IsClosed();

      internal:

        /// <summary>
        /// Internal factory function to wrap a native object pointer inside
        /// this managed class with null pointer check.
        /// </summary>
        /// <param name="nativeptr">The native object pointer</param>
        /// <returns>
        /// The managed wrapper object; null if the native pointer is null.
        /// </returns>
        inline static CqQuery<TKey, TResult>^ Create( std::shared_ptr<native::CqQuery> nativeptr )
        {
          return __nullptr == nativeptr ? nullptr :
            gcnew  CqQuery<TKey, TResult>( nativeptr );
        }


      private:

        /// <summary>
        /// Private constructor to wrap a native object pointer
        /// </summary>
        /// <param name="nativeptr">The native object pointer</param>
        inline CqQuery( std::shared_ptr<native::CqQuery> nativeptr )
        {
          m_nativeptr = gcnew native_shared_ptr<native::CqQuery>(nativeptr);
        }


         native_shared_ptr<native::CqQuery>^ m_nativeptr;
      };
    }  // namespace Client
  }  // namespace Geode
}  // namespace Apache

