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
#include "Portfolio.hpp"

#include <sstream>

namespace testobject {
const char* Portfolio::_secIds[] = {"SUN", "IBM",  "YHOO", "GOOG", "MSFT",
                                    "AOL", "APPL", "ORCL", "SAP",  "DELL"};

Portfolio::Portfolio(int32_t i, uint32_t size,
                     std::shared_ptr<CacheableStringArray> nm)
    : _names(nm) {
  _ID = i;
  _pkid = CacheableString::create(std::to_string(i));
  _status = (i % 2 == 0) ? "active" : "inactive";
  _type = CacheableString::create("_type" + std::to_string(i % 3));
  int numSecIds = sizeof(_secIds) / sizeof(char*);
  _position1 = std::make_shared<Position>(_secIds[Position::cnt % numSecIds],
                                          Position::cnt * 1000);
  if (i % 2 != 0) {
    _position2 = std::make_shared<Position>(_secIds[Position::cnt % numSecIds],
                                            Position::cnt * 1000);
  } else {
    _position2 = nullptr;
  }
  _positions = CacheableHashMap::create();
  _positions->emplace(
      CacheableString::create(_secIds[Position::cnt % numSecIds]), _position1);
  _newVal = new uint8_t[size + 1];
  memset(_newVal, 'B', size);
  _newVal[size] = '\0';
  _newValSize = size;
  _creationDate = CacheableDate::create(time(nullptr));
  _arrayNull = nullptr;
  _arrayZeroSize = nullptr;
}

Portfolio::~Portfolio() noexcept {
  if (_newVal) {
    delete[] _newVal;
    _newVal = nullptr;
  }
}

void Portfolio::toData(DataOutput& output) const {
  output.writeInt(_ID);
  output.writeObject(_pkid);
  output.writeObject(_position1);
  output.writeObject(_position2);
  output.writeObject(_positions);
  output.writeObject(_type);
  output.writeUTF(_status);
  output.writeObject(_names);
  output.writeBytes(_newVal, _newValSize + 1);
  output.writeObject(_creationDate);
  output.writeBytes(_arrayNull, 0);
  output.writeBytes(_arrayZeroSize, 0);
}

void Portfolio::fromData(DataInput& input) {
  _ID = input.readInt32();
  _pkid = std::dynamic_pointer_cast<CacheableString>(input.readObject());
  _position1 = std::dynamic_pointer_cast<Position>(input.readObject());
  _position2 = std::dynamic_pointer_cast<Position>(input.readObject());
  _positions = std::dynamic_pointer_cast<CacheableHashMap>(input.readObject());
  _type = std::dynamic_pointer_cast<CacheableString>(input.readObject());
  _status = input.readUTF();
  _names = std::dynamic_pointer_cast<CacheableStringArray>(input.readObject());
  input.readBytes(&_newVal, &_newValSize);
  _creationDate = std::dynamic_pointer_cast<CacheableDate>(input.readObject());
  int tmp = 0;
  input.readBytes(&_arrayNull, &tmp);
  input.readBytes(&_arrayZeroSize, &tmp);
}

std::string Portfolio::toString() const {
  std::stringstream result;
  result << "PortfolioObject: [ ID=" << _ID;

  result << " status=" << _status;

  result << " type=" << _type ? _type->toString() : "NULL";

  result << " pkid=" << _pkid ? _pkid->toString() : "NULL";

  result << " creation Date=" << _creationDate ? _creationDate->toString()
                                               : "NULL";

  result << "\t\t\t  P1: " << _position1 ? _position1->toString() : "NULL";

  result << "\t\t\t  P2: " << _position2 ? _position2->toString() : "NULL";

  return result.str();
}

}  // namespace testobject
