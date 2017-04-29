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



//#include "geode_includes.hpp"
#include "CacheableUndefined.hpp"
#include "DataOutput.hpp"
#include "DataInput.hpp"


using namespace System;

namespace Apache
{
  namespace Geode
  {
    namespace Client
    {

      // Region: IGeodeSerializable Members

      void CacheableUndefined::ToData(DataOutput^ output)
      {
      }

      IGeodeSerializable^ CacheableUndefined::FromData(DataInput^ input)
      {
        return this;
      }

      System::UInt32 CacheableUndefined::ObjectSize::get()
      {
        return static_cast<System::UInt32> (sizeof(CacheableUndefined^));
    }  // namespace Client
  }  // namespace Geode
}  // namespace Apache

 } //namespace 
