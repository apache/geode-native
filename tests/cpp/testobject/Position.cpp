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
#include "Position.hpp"
#include <cwchar>
#include <wchar.h>

using namespace apache::geode::client;
using namespace testobject;

int32_t Position::cnt = 0;

Position::Position() { init(); }

Position::Position(const char* id, int32_t out) {
  init();
  secId = CacheableString::create(id);
  qty = out - (cnt % 2 == 0 ? 1000 : 100);
  mktValue = qty * 1.2345998;
  sharesOutstanding = out;
  secType = new wchar_t[(wcslen(L"a") + 1)];
  wcscpy(secType, L"a");
  pid = cnt++;
}

// This constructor is just for some internal data validation test
Position::Position(int32_t iForExactVal) {
  init();
  char* id = new char[iForExactVal + 2];
  for (int i = 0; i <= iForExactVal; i++) {
    id[i] = 'a';
  }
  id[iForExactVal + 1] = '\0';
  secId = CacheableString::create(id);
  delete[] id;
  qty = (iForExactVal % 2 == 0 ? 1000 : 100);
  mktValue = qty * 2;
  sharesOutstanding = iForExactVal;
  secType = new wchar_t[(wcslen(L"a") + 1)];
  wcscpy(secType, L"a");
  pid = iForExactVal;
}

Position::~Position() {
  if (secType != NULL) {
    // free(secType);
    delete[] secType;
  }
}

void Position::init() {
  avg20DaysVol = 0;
  bondRating = nullptr;
  convRatio = 0.0;
  country = nullptr;
  delta = 0.0;
  industry = 0;
  issuer = 0;
  mktValue = 0.0;
  qty = 0.0;
  secId = nullptr;
  secLinks = nullptr;
  secType = NULL;
  sharesOutstanding = 0;
  underlyer = nullptr;
  volatility = 0;
  pid = 0;
}

void Position::toData(apache::geode::client::DataOutput& output) const {
  output.writeInt(avg20DaysVol);
  output.writeObject(bondRating);
  output.writeDouble(convRatio);
  output.writeObject(country);
  output.writeDouble(delta);
  output.writeInt(industry);
  output.writeInt(issuer);
  output.writeDouble(mktValue);
  output.writeDouble(qty);
  output.writeObject(secId);
  output.writeObject(secLinks);
  output.writeUTF(secType);
  output.writeInt(sharesOutstanding);
  output.writeObject(underlyer);
  output.writeInt(volatility);
  output.writeInt(pid);
}

void Position::fromData(apache::geode::client::DataInput& input) {
  avg20DaysVol = input.readInt64();
  bondRating = input.readObject<CacheableString>();
  convRatio = input.readDouble();
  country = input.readObject<CacheableString>();
  delta = input.readDouble();
  industry = input.readInt64();
  issuer = input.readInt64();
  mktValue = input.readDouble();
  qty = input.readDouble();
  secId = input.readObject<CacheableString>();
  secLinks = input.readObject<CacheableString>();
  input.readUTF(&secType);
  sharesOutstanding = input.readInt32();
  underlyer = input.readObject<CacheableString>();
  volatility = input.readInt64();
  pid = input.readInt32();
}

CacheableStringPtr Position::toString() const {
  char buf[2048];
  sprintf(buf,
          "Position Object:[ secId=%s type=%ls sharesOutstanding=%d id=%d ]",
          secId->toString(), this->secType, this->sharesOutstanding, this->pid);
  return CacheableString::create(buf);
}
