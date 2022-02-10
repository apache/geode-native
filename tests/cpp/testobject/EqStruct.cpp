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

#include "EqStruct.hpp"

#include <sstream>

#include <geode/DataInput.hpp>
#include <geode/DataOutput.hpp>

namespace testobject {

EqStruct::EqStruct(int index) {
  myIndex = index;  // index
  state = "1";

  timestamp = std::chrono::system_clock::now().time_since_epoch().count();
  executedPriceSum = 5.5;
  cxlQty = 10;
  isSyntheticOrder = 0;
  availQty = 100;
  positionQty = 10.0;
  isRestricted = 1;
  demandInd = "ASDSAD";
  side = "16";
  orderQty = 3000;
  price = 78.9;
  ordType = "D";
  stopPx = 22.3;
  senderCompID = "dsafdsf";
  tarCompID = "dsafsadfsaf";
  tarSubID = "rwetwj";
  handlInst = "M N";
  orderID = "sample";
  timeInForce = "4";
  clOrdID = "sample";
  orderCapacity = "6";
  cumQty = 0;
  symbol = "MSFT";
  symbolSfx = "0";
  execInst = "A";
  oldClOrdID = "";
  pegDifference = 0.1;
  discretionInst = "G";
  discretionOffset = 300.0;
  financeInd = "dsagfdsa";
  securityID = "MSFT.O";
  targetCompID = "LBLB";
  targetSubID = "EQUITY";
  isDoneForDay = 1;
  revisionSeqNum = 140;
  replaceQty = 0;
  usedClientAvailability = 45;
  clientAvailabilityKey = "UUUU";
  isIrregularSettlmnt = 1;

  var1 = "abcdefghijklmnopqrstuvwxyz";
  var2 = "abcdefghijklmnopqrstuvwxyz";
  var3 = "abcdefghijklmnopqrstuvwxyz";
  var4 = "abcdefghijklmnopqrstuvwxyz";
  var5 = "abcdefghijklmnopqrstuvwxyz";
  var6 = "abcdefghijklmnopqrstuvwxyz";
  var7 = "abcdefghijklmnopqrstuvwxyz";
  var8 = "abcdefghijklmnopqrstuvwxyz";
  var9 = "abcdefghijklmnopqrstuvwxyz";
}

void EqStruct::toData(apache::geode::client::DataOutput& out) const {
  // Strings
  out.writeUTF(state);
  out.writeUTF(demandInd);
  out.writeUTF(side);
  out.writeUTF(ordType);
  out.writeUTF(senderCompID);
  out.writeUTF(tarCompID);
  out.writeUTF(tarSubID);
  out.writeUTF(handlInst);
  out.writeUTF(orderID);
  out.writeUTF(timeInForce);
  out.writeUTF(clOrdID);
  out.writeUTF(orderCapacity);
  out.writeUTF(symbol);
  out.writeUTF(symbolSfx);
  out.writeUTF(execInst);
  out.writeUTF(oldClOrdID);
  out.writeUTF(discretionInst);
  out.writeUTF(financeInd);
  out.writeUTF(securityID);
  out.writeUTF(targetCompID);
  out.writeUTF(targetSubID);
  out.writeUTF(clientAvailabilityKey);
  out.writeUTF(var1);
  out.writeUTF(var2);
  out.writeUTF(var3);
  out.writeUTF(var4);
  out.writeUTF(var5);
  out.writeUTF(var6);
  out.writeUTF(var7);
  out.writeUTF(var8);
  out.writeUTF(var9);

  // ints
  out.writeInt(myIndex);
  out.writeInt(cxlQty);
  out.writeInt(isSyntheticOrder);
  out.writeInt(isRestricted);
  out.writeInt(orderQty);
  out.writeInt(cumQty);
  out.writeInt(isDoneForDay);
  out.writeInt(revisionSeqNum);
  out.writeInt(replaceQty);
  out.writeInt(isIrregularSettlmnt);

  // longs
  out.writeInt(static_cast<int64_t>(timestamp));
  out.writeInt(static_cast<int64_t>(availQty));
  out.writeInt(static_cast<int64_t>(usedClientAvailability));

  // doubles
  out.writeDouble(executedPriceSum);
  out.writeDouble(positionQty);
  out.writeDouble(price);
  out.writeDouble(stopPx);
  out.writeDouble(pegDifference);
  out.writeDouble(discretionOffset);
}

void EqStruct::fromData(apache::geode::client::DataInput& in) {
  // Strings
  state = in.readUTF();
  demandInd = in.readUTF();
  side = in.readUTF();
  ordType = in.readUTF();
  senderCompID = in.readUTF();
  tarCompID = in.readUTF();
  tarSubID = in.readUTF();
  handlInst = in.readUTF();
  orderID = in.readUTF();
  timeInForce = in.readUTF();
  clOrdID = in.readUTF();
  orderCapacity = in.readUTF();
  symbol = in.readUTF();
  symbolSfx = in.readUTF();
  execInst = in.readUTF();
  oldClOrdID = in.readUTF();
  discretionInst = in.readUTF();
  financeInd = in.readUTF();
  securityID = in.readUTF();
  targetCompID = in.readUTF();
  targetSubID = in.readUTF();
  clientAvailabilityKey = in.readUTF();
  var1 = in.readUTF();
  var2 = in.readUTF();
  var3 = in.readUTF();
  var4 = in.readUTF();
  var5 = in.readUTF();
  var6 = in.readUTF();
  var7 = in.readUTF();
  var8 = in.readUTF();
  var9 = in.readUTF();

  // ints
  myIndex = in.readInt32();
  cxlQty = in.readInt32();
  isSyntheticOrder = in.readInt32();
  isRestricted = in.readInt32();
  orderQty = in.readInt32();
  cumQty = in.readInt32();
  isDoneForDay = in.readInt32();
  revisionSeqNum = in.readInt32();
  replaceQty = in.readInt32();
  isIrregularSettlmnt = in.readInt32();

  // longs
  timestamp = in.readInt64();
  availQty = in.readInt64();
  usedClientAvailability = in.readInt64();

  // doubles
  executedPriceSum = in.readDouble();
  positionQty = in.readDouble();
  price = in.readDouble();
  stopPx = in.readDouble();
  pegDifference = in.readDouble();
  discretionOffset = in.readDouble();
}
std::string EqStruct::toString() const {
  std::stringstream strm;

  strm << "EqStruct:[timestamp = " << timestamp << " myIndex = " << myIndex
       << " cxlQty = " << cxlQty << "]";
  return strm.str();
}

}  // namespace testobject
