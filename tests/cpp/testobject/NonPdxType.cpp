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
/*
 * NonPdxType.cpp
 *
 *  Created on: Apr 30, 2012
 *      Author: vrao
 */

#include "NonPdxType.hpp"

namespace PdxTests {

template <typename T1, typename T2>
bool PdxTests::NonPdxType::genericValCompare(T1 value1, T2 value2) const {
  if (value1 != value2) return false;
  LOG_INFO("NonPdxType::genericValCompare");
  return true;
}

template <typename T1, typename T2, typename L>
bool PdxTests::NonPdxType::genericCompare(T1* value1, T2* value2,
                                          L length) const {
  L i = 0;
  while (i < length) {
    if (value1[i] != value2[i]) {
      return false;
    } else {
      i++;
    }
  }
  LOG_INFO("NonPdxType::genericCompareArray");
  return true;
}

template <typename T1, typename T2>
bool PdxTests::NonPdxType::generic2DCompare(T1** value1, T2** value2,
                                            int length, int* arrLengths) const {
  LOG_INFO("generic2DCompare length = %d ", length);
  LOG_INFO("generic2DCompare value1 = %d \t value2", value1[0][0],
           value2[0][0]);
  LOG_INFO("generic2DCompare value1 = %d \t value2", value1[1][0],
           value2[1][0]);
  LOG_INFO("generic2DCompare value1 = %d \t value2", value1[1][1],
           value2[1][1]);
  for (int j = 0; j < length; j++) {
    LOG_INFO("generic2DCompare arrlength0 = %d ", arrLengths[j]);
    for (int k = 0; k < arrLengths[j]; k++) {
      LOG_INFO("generic2DCompare arrlength = %d ", arrLengths[j]);
      LOG_INFO("generic2DCompare value1 = %d \t value2 = %d ", value1[j][k],
               value2[j][k]);
      if (value1[j][k] != value2[j][k]) return false;
    }
  }
  LOG_INFO("NonPdxType::generic2DCompare");
  return true;
}

bool PdxTests::NonPdxType::selfCheck() { return false; }

bool PdxTests::NonPdxType::equals(PdxTests::NonPdxType& other,
                                  bool isPdxReadSerialized) const {
  NonPdxType* ot = dynamic_cast<NonPdxType*>(&other);
  if (!ot) {
    return false;
  }
  if (ot == this) {
    return true;
  }
  genericValCompare(ot->m_int32, m_int32);
  genericValCompare(ot->m_bool, m_bool);
  genericValCompare(ot->m_byte, m_byte);
  genericValCompare(ot->m_int16, m_int16);
  genericValCompare(ot->m_long, m_long);
  genericValCompare(ot->m_float, m_float);
  genericValCompare(ot->m_double, m_double);
  genericValCompare(ot->m_sbyte, m_sbyte);
  genericValCompare(ot->m_uint16, m_uint16);
  genericValCompare(ot->m_uint32, m_uint32);
  genericValCompare(ot->m_ulong, m_ulong);
  genericValCompare(ot->m_char, m_char);
  if (ot->m_string != m_string) {
    return false;
  }
  genericCompare(ot->m_byteArray.data(), m_byteArray.data(),
                 m_byteArray.size());
  genericCompare(ot->m_int16Array.data(), m_int16Array.data(),
                 m_int16Array.size());
  genericCompare(ot->m_int32Array.data(), m_int32Array.data(),
                 m_int32Array.size());
  genericCompare(ot->m_longArray.data(), m_longArray.data(),
                 m_longArray.size());
  genericCompare(ot->m_floatArray.data(), m_floatArray.data(),
                 m_floatArray.size());
  genericCompare(ot->m_doubleArray.data(), m_doubleArray.data(),
                 m_doubleArray.size());
  genericCompare(ot->m_uint32Array.data(), m_uint32Array.data(),
                 m_uint32Array.size());
  genericCompare(ot->m_ulongArray.data(), m_ulongArray.data(),
                 m_ulongArray.size());
  genericCompare(ot->m_uint16Array.data(), m_uint16Array.data(),
                 m_uint16Array.size());
  genericCompare(ot->m_sbyteArray.data(), m_sbyteArray.data(),
                 m_sbyteArray.size());
  genericCompare(ot->m_charArray.data(), m_charArray.data(),
                 m_charArray.size());
  // generic2DCompare(ot->m_byteByteArray, m_byteByteArray, byteByteArrayLen,
  // lengthArr);

  LOG_INFO("NonPdxType::equals isPdxReadSerialized = %d", isPdxReadSerialized);

  if (!isPdxReadSerialized) {
    for (size_t i = 0; i < m_objectArray->size(); i++) {
      auto wrapper1 =
          std::dynamic_pointer_cast<PdxWrapper>(ot->m_objectArray->at(i));
      auto otherAddr1 =
          std::static_pointer_cast<NonPdxAddress>(wrapper1->getObject());
      auto wrapper2 =
          std::dynamic_pointer_cast<PdxWrapper>(m_objectArray->at(i));
      auto myAddr1 =
          std::static_pointer_cast<NonPdxAddress>(wrapper2->getObject());
      if (!otherAddr1->equals(*myAddr1)) return false;
    }
  }

  auto myenum = std::dynamic_pointer_cast<CacheableEnum>(m_pdxEnum);
  auto otenum = std::dynamic_pointer_cast<CacheableEnum>(ot->m_pdxEnum);
  if (myenum->getEnumOrdinal() != otenum->getEnumOrdinal()) return false;
  if (myenum->getEnumClassName() != otenum->getEnumClassName()) return false;
  if (myenum->getEnumName() != otenum->getEnumName()) return false;

  genericValCompare(ot->m_arraylist->size(), m_arraylist->size());
  for (size_t k = 0; k < m_arraylist->size(); k++) {
    genericValCompare(ot->m_arraylist->at(k), m_arraylist->at(k));
  }

  genericValCompare(ot->m_vector->size(), m_vector->size());
  for (size_t j = 0; j < m_vector->size(); j++) {
    genericValCompare(ot->m_vector->at(j), m_vector->at(j));
  }

  LOG_INFO("NonPdxType::equals done");
  return true;
}

}  // namespace PdxTests
