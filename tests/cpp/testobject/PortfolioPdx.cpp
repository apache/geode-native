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

#include <sstream>
#include <util/Log.hpp>

namespace testobject {
using apache::geode::client::CacheableDate;
using apache::geode::client::CacheableHashMap;
using apache::geode::client::CacheableString;

const char* PortfolioPdx::_secIds[] = {"SUN", "IBM",  "YHOO", "GOOG", "MSFT",
                                       "AOL", "APPL", "ORCL", "SAP",  "DELL"};

PortfolioPdx::PortfolioPdx(int32_t i, int32_t size, std::vector<std::string> nm)
    : _names(nm) {
  _id = i;

  _pkid = std::to_string(i);

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
  pw.writeInt("ID", _id);
  pw.markIdentityField("ID");

  pw.writeString("pkid", _pkid);
  pw.markIdentityField("pkid");

  pw.writeObject("position1", _position1);
  pw.markIdentityField("position1");

  pw.writeObject("position2", _position2);
  pw.markIdentityField("position2");

  pw.writeObject("positions", _positions);
  pw.markIdentityField("positions");

  pw.writeString("type", _type);
  pw.markIdentityField("type");

  pw.writeString("status", _status);
  pw.markIdentityField("status");

  pw.writeStringArray("names", _names);
  pw.markIdentityField("names");

  pw.writeByteArray("newVal", _newVal);
  pw.markIdentityField("newVal");

  pw.writeDate("creationDate", _creationDate);
  pw.markIdentityField("creationDate");

  pw.writeByteArray("arrayNull", _arrayNull);
  pw.writeByteArray("arrayZeroSize", _arrayZeroSize);
}

void PortfolioPdx::fromData(PdxReader& pr) {
  _id = pr.readInt("ID");
  _pkid = pr.readString("pkid");

  _position1 =
      std::dynamic_pointer_cast<PositionPdx>(pr.readObject("position1"));
  _position2 =
      std::dynamic_pointer_cast<PositionPdx>(pr.readObject("position2"));
  _positions =
      std::dynamic_pointer_cast<CacheableHashMap>(pr.readObject("positions"));
  _type = pr.readString("type");
  _status = pr.readString("status");

  _names = pr.readStringArray("names");
  _newVal = pr.readByteArray("newVal");
  _creationDate = pr.readDate("creationDate");
  _arrayNull = pr.readByteArray("arrayNull");
  _arrayZeroSize = pr.readByteArray("arrayZeroSize");
}
std::string PortfolioPdx::toString() const {
  std::stringstream result;
  auto suffix = "\n    ";
  result << "PortfolioPdxObject: [" << suffix;

  result << "id = " << _id << suffix;

  result << "status=" << _status << suffix;

  result << "type=" << _type << suffix;

  result << "pkid=" << _pkid << suffix;

  result << "creation Date="
         << (_creationDate ? _creationDate->toString() : "NULL") << suffix;

  result << "P1: " << (_position1 ? _position1->toString() : "NULL") << suffix;

  result << "P2: " << (_position2 ? _position2->toString() : "NULL") << "\n";

  result << "]";

  return result.str();
}

}  // namespace testobject
