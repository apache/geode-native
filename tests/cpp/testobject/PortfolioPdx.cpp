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
#include "PortfolioPdx.hpp"

#include <util/Log.hpp>

namespace testobject {

using apache::geode::client::CacheableDate;
using apache::geode::client::CacheableHashMap;
using apache::geode::client::CacheableString;

const char* PortfolioPdx::secIds[] = {"SUN", "IBM",  "YHOO", "GOOG", "MSFT",
                                      "AOL", "APPL", "ORCL", "SAP",  "DELL"};

PortfolioPdx::PortfolioPdx(int32_t i, int32_t size, std::vector<std::string> nm)
    : names(nm) {
  id = i;

  pkid = std::to_string(i);

  status = (i % 2 == 0) ? "active" : "inactive";

  type = "type" + std::to_string(i % 3);

  int numSecIds = sizeof(secIds) / sizeof(char*);
  position1 = std::make_shared<PositionPdx>(
      secIds[PositionPdx::cnt % numSecIds], PositionPdx::cnt * 1000);
  if (i % 2 != 0) {
    position2 = std::make_shared<PositionPdx>(
        secIds[PositionPdx::cnt % numSecIds], PositionPdx::cnt * 1000);
  } else {
    position2 = nullptr;
  }
  positions = CacheableHashMap::create();
  positions->emplace(
      CacheableString::create(secIds[PositionPdx::cnt % numSecIds]), position1);

  if (size > 0) {
    newVal = std::vector<int8_t>(size);
    for (int index = 0; index < size; index++) {
      newVal[index] = static_cast<int8_t>('B');
    }
  }
  newValSize = size;

  time_t timeVal = 1310447869;
  creationDate = CacheableDate::create(timeVal);
  arrayZeroSize = std::vector<int8_t>(0);
}

void PortfolioPdx::toData(PdxWriter& pw) const {
  pw.writeInt("ID", id);
  pw.markIdentityField("ID");

  pw.writeString("pkid", pkid);
  pw.markIdentityField("pkid");

  pw.writeObject("position1", position1);
  pw.markIdentityField("position1");

  pw.writeObject("position2", position2);
  pw.markIdentityField("position2");

  pw.writeObject("positions", positions);
  pw.markIdentityField("positions");

  pw.writeString("type", type);
  pw.markIdentityField("type");

  pw.writeString("status", status);
  pw.markIdentityField("status");

  pw.writeStringArray("names", names);
  pw.markIdentityField("names");

  pw.writeByteArray("newVal", newVal);
  pw.markIdentityField("newVal");

  pw.writeDate("creationDate", creationDate);
  pw.markIdentityField("creationDate");

  pw.writeByteArray("arrayNull", arrayNull);
  pw.writeByteArray("arrayZeroSize", arrayZeroSize);
}

void PortfolioPdx::fromData(PdxReader& pr) {
  id = pr.readInt("ID");
  pkid = pr.readString("pkid");

  position1 =
      std::dynamic_pointer_cast<PositionPdx>(pr.readObject("position1"));
  position2 =
      std::dynamic_pointer_cast<PositionPdx>(pr.readObject("position2"));
  positions =
      std::dynamic_pointer_cast<CacheableHashMap>(pr.readObject("positions"));
  type = pr.readString("type");
  status = pr.readString("status");

  names = pr.readStringArray("names");
  newVal = pr.readByteArray("newVal");
  creationDate = pr.readDate("creationDate");
  arrayNull = pr.readByteArray("arrayNull");
  arrayZeroSize = pr.readByteArray("arrayZeroSize");
}
std::string PortfolioPdx::toString() const {
  LOG_INFO("PortfolioPdx::toString() Start");
  char idbuf[1024];
  sprintf(idbuf, "PortfolioPdxObject: [ id=%d ]", id);

  char pkidbuf[1024];
  sprintf(pkidbuf, " status=%s type=%s pkid=%s\n", this->status.c_str(),
          this->type.c_str(), this->pkid.c_str());

  char position1buf[2048];
  if (position1 != nullptr) {
    sprintf(position1buf, "\t\t\t  P1: %s", position1->toString().c_str());
  } else {
    sprintf(position1buf, "\t\t\t  P1: %s", "NULL");
  }
  char position2buf[2048];
  if (position2 != nullptr) {
    sprintf(position2buf, " P2: %s", position2->toString().c_str());
  } else {
    sprintf(position2buf, " P2: %s ]", "NULL");
  }
  char creationdatebuf[2048];
  if (creationDate != nullptr) {
    sprintf(creationdatebuf, "creation Date %s",
            creationDate->toString().c_str());
  } else {
    sprintf(creationdatebuf, "creation Date %s", "NULL");
  }

  char stringBuf[9000];
  snprintf(stringBuf, sizeof(stringBuf), "%s%s%s%s%s", idbuf, pkidbuf,
           creationdatebuf, position1buf, position2buf);
  return stringBuf;
}

}  // namespace testobject
