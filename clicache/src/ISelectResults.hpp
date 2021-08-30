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
#include <geode/SelectResults.hpp>
#include "end_native.hpp"


#include "ISerializable.hpp"

using namespace System;

namespace Apache
{
  namespace Geode
  {
    namespace Client
    {

      generic<class TResult>
      ref class SelectResultsIterator;

      /// <summary>
      /// Interface to encapsulate a select query result set.
      /// </summary>
      generic<class TResult>
      public interface class ISelectResults
        : public System::Collections::Generic::IEnumerable<TResult>
      {
      public:

        /// <summary>
        /// The size of the <c>ISelectResults</c>.
        /// </summary>
        property size_t Size
        {
          size_t get( );
        }

        /// <summary>
        /// Get an object at the given index.
        /// </summary>
        property TResult default[ size_t ]
        {
          TResult get( size_t index );
        }

      };
    }  // namespace Client
  }  // namespace Geode
}  // namespace Apache

