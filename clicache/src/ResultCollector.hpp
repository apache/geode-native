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
#include "IResultCollector.hpp"
#include "begin_native.hpp"
#include <geode/ResultCollector.hpp>
#include "end_native.hpp"

#include "native_shared_ptr.hpp"


using namespace System;
using namespace System::Collections::Generic;

namespace Apache
{
  namespace Geode
  {
    namespace Client
    {
     namespace native = apache::geode::client;

     generic<class TResult>
	   interface class IResultCollector;

      /// <summary>
      /// collect function execution results, default collector
      /// </summary>
     generic<class TResult>
     public ref class ResultCollector
       : public IResultCollector<TResult>
     {
     public:

        /// <summary>
        /// add result from a single function execution
        /// </summary>
        virtual void AddResult( const TResult rs );

        /// <summary>
        /// get result 
        /// </summary>
        virtual System::Collections::Generic::ICollection<TResult>^  GetResult(); 

        /// <summary>
        /// get result 
        /// </summary>
        virtual System::Collections::Generic::ICollection<TResult>^  GetResult(UInt32 timeout); 

        /// <summary>
        ///Call back provided to caller, which is called after function execution is
        ///complete and caller can retrieve results using getResult()
        /// </summary>
  //generic<class TKey>
	virtual void EndResults(); 

  //generic<class TKey>
  virtual void ClearResults();

      internal:

        /// <summary>
        /// Internal constructor to wrap a native object pointer
        /// </summary>
        /// <param name="nativeptr">The native object pointer</param>
        inline ResultCollector( native::ResultCollectorPtr nativeptr )
        {
           m_nativeptr = gcnew native_shared_ptr<native::ResultCollector>(nativeptr);
        }

        native_shared_ptr<native::ResultCollector>^ m_nativeptr;
      };
    }  // namespace Client
  }  // namespace Geode
}  // namespace Apache

