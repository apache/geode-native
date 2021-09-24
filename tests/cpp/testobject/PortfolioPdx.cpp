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

const char* PortfolioPdx::_secIds[] = {"SUN", "IBM",  "YHOO", "GOOG", "MSFT",
                                       "AOL", "APPL", "ORCL", "SAP",  "DELL"};

PortfolioPdx::PortfolioPdx(int32_t i, int32_t size, std::vector<std::string> nm)
    : _names(nm) {
  id = i;

  pkid = std::to_string(i);

  _status = (i % 2 == 0) ? "active" : "inactive";

  _type = "_type" + std::to_string(i % 3);

  int numSecIds = sizeof(_secIds) / sizeof(char*);
  _position1 = std::make_shared<PositionPdx>(
      _secIds[PositionPdx::cnt % numSecIds], PositionPdx::cnt * 1000);
  if (i % 2 != 0) {
    _position2 = std::make_shared<PositionPdx>(
        _secIds[PositionPdx::cnt % numSecIds], PositionPdx::cnt * 1000);
  } else {
    _position2 = nullptr;
  }
  _positions = CacheableHashMap::create();
  _positions->emplace(
      CacheableString::create(_secIds[PositionPdx::cnt % numSecIds]),
      _position1);

  if (size > 0) {
    _newVal = std::vector<int8_t>(size);
    for (int index = 0; index < size; index++) {
      _newVal[index] = static_cast<int8_t>('B');
    }
  }
  _newValSize = size;

  time_t timeVal = 1310447869;
  _creationDate = CacheableDate::create(timeVal);
  _arrayZeroSize = std::vector<int8_t>(0);
}

void PortfolioPdx::toData(PdxWriter& pw) const {
  pw.writeInt("ID", id);
  pw.markIdentityField("ID");

  pw.writeString("pkid", pkid);
  pw.markIdentityField("pkid");

  pw.writeObject("_position1", _position1);
  pw.markIdentityField("_position1");

  pw.writeObject("_position2", _position2);
  pw.markIdentityField("_position2");

  pw.writeObject("_positions", _positions);
  pw.markIdentityField("_positions");

  pw.writeString("_type", _type);
  pw.markIdentityField("_type");

  pw.writeString("_status", _status);
  pw.markIdentityField("_status");

  pw.writeStringArray("_names", _names);
  pw.markIdentityField("_names");

  pw.writeByteArray("_newVal", _newVal);
  pw.markIdentityField("_newVal");

  pw.writeDate("_creationDate", _creationDate);
  pw.markIdentityField("_creationDate");

  pw.writeByteArray("_arrayNull", _arrayNull);
  pw.writeByteArray("_arrayZeroSize", _arrayZeroSize);
}

void PortfolioPdx::fromData(PdxReader& pr) {
  id = pr.readInt("ID");
  pkid = pr.readString("pkid");

  _position1 =
      std::dynamic_pointer_cast<PositionPdx>(pr.readObject("_position1"));
  _position2 =
      std::dynamic_pointer_cast<PositionPdx>(pr.readObject("_position2"));
  _positions =
      std::dynamic_pointer_cast<CacheableHashMap>(pr.readObject("_positions"));
  _type = pr.readString("_type");
  _status = pr.readString("_status");

  _names = pr.readStringArray("_names");
  _newVal = pr.readByteArray("_newVal");
  _creationDate = pr.readDate("_creationDate");
  _arrayNull = pr.readByteArray("_arrayNull");
  _arrayZeroSize = pr.readByteArray("_arrayZeroSize");
}
std::string PortfolioPdx::toString() const {
  LOGINFO("PortfolioPdx::toString() Start");
  char idbuf[1024];
  sprintf(idbuf, "PortfolioPdxObject: [ id=%d ]", id);

  char pkidbuf[1024];
  sprintf(pkidbuf, " _status=%s _type=%s pkid=%s\n", this->_status.c_str(),
          this->_type.c_str(), this->pkid.c_str());

  char _position1buf[2048];
  if (_position1 != nullptr) {
    sprintf(_position1buf, "\t\t\t  P1: %s", _position1->toString().c_str());
  } else {
    sprintf(_position1buf, "\t\t\t  P1: %s", "NULL");
  }
  char _position2buf[2048];
  if (_position2 != nullptr) {
    sprintf(_position2buf, " P2: %s", _position2->toString().c_str());
  } else {
    sprintf(_position2buf, " P2: %s ]", "NULL");
  }
  char creationdatebuf[2048];
  if (_creationDate != nullptr) {
    sprintf(creationdatebuf, "creation Date %s",
            _creationDate->toString().c_str());
  } else {
    sprintf(creationdatebuf, "creation Date %s", "NULL");
  }

  char stringBuf[9000];
  snprintf(stringBuf, sizeof(stringBuf), "%s%s%s%s%s", idbuf, pkidbuf,
           creationdatebuf, _position1buf, _position2buf);
  return stringBuf;
}

}  // namespace testobject
