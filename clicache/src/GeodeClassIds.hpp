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
#include <geode/GeodeTypeIds.hpp>
#include "end_native.hpp"



namespace Apache
{
  namespace Geode
  {
    namespace Client
    {

      struct PdxTypes
      {
        enum PdxTypesInternal
        {
          BOOLEAN,
          BYTE,
          CHAR,
          SHORT,
          INT,
          LONG,
          FLOAT,
          DOUBLE,
          DATE,
          STRING,
          OBJECT,
          BOOLEAN_ARRAY,
          CHAR_ARRAY,
          BYTE_ARRAY,
          SHORT_ARRAY,
          INT_ARRAY,
          LONG_ARRAY,
          FLOAT_ARRAY,
          DOUBLE_ARRAY,
          STRING_ARRAY,
          OBJECT_ARRAY,
          ARRAY_OF_BYTE_ARRAYS
        };
      };

      /// <summary>
      /// Static class containing the classIds of the built-in cacheable types.
      /// </summary>
      public ref class GeodeClassIds
      {
      public:

        /// <summary>
        /// ClassId of <c>Properties</c> class
        /// </summary>
        literal System::UInt32 Properties =
          apache::geode::client::GeodeTypeIds::Properties + 0x80000000;

        /// <summary>        
        /// ClassId of <c>CharArray</c> class
        /// </summary>
        literal System::UInt32 CharArray =
          apache::geode::client::GeodeTypeIds::CharArray + 0x80000000;

        /// <summary>
        /// ClassId of <c>BooleanArray</c> class
        /// </summary>
        literal System::UInt32 BooleanArray =
          apache::geode::client::GeodeTypeIds::BooleanArray + 0x80000000;

        /// <summary>
        /// ClassId of <c>RegionAttributes</c> class
        /// </summary>
        literal System::UInt32 RegionAttributes =
          apache::geode::client::GeodeTypeIds::RegionAttributes + 0x80000000;

        /// <summary>
        /// ClassId of <c>CacheableUndefined</c> class
        /// Implementation note: this has DSFID of FixedIDByte hence a
        /// different increment.
        /// </summary>
        literal System::UInt32 CacheableUndefined =
          apache::geode::client::GeodeTypeIds::CacheableUndefined + 0xa0000000;

        literal System::UInt32 EnumInfo =
          apache::geode::client::GeodeTypeIds::EnumInfo + 0xa0000000;

        /// <summary>
        /// ClassId of <c>Struct</c> class
        /// </summary>
        literal System::UInt32 Struct =
          apache::geode::client::GeodeTypeIds::Struct + 0x80000000;

        /// <summary>
        /// ClassId of <c>CacheableString</c> class
        /// </summary>
        literal System::UInt32 CacheableString =
          apache::geode::client::GeodeTypeIds::CacheableString + 0x80000000;

        /// <summary>
        /// ClassId of <c>CacheableString</c> class for huge strings
        /// </summary>
        literal System::UInt32 CacheableStringHuge =
          apache::geode::client::GeodeTypeIds::CacheableStringHuge + 0x80000000;

        /// <summary>
        /// ClassId of <c>CacheableBytes</c> class
        /// </summary>
        literal System::UInt32 CacheableBytes =
          apache::geode::client::GeodeTypeIds::CacheableBytes + 0x80000000;

        /// <summary>
        /// ClassId of <c>CacheableInt16Array</c> class
        /// </summary>
        literal System::UInt32 CacheableInt16Array =
          apache::geode::client::GeodeTypeIds::CacheableInt16Array + 0x80000000;

        /// <summary>
        /// ClassId of <c>CacheableInt32Array</c> class
        /// </summary>
        literal System::UInt32 CacheableInt32Array =
          apache::geode::client::GeodeTypeIds::CacheableInt32Array + 0x80000000;

        /// <summary>
        /// ClassId of <c>CacheableInt64Array</c> class
        /// </summary>
        literal System::UInt32 CacheableInt64Array =
          apache::geode::client::GeodeTypeIds::CacheableInt64Array + 0x80000000;

        /// <summary>
        /// ClassId of <c>CacheableFloatArray</c> class
        /// </summary>
        literal System::UInt32 CacheableFloatArray =
          apache::geode::client::GeodeTypeIds::CacheableFloatArray + 0x80000000;

        /// <summary>
        /// ClassId of <c>CacheableDoubleArray</c> class
        /// </summary>
        literal System::UInt32 CacheableDoubleArray =
          apache::geode::client::GeodeTypeIds::CacheableDoubleArray + 0x80000000;

        /// <summary>
        /// ClassId of <c>CacheableVector</c> class for object arrays
        /// </summary>
        literal System::UInt32 CacheableObjectArray =
          apache::geode::client::GeodeTypeIds::CacheableObjectArray + 0x80000000;

        /// <summary>
        /// ClassId of <c>CacheableBoolean</c> class
        /// </summary>
        literal System::UInt32 CacheableBoolean =
          apache::geode::client::GeodeTypeIds::CacheableBoolean + 0x80000000;

        /// <summary>
        /// ClassId of <c>CacheableInt16</c> class for wide-characters
        /// </summary>
        literal System::UInt32 CacheableCharacter =
          apache::geode::client::GeodeTypeIds::CacheableCharacter + 0x80000000;

        /// <summary>
        /// ClassId of <c>CacheableByte</c> class
        /// </summary>
        literal System::UInt32 CacheableByte =
          apache::geode::client::GeodeTypeIds::CacheableByte + 0x80000000;

        /// <summary>
        /// ClassId of <c>CacheableInt16</c> class
        /// </summary>
        literal System::UInt32 CacheableInt16 =
          apache::geode::client::GeodeTypeIds::CacheableInt16 + 0x80000000;

        /// <summary>
        /// ClassId of <c>CacheableInt32</c> class
        /// </summary>
        literal System::UInt32 CacheableInt32 =
          apache::geode::client::GeodeTypeIds::CacheableInt32 + 0x80000000;

        /// <summary>
        /// ClassId of <c>CacheableInt64</c> class
        /// </summary>
        literal System::UInt32 CacheableInt64 =
          apache::geode::client::GeodeTypeIds::CacheableInt64 + 0x80000000;

        /// <summary>
        /// ClassId of <c>CacheableFloat</c> class
        /// </summary>
        literal System::UInt32 CacheableFloat =
          apache::geode::client::GeodeTypeIds::CacheableFloat + 0x80000000;

        /// <summary>
        /// ClassId of <c>CacheableDouble</c> class
        /// </summary>
        literal System::UInt32 CacheableDouble =
          apache::geode::client::GeodeTypeIds::CacheableDouble + 0x80000000;

        /// <summary>
        /// ClassId of <c>CacheableDate</c> class
        /// </summary>
        literal System::UInt32 CacheableDate =
          apache::geode::client::GeodeTypeIds::CacheableDate + 0x80000000;

        /// <summary>
        /// ClassId of <c>CacheableFileName</c> class
        /// </summary>
        literal System::UInt32 CacheableFileName =
          apache::geode::client::GeodeTypeIds::CacheableFileName + 0x80000000;

        /// <summary>
        /// ClassId of <c>CacheableStringArray</c> class
        /// </summary>
        literal System::UInt32 CacheableStringArray =
          apache::geode::client::GeodeTypeIds::CacheableStringArray + 0x80000000;

        /// <summary>
        /// ClassId of <c>CacheableVector</c> class
        /// </summary>
        literal System::UInt32 CacheableVector =
          apache::geode::client::GeodeTypeIds::CacheableVector + 0x80000000;

        /// <summary>
        /// ClassId of <c>CacheableStack</c> class
        /// </summary>
        literal System::UInt32 CacheableStack =
          apache::geode::client::GeodeTypeIds::CacheableStack + 0x80000000;

        /// <summary>
        /// ClassId of <c>CacheableArrayList</c> class
        /// </summary>
        literal System::UInt32 CacheableArrayList =
          apache::geode::client::GeodeTypeIds::CacheableArrayList + 0x80000000;

        /// <summary>
        /// ClassId of <c>CacheableArrayList</c> class
        /// </summary>
        literal System::UInt32 CacheableLinkedList =
          apache::geode::client::GeodeTypeIds::CacheableLinkedList + 0x80000000;

        /// <summary>
        /// ClassId of <c>CacheableHashSet</c> class
        /// </summary>
        literal System::UInt32 CacheableHashSet =
          apache::geode::client::GeodeTypeIds::CacheableHashSet + 0x80000000;

        /// <summary>
        /// ClassId of <c>CacheableLinkedHashSet</c> class
        /// </summary>
        literal System::UInt32 CacheableLinkedHashSet =
          apache::geode::client::GeodeTypeIds::CacheableLinkedHashSet + 0x80000000;

        /// <summary>
        /// ClassId of <c>CacheableHashMap</c> class
        /// </summary>
        literal System::UInt32 CacheableHashMap =
          apache::geode::client::GeodeTypeIds::CacheableHashMap + 0x80000000;

        /// <summary>
        /// ClassId of <c>CacheableHashTable</c> class
        /// </summary>
        literal System::UInt32 CacheableHashTable =
          apache::geode::client::GeodeTypeIds::CacheableHashTable + 0x80000000;

        /// <summary>
        /// ClassId of <c>CacheableIdentityHashMap</c> class
        /// </summary>
        literal System::UInt32 CacheableIdentityHashMap =
          apache::geode::client::GeodeTypeIds::CacheableIdentityHashMap + 0x80000000;

        /// <summary>
        /// Not used.
        /// </summary>
        literal System::UInt32 CacheableTimeUnit =
          apache::geode::client::GeodeTypeIds::CacheableTimeUnit + 0x80000000;

        /// <summary>
        /// ClassId of <c>CacheableString</c> class for null strings
        /// </summary>
        literal System::UInt32 CacheableNullString =
          apache::geode::client::GeodeTypeIds::CacheableNullString + 0x80000000;

        /// <summary>
        /// ClassId of <c>CacheableString</c> class for ASCII strings
        /// </summary>
        literal System::UInt32 CacheableASCIIString =
          apache::geode::client::GeodeTypeIds::CacheableASCIIString + 0x80000000;

        /// <summary>
        /// ClassId of <c>CacheableString</c> class for huge ASCII strings
        /// </summary>
        literal System::UInt32 CacheableASCIIStringHuge =
          apache::geode::client::GeodeTypeIds::CacheableASCIIStringHuge + 0x80000000;


        // Built-in managed types.

        /// <summary>
        /// ClassId of <c>CacheableObject</c> class
        /// </summary>
        literal System::UInt32 CacheableManagedObject = 7 + 0x80000000;

        /// <summary>
        /// ClassId of <c>CacheableObjectXml</c> class
        /// </summary>
        literal System::UInt32 CacheableManagedObjectXml = 8 + 0x80000000;
      internal:

        literal System::UInt32 PdxType = apache::geode::client::GeodeTypeIds::PdxType + 0x80000000;

        literal System::UInt32 DATA_SERIALIZABLE = 45;
        literal System::UInt32 JAVA_CLASS = 43;

        //internal geode typeids..
        /*  literal Byte USERCLASS = 40;
          literal Byte USERMAP = 94;
          literal Byte USERCOLLECTION = 95;
          literal Byte ARRAYOFBYTEARRAYS = 91;
          literal Byte GEODEREGION =  98;

          literal Byte BOOLEAN_TYPE = 17;
          literal Byte CHARACTER_TYPE = 18;
          literal Byte BYTE_TYPE = 19;
          literal Byte SHORT_TYPE = 20;
          literal Byte INTEGER_TYPE = 21;
          literal Byte LONG_TYPE = 22;
          literal Byte FLOAT_TYPE = 23;
          literal Byte DOUBLE_TYPE = 24;
          literal Byte VOID_TYPE = 25;   */

        literal Byte PDX = 93;
        literal Byte PDX_ENUM = 94;

        literal Byte BYTE_SIZE = 1;

        literal Byte BOOLEAN_SIZE = 1;

        literal Byte CHAR_SIZE = 2;

        literal Byte SHORT_SIZE = 2;

        literal Byte INTEGER_SIZE = 4;

        literal Byte FLOAT_SIZE = 4;

        literal Byte LONG_SIZE = 8;

        literal Byte DOUBLE_SIZE = 8;

        literal Byte DATE_SIZE = 8;
      };
    }  // namespace Client
  }  // namespace Geode
}  // namespace Apache


