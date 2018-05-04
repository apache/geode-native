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


#include "SelectResultsIterator.hpp"
#include "impl/SafeConvert.hpp"

namespace Apache
{
  namespace Geode
  {
    namespace Client
    {
      using namespace System;

      generic<class TResult>
      TResult SelectResultsIterator<TResult>::Current::get( )
      {
        return m_results[m_index];
      }

      generic<class TResult>
      Object^ SelectResultsIterator<TResult>::Current2::get( )
      {
        return m_results[m_index];
      }

      generic<class TResult>
      bool SelectResultsIterator<TResult>::MoveNext( )
      {
         return ++m_index < m_results->Size;
      }

      generic<class TResult>
      void SelectResultsIterator<TResult>::Reset( )
      {
        m_index = -1;
      }

    }  // namespace Client
  }  // namespace Geode
}  // namespace Apache
