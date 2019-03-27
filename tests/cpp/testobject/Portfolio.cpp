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

namespace testobject {

const char* Portfolio::secIds[] = {"SUN", "IBM",  "YHOO", "GOOG", "MSFT",
                                   "AOL", "APPL", "ORCL", "SAP",  "DELL"};

Portfolio::Portfolio(int32_t i, uint32_t size,
                     std::shared_ptr<CacheableStringArray> nm)
    : names(nm) {
  ID = i;
  char pkidbuf[1024];
  sprintf(pkidbuf, "%d", i);
  pkid = CacheableString::create(pkidbuf);
  status = (i % 2 == 0) ? "active" : "inactive";
  char buf[100];
  sprintf(buf, "type%d", (i % 3));
  type = CacheableString::create(buf);
  int numSecIds = sizeof(secIds) / sizeof(char*);
  position1 = std::make_shared<Position>(secIds[Position::cnt % numSecIds],
                                         Position::cnt * 1000);
  if (i % 2 != 0) {
    position2 = std::make_shared<Position>(secIds[Position::cnt % numSecIds],
                                           Position::cnt * 1000);
  } else {
    position2 = nullptr;
  }
  positions = CacheableHashMap::create();
  positions->emplace(CacheableString::create(secIds[Position::cnt % numSecIds]),
                     position1);
  newVal = new uint8_t[size + 1];
  memset(newVal, 'B', size);
  newVal[size] = '\0';
  newValSize = size;
  creationDate = CacheableDate::create(time(nullptr));
  arrayNull = nullptr;
  arrayZeroSize = nullptr;
}

Portfolio::~Portfolio() noexcept {
  if (newVal) {
    delete[] newVal;
    newVal = nullptr;
  }
}

void Portfolio::toData(DataOutput& output) const {
  output.writeInt(ID);
  output.writeObject(pkid);
  output.writeObject(position1);
  output.writeObject(position2);
  output.writeObject(positions);
  output.writeObject(type);
  output.writeUTF(status);
  output.writeObject(names);
  output.writeBytes(newVal, newValSize + 1);
  output.writeObject(creationDate);
  output.writeBytes(arrayNull, 0);
  output.writeBytes(arrayZeroSize, 0);
}

void Portfolio::fromData(DataInput& input) {
  ID = input.readInt32();
  pkid = std::dynamic_pointer_cast<CacheableString>(input.readObject());
  position1 = std::dynamic_pointer_cast<Position>(input.readObject());
  position2 = std::dynamic_pointer_cast<Position>(input.readObject());
  positions = std::dynamic_pointer_cast<CacheableHashMap>(input.readObject());
  type = std::dynamic_pointer_cast<CacheableString>(input.readObject());
  status = input.readUTF();
  names = std::dynamic_pointer_cast<CacheableStringArray>(input.readObject());
  input.readBytes(&newVal, &newValSize);
  creationDate = std::dynamic_pointer_cast<CacheableDate>(input.readObject());
  int tmp = 0;
  input.readBytes(&arrayNull, &tmp);
  input.readBytes(&arrayZeroSize, &tmp);
}
std::string Portfolio::toString() const {
  char idbuf[1024];
  sprintf(idbuf, "PortfolioObject: [ ID=%d", ID);
  char pkidbuf[1024];
  if (pkid != nullptr) {
    sprintf(pkidbuf, " status=%s type=%s pkid=%s\n", this->status.c_str(),
            this->type->toString().c_str(), this->pkid->value().c_str());
  } else {
    sprintf(pkidbuf, " status=%s type=%s pkid=%s\n", this->status.c_str(),
            this->type->toString().c_str(), this->pkid->value().c_str());
  }
  char position1buf[2048];
  if (position1 != nullptr) {
    sprintf(position1buf, "\t\t\t  P1: %s", position1->toString().c_str());
  } else {
    sprintf(position1buf, "\t\t\t  P1: %s", "NULL");
  }
  char position2buf[2048];
  if (position2 != nullptr) {
    snprintf(position2buf, sizeof(position2buf), " P2: %s",
             position2->toString().c_str());
  } else {
    snprintf(position2buf, sizeof(position2buf), " P2: %s ]", "NULL");
  }
  char creationdatebuf[2048];
  if (creationDate != nullptr) {
    sprintf(creationdatebuf, "creation Date %s",
            creationDate->toString().c_str());
  } else {
    sprintf(creationdatebuf, "creation Date %s", "NULL");
  }

  char stringBuf[9000];
  snprintf(stringBuf, sizeof(stringBuf), "%.1024s%.1024s%.1024s%.2048s%.2048s",
           idbuf, pkidbuf, creationdatebuf, position1buf, position2buf);
  return stringBuf;
}

}  // namespace testobject
