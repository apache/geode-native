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
#include "PositionPdx.hpp"

#include <wchar.h>

#include <cwchar>

namespace testobject {

int32_t PositionPdx::cnt = 0;

PositionPdx::PositionPdx() { init(); }

PositionPdx::PositionPdx(const char* id, int32_t out) {
  init();

  secId = std::string(id);

  qty = out * (cnt % 2 == 0 ? 10.0 : 100.0);
  mktValue = qty * 1.2345998;
  sharesOutstanding = out;
  secType = "a";

  pid = cnt++;
}

// This constructor is just for some internal data validation test
PositionPdx::PositionPdx(int32_t iForExactVal) {
  init();

  secId = std::string('a', iForExactVal + 1);

  qty = (iForExactVal % 2 == 0 ? 1000 : 100);
  mktValue = qty * 2;
  sharesOutstanding = iForExactVal;
  secType = "a";
  pid = iForExactVal;
}

void PositionPdx::init() {
  avg20DaysVol = 0;
  bondRating = "";
  convRatio = 0.0;
  country = "";
  delta = 0.0;
  industry = 0;
  issuer = 0;
  mktValue = 0.0;
  qty = 0.0;
  secId = "";
  secLinks = "";
  secType = "";
  sharesOutstanding = 0;
  underlyer = "";
  volatility = 0;
  pid = 0;
}

void PositionPdx::toData(PdxWriter& pw) const {
  pw.writeLong("avg20DaysVol", avg20DaysVol);
  pw.markIdentityField("avg20DaysVol");

  pw.writeString("bondRating", bondRating);
  pw.markIdentityField("bondRating");

  pw.writeDouble("convRatio", convRatio);
  pw.markIdentityField("convRatio");

  pw.writeString("country", country);
  pw.markIdentityField("country");

  pw.writeDouble("delta", delta);
  pw.markIdentityField("delta");

  pw.writeLong("industry", industry);
  pw.markIdentityField("industry");

  pw.writeLong("issuer", issuer);
  pw.markIdentityField("issuer");

  pw.writeDouble("mktValue", mktValue);
  pw.markIdentityField("mktValue");

  pw.writeDouble("qty", qty);
  pw.markIdentityField("qty");

  pw.writeString("secId", secId);
  pw.markIdentityField("secId");

  pw.writeString("secLinks", secLinks);
  pw.markIdentityField("secLinks");

  pw.writeString("secType", secType);
  pw.markIdentityField("secType");

  pw.writeInt("sharesOutstanding", sharesOutstanding);
  pw.markIdentityField("sharesOutstanding");

  pw.writeString("underlyer", underlyer);
  pw.markIdentityField("underlyer");

  pw.writeLong("volatility", volatility);
  pw.markIdentityField("volatility");

  pw.writeInt("pid", pid);
  pw.markIdentityField("pid");
}

void PositionPdx::fromData(PdxReader& pr) {
  avg20DaysVol = pr.readLong("avg20DaysVol");
  bondRating = pr.readString("bondRating");
  convRatio = pr.readDouble("convRatio");
  country = pr.readString("country");
  delta = pr.readDouble("delta");
  industry = pr.readLong("industry");
  issuer = pr.readLong("issuer");
  mktValue = pr.readDouble("mktValue");
  qty = pr.readDouble("qty");
  secId = pr.readString("secId");
  secLinks = pr.readString("secLinks");
  secType = pr.readString("secType");
  sharesOutstanding = pr.readInt("sharesOutstanding");
  underlyer = pr.readString("underlyer");
  volatility = pr.readLong("volatility");
  pid = pr.readInt("pid");
}
std::string PositionPdx::toString() const {
  char buf[1024];
  return "PositionPdx Object:[ id=" + std::to_string(pid) + "]";
}

}  // namespace testobject
