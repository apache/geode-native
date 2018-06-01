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
#include "IDataSerializablePrimitive.hpp"


using namespace System;
using namespace System::Collections::Generic;

namespace Apache
{
  namespace Geode
  {
    namespace Client
    {

      /// <summary>
      /// A mutable <c>ISerializable</c> vector wrapper that can serve as
      /// a distributable object for caching.
      /// </summary>
      ref class CacheableStack
        : public IDataSerializablePrimitive
      {
      public:
        /// <summary>
        /// Allocates a new empty instance.
        /// </summary>
        inline CacheableStack(System::Collections::ICollection^ stack)
        { 
          m_stack = stack;
        }
                
        /// <summary>
        /// Static function to create a new empty instance.
        /// </summary>
        inline static CacheableStack^ Create()
        {
          return gcnew CacheableStack(gcnew System::Collections::Generic::Stack<Object^>());
        }

        /// <summary>
        /// Static function to create a new empty instance.
        /// </summary>
        inline static CacheableStack^ Create(System::Collections::ICollection^ stack)
        {
          return gcnew CacheableStack(stack);
        }

        
        
        // Region: ISerializable Members

        /// <summary>
        /// Serializes this object.
        /// </summary>
        /// <param name="output">
        /// the DataOutput object to use for serializing the object
        /// </param>
        virtual void ToData(DataOutput^ output);

        /// <summary>
        /// Deserialize this object, typical implementation should return
        /// the 'this' pointer.
        /// </summary>
        /// <param name="input">
        /// the DataInput stream to use for reading the object data
        /// </param>
        /// <returns>the deserialized object</returns>
        virtual void FromData(DataInput^ input);

        /// <summary>
        /// return the size of this object in bytes
        /// </summary>
        virtual property System::UInt64 ObjectSize
        {
          virtual System::UInt64 get();
        }

        /// <summary>
        /// Returns the classId of the instance being serialized.
        /// This is used by deserialization to determine what instance
        /// type to create and deserialize into.
        /// </summary>
        /// <returns>the classId</returns>
        property int8_t DsCode
        {
          virtual int8_t get();
        }

        virtual property System::Collections::ICollection^ Value
        {
          virtual System::Collections::ICollection^ get()
          {
            return m_stack;
          }
        }
        // End Region: ISerializable Members

        /// <summary>
        /// Factory function to register this class.
        /// </summary>
        static ISerializable^ CreateDeserializable()
        {
          return gcnew CacheableStack(gcnew System::Collections::Generic::Stack<Object^>());
        }

        private:
          System::Collections::ICollection^ m_stack;
      };
    }  // namespace Client
  }  // namespace Geode
}  // namespace Apache

