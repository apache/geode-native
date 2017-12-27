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
#include "native_shared_ptr.hpp"
#include "begin_native.hpp"
#include <geode/QueryService.hpp>
#include "end_native.hpp"

using namespace System;

namespace Apache
{
  namespace Geode
  {
    namespace Client
    {

      namespace native = apache::geode::client;

      generic<class TResult>
      ref class Query;

      generic<class TKey, class TResult>
      ref class CqQuery;

      generic<class TKey, class TResult>
      ref class CqAttributes;

      ref class CqServiceStatistics;

      /// <summary>
      /// Provides a query service.
      /// </summary>
      public ref class QueryService sealed
      {
      public:

        /// <summary>
        /// Get a <c>Query</c> object to enable querying.
        /// </summary>
        generic<class TResult>
        Query<TResult>^ NewQuery( String^ query );
        /// @nativeclient
        /// <summary>
        /// Get a <c>CqQuery</c> object to enable continuous querying.
        /// </summary>
        /// @endnativeclient
        generic<class TKey, class TResult>
        CqQuery<TKey, TResult>^ NewCq(String^ query, CqAttributes<TKey, TResult>^ cqAttr, bool isDurable);

        /// @nativeclient
        /// <summary>
        /// Get a <c>CqQuery</c> object to enable continuous querying.
        /// </summary>
        /// @endnativeclient
        generic<class TKey, class TResult>
        CqQuery<TKey, TResult>^ NewCq( String^ name, String^ query, CqAttributes<TKey, TResult>^ cqAttr, bool isDurable );
        /// @nativeclient
        /// <summary>
        /// Close all  <c>CqQuery</c> on this client.
        /// </summary>
        /// @endnativeclient
        void CloseCqs();

        /// @nativeclient
        /// <summary>
        /// Get all  <c>CqQuery</c> on this client.
        /// </summary>
        /// @endnativeclient
        generic<class TKey, class TResult>
        array<CqQuery<TKey, TResult>^>^ GetCqs();

        /// @nativeclient
        /// <summary>
        /// Get the  <c>CqQuery</c> with the given name on this client.
        /// </summary>
        /// @endnativeclient
        generic<class TKey, class TResult>
        CqQuery<TKey, TResult>^ GetCq(String^ name);

        /// @nativeclient
        /// <summary>
        /// Get the  <c>CqQuery</c> with the given name on this client.
        /// </summary>
        /// @endnativeclient
        void ExecuteCqs();

        /// @nativeclient
        /// <summary>
        /// Stop all  <c>CqQuery</c>  on this client.
        /// </summary>
        /// @endnativeclient
        void StopCqs();

        /// @nativeclient
        /// <summary>
        /// Get <c>CqServiceStatistics</c>  on this client.
        /// </summary>
        /// @endnativeclient
        CqServiceStatistics^ GetCqStatistics();

        /// @nativeclient
        /// <summary>
        /// Get all durableCq nanes from server for this client.
        /// </summary>
        /// @endnativeclient
        System::Collections::Generic::List<String^>^ GetAllDurableCqsFromServer();

      internal:

        /// <summary>
        /// Internal factory function to wrap a native object pointer inside
        /// this managed class with null pointer check.
        /// </summary>
        /// <param name="nativeptr">The native object pointer</param>
        /// <returns>
        /// The managed wrapper object; null if the native pointer is null.
        /// </returns>
        inline static QueryService^ Create(std::shared_ptr<native::QueryService> nativeptr )
        {
          return __nullptr == nativeptr ? nullptr :
            gcnew QueryService( nativeptr );
        }


      private:

        /// <summary>
        /// Private constructor to wrap a native object pointer
        /// </summary>
        /// <param name="nativeptr">The native object pointer</param>
        inline QueryService(std::shared_ptr<native::QueryService> nativeptr)
        {
           m_nativeptr = gcnew native_shared_ptr<native::QueryService>(nativeptr);
        }

        native_shared_ptr<native::QueryService>^ m_nativeptr;
      };
    }  // namespace Client
  }  // namespace Geode
}  // namespace Apache
