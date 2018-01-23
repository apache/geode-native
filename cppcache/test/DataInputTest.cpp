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

#include <string>  // for ::memset()

#include <stdint.h>
#define NOMINMAX
#include <cstdint>

#include <gtest/gtest.h>

#include <geode/CacheFactory.hpp>
#include <geode/DataInput.hpp>
#include <memory>
#include "ByteArrayFixture.hpp"
#include "DataInputInternal.hpp"
#include "DataOutputInternal.hpp"
#include "SerializationRegistry.hpp"

namespace {

using namespace apache::geode::client;

class DataInputUnderTest : public DataInputInternal {
 public:
  using DataInputInternal::DataInputInternal;

  virtual const SerializationRegistry &getSerializationRegistry()
      const override {
    return m_serializationRegistry;
  }

 private:
  SerializationRegistry m_serializationRegistry;
};

class TestDataInput {
 public:
  explicit TestDataInput(const char *str)
      : m_byteArray(ByteArray::fromString(str)),
        m_dataInput(m_byteArray.get(), m_byteArray.size()) {
    // NOP
  }

  explicit TestDataInput(const wchar_t *str)
      : m_byteArray(ByteArray::fromString(str)),
        m_dataInput(m_byteArray.get(), m_byteArray.size()) {
    // NOP
  }

  operator const DataInput &() const { return m_dataInput; }

  operator DataInput &() { return m_dataInput; }

  void read(uint8_t *value) { *value = m_dataInput.read(); }

  void read(int8_t *value) { *value = m_dataInput.read(); }

  void readBoolean(bool *value) { *value = m_dataInput.readBoolean(); }

  void readBytesOnly(uint8_t *buffer, uint32_t len) {
    m_dataInput.readBytesOnly(buffer, len);
  }

  void readBytesOnly(int8_t *buffer, uint32_t len) {
    m_dataInput.readBytesOnly(buffer, len);
  }

  void readBytes(uint8_t **buffer, int32_t *len) {
    m_dataInput.readBytes(buffer, len);
  }

  void readBytes(int8_t **buffer, int32_t *len) {
    m_dataInput.readBytes(buffer, len);
  }

  int16_t readInt16() { return m_dataInput.readInt16(); }

  int32_t readInt32() { return m_dataInput.readInt32(); }

  void readInt(uint64_t *value) { *value = m_dataInput.readInt64(); }

  int64_t readInt64() { return m_dataInput.readInt64(); }

  int32_t readArrayLen() { return m_dataInput.readArrayLen(); }

  int64_t readUnsignedVL() { return m_dataInput.readUnsignedVL(); }

  float readFloat() { return m_dataInput.readFloat(); }

  double readDouble() { return m_dataInput.readDouble(); }

  bool readNativeBool() { return m_dataInput.readNativeBool(); }

  int32_t readNativeInt32() { return m_dataInput.readNativeInt32(); }

  std::shared_ptr<Serializable> readDirectObject(int8_t typeId = -1) {
    return m_dataInput.readDirectObject(typeId);
  }

  std::shared_ptr<Serializable> readObject() {
    return m_dataInput.readObject();
  }

  void readObject(std::shared_ptr<Serializable> &ptr) {
    m_dataInput.readObject(ptr);
  }

  std::vector<char16_t> readCharArray() {
    return m_dataInput.readCharArray();
  }

  std::vector<std::string> readStringArray() {
    return m_dataInput.readStringArray();
  }

  void readArrayOfByteArrays(int8_t ***arrayofBytearr, int32_t &arrayLength,
                             int32_t **elementLength) {
    m_dataInput.readArrayOfByteArrays(arrayofBytearr, arrayLength,
                                      elementLength);
  }

  const uint8_t *currentBufferPosition() const {
    return m_dataInput.currentBufferPosition();
  }

  int32_t getBytesRead() const { return m_dataInput.getBytesRead(); }

  int32_t getBytesRemaining() const { return m_dataInput.getBytesRemaining(); }

  void advanceCursor(int32_t offset) { m_dataInput.advanceCursor(offset); }

  void rewindCursor(int32_t offset) { m_dataInput.rewindCursor(offset); }

  void reset() { m_dataInput.reset(); }

  void setBuffer() { m_dataInput.setBuffer(); }

  const std::string &getPoolName() {
    return DataInputInternal::getPoolName(m_dataInput);
  }

  void setPoolName(const std::string &poolName) {
    DataInputInternal::setPoolName(m_dataInput, poolName);
  }

  template <class CharT = char, class... Tail>
  inline std::basic_string<CharT, Tail...> readUTF() {
    return m_dataInput.readUTF<CharT, Tail...>();
  }

  template <class CharT = char, class... Tail>
  inline std::basic_string<CharT, Tail...> readString() {
    return m_dataInput.readString<CharT, Tail...>();
  }

 private:
  ByteArray m_byteArray;

  DataInputUnderTest m_dataInput;
};

class DataInputTest : public ::testing::Test, protected ByteArrayFixture {
 public:
  virtual ~DataInputTest() {
    // NOP
  }
};

TEST_F(DataInputTest, ThrowsWhenReadingInputWithSizeZero) {
  TestDataInput dataInput("");

  uint8_t aByte = 0U;
  ASSERT_THROW(dataInput.read(&aByte),
               apache::geode::client::OutOfRangeException);
}

TEST_F(DataInputTest, ThrowsWhenReadingMoreBytesThanSizePassedToConstructor) {
  TestDataInput dataInput("01");

  uint8_t aByte = 0U;
  dataInput.read(&aByte);
  EXPECT_EQ(1U, aByte);

  aByte = 0U;
  ASSERT_THROW(dataInput.read(&aByte),
               apache::geode::client::OutOfRangeException);
}

TEST_F(DataInputTest, CanReadUnsignedBytesFromInput) {
  TestDataInput dataInput("FF00");

  uint8_t aByte = 0U;
  dataInput.read(&aByte);
  EXPECT_EQ(aByte, 255);

  aByte = 0U;
  dataInput.read(&aByte);
  EXPECT_EQ(aByte, 0);
}

TEST_F(DataInputTest, CanReadSignedBytesFromInput) {
  TestDataInput dataInput("807F");

  int8_t aByte = 0U;
  dataInput.read(&aByte);
  EXPECT_EQ(aByte, -128);

  aByte = 0;
  dataInput.read(&aByte);
  EXPECT_EQ(aByte, 127);
}

TEST_F(DataInputTest, CanReadABooleanFromInput) {
  bool boolArray[2] = {true, false};
  DataInputUnderTest dataInput(reinterpret_cast<uint8_t *>(boolArray), 2);

  auto aBool = dataInput.readBoolean();
  EXPECT_EQ(aBool, true);

  aBool = dataInput.readBoolean();
  EXPECT_EQ(aBool, false);
}

TEST_F(DataInputTest, CanReadAnArrayOfBytesFromInput) {
  TestDataInput dataInput("010203");

  uint8_t byteArrayCopy[4];
  dataInput.readBytesOnly(byteArrayCopy, 3);
  EXPECT_EQ(byteArrayCopy[0], 1);
  EXPECT_EQ(byteArrayCopy[1], 2);
  EXPECT_EQ(byteArrayCopy[2], 3);
}

TEST_F(DataInputTest,
       ThrowsWhenReadingMoreContinuousBytesThanSizePassedToConstructor) {
  TestDataInput dataInput("010203");

  // fails to read 4 bytes from 3 byte buffer
  uint8_t byteArrayCopy[4];
  ASSERT_THROW(dataInput.readBytesOnly(byteArrayCopy, 4),
               apache::geode::client::OutOfRangeException);
}

TEST_F(DataInputTest, CanReadIntWithAMaxSizeUnsigned64BitIntInput) {
  uint64_t intArray[1] = {std::numeric_limits<uint64_t>::max()};
  DataInputUnderTest dataInput(reinterpret_cast<uint8_t *>(intArray),
                               sizeof(intArray));

  uint64_t aInt = dataInput.readInt64();
  EXPECT_EQ(aInt, std::numeric_limits<uint64_t>::max());
}

TEST_F(DataInputTest, CanReadUTFWithAnUTFStringInput) {
  const std::string expectedString = "foobar";
  DataOutputInternal stream;
  stream.writeUTF(expectedString);

  DataInputUnderTest dataInput(stream.getBufferCopy(),
                               stream.getBufferLength());
  auto actualString = dataInput.readUTF();

  EXPECT_TRUE(expectedString == actualString);
}

TEST_F(DataInputTest, InputResetCausesPristineRead) {
  TestDataInput dataInput("010203");

  // 1) read byte off buffer
  // 2) then reset buffer back
  uint8_t aByte = 0U;
  dataInput.read(&aByte);
  dataInput.reset();

  // 3) next byte read should be start byte
  aByte = 0U;
  dataInput.read(&aByte);
  EXPECT_EQ(aByte, 1);
}

TEST_F(DataInputTest, InputRewindCausesReplayedRead) {
  TestDataInput dataInput("010203");

  uint8_t aByte = 0U;
  dataInput.read(&aByte);
  dataInput.read(&aByte);
  dataInput.read(&aByte);

  dataInput.rewindCursor(1);

  dataInput.read(&aByte);
  EXPECT_EQ(aByte, 3);
}

TEST_F(DataInputTest, TestReadUint8) {
  TestDataInput dataInput("37");
  uint8_t value = 0U;
  dataInput.read(&value);
  EXPECT_EQ((uint8_t)55U, value) << "Correct uint8_t";
}

TEST_F(DataInputTest, TestReadInt8) {
  TestDataInput dataInput("37");
  int8_t value = 0;
  dataInput.read(&value);
  EXPECT_EQ((int8_t)55, value) << "Correct int8_t";
}

TEST_F(DataInputTest, TestReadBoolean) {
  TestDataInput dataInput("01");
  bool value = false;
  dataInput.readBoolean(&value);
  EXPECT_EQ(true, value) << "Correct bool";
}

TEST_F(DataInputTest, TestReadUint8_tBytesOnly) {
  TestDataInput dataInput("BABEFACE");
  uint8_t buffer[4];
  ::memset(buffer, 0U, 4 * sizeof(uint8_t));
  dataInput.readBytesOnly(buffer, 4);
  EXPECT_EQ((uint8_t)186U, buffer[0]) << "Correct zeroth uint8_t";
  EXPECT_EQ((uint8_t)190U, buffer[1]) << "Correct first uint8_t";
  EXPECT_EQ((uint8_t)250U, buffer[2]) << "Correct second uint8_t";
  EXPECT_EQ((uint8_t)206U, buffer[3]) << "Correct third uint8_t";
}

TEST_F(DataInputTest, TestReadInt8_tBytesOnly) {
  TestDataInput dataInput("DEADBEEF");
  int8_t buffer[4];
  ::memset(buffer, 0, 4 * sizeof(int8_t));
  dataInput.readBytesOnly(buffer, 4);
  EXPECT_EQ((int8_t)-34, buffer[0]) << "Correct zeroth int8_t";
  EXPECT_EQ((int8_t)-83, buffer[1]) << "Correct first int8_t";
  EXPECT_EQ((int8_t)-66, buffer[2]) << "Correct second int8_t";
  EXPECT_EQ((int8_t)-17, buffer[3]) << "Correct third int8_t";
}

TEST_F(DataInputTest, TestReadUint8_tBytes) {
  TestDataInput dataInput("04BABEFACE");
  uint8_t *buffer = nullptr;
  int32_t len = 0;
  dataInput.readBytes(&buffer, &len);
  EXPECT_NE((uint8_t *)nullptr, buffer) << "Non-null buffer";
  ASSERT_EQ(4, len) << "Correct length";
  EXPECT_EQ((uint8_t)186U, buffer[0]) << "Correct zeroth uint8_t";
  EXPECT_EQ((uint8_t)190U, buffer[1]) << "Correct first uint8_t";
  EXPECT_EQ((uint8_t)250U, buffer[2]) << "Correct second uint8_t";
  EXPECT_EQ((uint8_t)206U, buffer[3]) << "Correct third uint8_t";
  _GEODE_SAFE_DELETE_ARRAY(buffer);
}

TEST_F(DataInputTest, TestReadInt8_tBytes) {
  TestDataInput dataInput("04DEADBEEF");
  int8_t *buffer = nullptr;
  int32_t len = 0;
  dataInput.readBytes(&buffer, &len);
  EXPECT_NE((int8_t *)nullptr, buffer) << "Non-null buffer";
  ASSERT_EQ(4, len) << "Correct length";
  EXPECT_EQ((int8_t)-34, buffer[0]) << "Correct zeroth int8_t";
  EXPECT_EQ((int8_t)-83, buffer[1]) << "Correct first int8_t";
  EXPECT_EQ((int8_t)-66, buffer[2]) << "Correct second int8_t";
  EXPECT_EQ((int8_t)-17, buffer[3]) << "Correct third int8_t";
  _GEODE_SAFE_DELETE_ARRAY(buffer);
}

TEST_F(DataInputTest, TestReadIntUint16) {
  TestDataInput dataInput("123456789ABCDEF0");
  uint16_t value = dataInput.readInt16();
  EXPECT_EQ((uint16_t)4660U, value) << "Correct uint16_t";
}

TEST_F(DataInputTest, TestReadIntInt16) {
  TestDataInput dataInput("123456789ABCDEF0");
  int16_t value = dataInput.readInt16();
  EXPECT_EQ((int16_t)4660, value) << "Correct int16_t";
}

TEST_F(DataInputTest, TestReadIntUint32) {
  TestDataInput dataInput("123456789ABCDEF0");
  uint32_t value = dataInput.readInt32();
  EXPECT_EQ((uint32_t)305419896U, value) << "Correct uint32_t";
}

TEST_F(DataInputTest, TestReadIntInt32) {
  TestDataInput dataInput("123456789ABCDEF0");
  int32_t value = dataInput.readInt32();
  EXPECT_EQ((int32_t)305419896, value) << "Correct int32_t";
}

TEST_F(DataInputTest, TestReadIntUint64) {
  TestDataInput dataInput("123456789ABCDEF0");
  uint64_t value = 0U;
  dataInput.readInt(&value);
  EXPECT_EQ((uint64_t)1311768467463790320U, value) << "Correct uint64_t";
}

TEST_F(DataInputTest, TestReadIntInt64) {
  TestDataInput dataInput("123456789ABCDEF0");
  int64_t value = dataInput.readInt64();
  EXPECT_EQ((int64_t)1311768467463790320, value) << "Correct int64_t";
}

TEST_F(DataInputTest, TestReadArrayLen) {
  int32_t len = 0;

  TestDataInput dataInput0("FF12345678");
  len = dataInput0.readArrayLen();
  EXPECT_EQ(-1, len) << "Correct length for 0xFF";

  TestDataInput dataInput1("FE12345678");
  len = dataInput1.readArrayLen();
  EXPECT_EQ(4660, len) << "Correct length for 0xFE";

  TestDataInput dataInput2("FD12345678");
  len = dataInput2.readArrayLen();
  EXPECT_EQ(305419896, len) << "Correct length for 0xFD";

  TestDataInput dataInput3("FC12345678");
  len = dataInput3.readArrayLen();
  EXPECT_EQ(252, len) << "Correct length for 0xFC";
}

TEST_F(DataInputTest, TestReadUnsignedVL) {
  // DataInput::readUnsignedVL() uses a variable-length encoding
  // that uses the top bit of a byte to indicate whether another
  // byte follows. Thus, the 64 bits must be split into 7-bit
  // groups and then the follow bit applied. Note that integer
  // is encoded byte-at-a-time from the smaller end.
  //
  //  0001  0010  0011  0100  0101  0110  0111  1000  1001  1010  1011  1100
  //  1101  1110  1111  0000
  //  0  0010010  0011010  0010101  1001111  0001001  1010101  1110011  0111101
  //  1110000
  // 00    12       1A       15       4F       09       55       73       3D 70
  // 00    92       9A       95       CF       89       D5       F3       BD F0

  TestDataInput dataInput("F0BDF3D589CF959A9200");
  int64_t value = dataInput.readUnsignedVL();
  EXPECT_EQ((int64_t)1311768467463790320, value) << "Correct int64_t";
}

TEST_F(DataInputTest, TestReadFloat) {
  TestDataInput dataInput("123456789ABCDEF0");
  float value = dataInput.readFloat();
  EXPECT_FLOAT_EQ(5.6904566e-28F, value) << "Correct float";
}

TEST_F(DataInputTest, TestReadDouble) {
  TestDataInput dataInput("123456789ABCDEF0");
  double value = 0.;
  value = dataInput.readDouble();
  EXPECT_DOUBLE_EQ(5.626349274901198e-221, value) << "Correct double";
}

TEST_F(DataInputTest, TestReadUTFNarrow) {
  TestDataInput dataInput(
      "001B596F7520686164206D65206174206D65617420746F726E61646F2E");
  auto value = dataInput.readUTF();
  ASSERT_EQ(27, value.length()) << "Correct length";
  EXPECT_EQ("You had me at meat tornado.", value) << "Correct string";
}

TEST_F(DataInputTest, TestReadObjectSharedPtr) {
  TestDataInput dataInput(
      "57001B596F7520686164206D65206174206D65617420746F726E61646F2E");
  auto objptr =
      std::static_pointer_cast<CacheableString>(dataInput.readObject());
  EXPECT_EQ("You had me at meat tornado.", objptr->value())
      << "Correct const char *";
}

TEST_F(DataInputTest, TestReadNativeBool) {
  TestDataInput dataInput("0001");
  const bool value = dataInput.readNativeBool();
  EXPECT_EQ(true, value) << "Correct bool";
}

TEST_F(DataInputTest, TestReadNativeInt32) {
  TestDataInput dataInput("0012345678");
  const int32_t value = dataInput.readNativeInt32();
  EXPECT_EQ((int32_t)305419896, value) << "Correct int32_t";
}

TEST_F(DataInputTest, TestReadDirectObject) {
  TestDataInput dataInput(
      "57001B596F7520686164206D65206174206D65617420746F726E61646F2E");
  auto objptr = dataInput.readDirectObject();
  EXPECT_EQ("You had me at meat tornado.",
            std::dynamic_pointer_cast<CacheableString>(objptr)->toString())
      << "Correct const char *";
}

TEST_F(DataInputTest, TestReadObjectSerializablePtr) {
  TestDataInput dataInput(
      "57001B596F7520686164206D65206174206D65617420746F726E61646F2E");
  std::shared_ptr<Serializable> objptr;
  dataInput.readObject(objptr);
  EXPECT_EQ("You had me at meat tornado.",
            std::dynamic_pointer_cast<CacheableString>(objptr)->toString())
      << "Correct const char *";
}

TEST_F(DataInputTest, TestReadCharArray) {
  TestDataInput dataInput(
      "1C0059006F007500200068006100640020006D00650020006100740020006D0065006100"
      "7400200074006F0072006E00610064006F002E0000");
  auto value = dataInput.readCharArray();
  ASSERT_EQ((int32_t)28, value.size()) << "Correct length";
  EXPECT_EQ(std::u16string(u"You had me at meat tornado."),
            std::u16string(value.data()))
      << "Correct const char *";
}

TEST_F(DataInputTest, TestReadString) {
  TestDataInput dataInput(
      "57001B596F7520686164206D65206174206D65617420746F726E61646F2E");
  auto value = dataInput.readString();
  EXPECT_EQ("You had me at meat tornado.", value) << "Correct std::string";
}

TEST_F(DataInputTest, TestReadStringToUtf8String) {
  auto expected = std::string(u8"You had me at");
  expected.push_back(0);
  expected.append(u8"meat tornad\u00F6!\U000F0000");

  TestDataInput dataInput(
      "2A0023596F7520686164206D65206174C0806D65617420746F726E6164C3B621EDAE80ED"
      "B080");
  auto str = dataInput.readString();

  EXPECT_EQ(expected, str);
}

TEST_F(DataInputTest, TestReadStringToUtf16String) {
  auto expected = std::u16string(u"You had me at");
  expected.push_back(0);
  expected.append(u"meat tornad\u00F6!\U000F0000");

  TestDataInput dataInput(
      "2A0023596F7520686164206D65206174C0806D65617420746F726E6164C3B621EDAE80ED"
      "B080");
  auto str = dataInput.readString<char16_t>();

  EXPECT_EQ(expected, str);
}

TEST_F(DataInputTest, TestReadStringToUcs4String) {
  auto expected = std::u32string(U"You had me at");
  expected.push_back(0);
  expected.append(U"meat tornad\u00F6!\U000F0000");

  TestDataInput dataInput(
      "2A0023596F7520686164206D65206174C0806D65617420746F726E6164C3B621EDAE80ED"
      "B080");
  auto str = dataInput.readString<char32_t>();

  EXPECT_EQ(expected, str);
}

TEST_F(DataInputTest, TestReadStringToWideString) {
  auto expected = std::wstring(L"You had me at");
  expected.push_back(0);
  expected.append(L"meat tornad\u00F6!\U000F0000");

  TestDataInput dataInput(
      "2A0023596F7520686164206D65206174C0806D65617420746F726E6164C3B621EDAE80ED"
      "B080");
  auto str = dataInput.readString<wchar_t>();

  EXPECT_EQ(expected, str);
}

TEST_F(DataInputTest, TestReadStringArray) {
  TestDataInput dataInput(
      "0157001B596F7520686164206D65206174206D65617420746F726E61646F2E");
  auto value = dataInput.readStringArray();
  ASSERT_EQ(1, value.size()) << "Correct length";
  EXPECT_EQ("You had me at meat tornado.", value[0]) << "Correct char *";
}

TEST_F(DataInputTest, TestReadArrayOfByteArrays) {
  TestDataInput dataInput("0104DEADBEEF");
  int8_t **arrayOfByteArrays = nullptr;
  int32_t arrayLength = 0;
  int32_t *elementLength = nullptr;
  dataInput.readArrayOfByteArrays(&arrayOfByteArrays, arrayLength,
                                  &elementLength);
  EXPECT_NE((int8_t **)nullptr, arrayOfByteArrays)
      << "Non-null array of byte arrays";
  ASSERT_EQ((int32_t)1, arrayLength) << "Correct array length";
  EXPECT_NE((int8_t *)nullptr, arrayOfByteArrays[0])
      << "Non-null first byte array";
  ASSERT_EQ(4, elementLength[0]) << "Correct length";
  EXPECT_EQ((int8_t)-34, arrayOfByteArrays[0][0]) << "Correct zeroth int8_t";
  EXPECT_EQ((int8_t)-83, arrayOfByteArrays[0][1]) << "Correct first int8_t";
  EXPECT_EQ((int8_t)-66, arrayOfByteArrays[0][2]) << "Correct second int8_t";
  EXPECT_EQ((int8_t)-17, arrayOfByteArrays[0][3]) << "Correct third int8_t";
  _GEODE_SAFE_DELETE_ARRAY(elementLength);
  _GEODE_SAFE_DELETE_ARRAY(arrayOfByteArrays);
}

TEST_F(DataInputTest, TestGetBytesRead) {
  TestDataInput dataInput("123456789ABCDEF0");
  EXPECT_EQ((int32_t)0, dataInput.getBytesRead())
      << "Correct bytes read before any reads";
  uint8_t value = 0U;
  dataInput.read(&value);
  dataInput.read(&value);
  dataInput.read(&value);
  dataInput.read(&value);
  EXPECT_EQ((int32_t)4, dataInput.getBytesRead())
      << "Correct bytes read after half of the reads";
  dataInput.read(&value);
  dataInput.read(&value);
  dataInput.read(&value);
  dataInput.read(&value);
  EXPECT_EQ((int32_t)8, dataInput.getBytesRead())
      << "Correct bytes read after all of the reads";
}

TEST_F(DataInputTest, TestGetBytesRemaining) {
  TestDataInput dataInput("123456789ABCDEF0");
  EXPECT_EQ((int32_t)8, dataInput.getBytesRemaining())
      << "Correct bytes remaining before any reads";
  uint8_t value = 0U;
  dataInput.read(&value);
  dataInput.read(&value);
  dataInput.read(&value);
  dataInput.read(&value);
  EXPECT_EQ((int32_t)4, dataInput.getBytesRemaining())
      << "Correct bytes remaining after half of the reads";
  dataInput.read(&value);
  dataInput.read(&value);
  dataInput.read(&value);
  dataInput.read(&value);
  EXPECT_EQ((int32_t)0, dataInput.getBytesRemaining())
      << "Correct bytes remaining after all of the reads";
}

TEST_F(DataInputTest, TestAdvanceCursor) {
  TestDataInput dataInput("123456789ABCDEF0");
  EXPECT_EQ((int32_t)0, dataInput.getBytesRead())
      << "Correct bytes read before any advancement";
  EXPECT_EQ((int32_t)8, dataInput.getBytesRemaining())
      << "Correct bytes remaining before any advancement";
  dataInput.advanceCursor(5);
  EXPECT_EQ((int32_t)5, dataInput.getBytesRead())
      << "Correct bytes read after forward advancement";
  EXPECT_EQ((int32_t)3, dataInput.getBytesRemaining())
      << "Correct bytes remaining after forward advancement";
  dataInput.advanceCursor(-3);
  EXPECT_EQ((int32_t)2, dataInput.getBytesRead())
      << "Correct bytes read after rearward advancement";
  EXPECT_EQ((int32_t)6, dataInput.getBytesRemaining())
      << "Correct bytes remaining after rearward advancement";
}

TEST_F(DataInputTest, TestRewindCursor) {
  TestDataInput dataInput("123456789ABCDEF0");
  EXPECT_EQ((int32_t)0, dataInput.getBytesRead())
      << "Correct bytes read before any rewinding";
  EXPECT_EQ((int32_t)8, dataInput.getBytesRemaining())
      << "Correct bytes remaining before any rewinding";
  dataInput.rewindCursor(-5);
  EXPECT_EQ((int32_t)5, dataInput.getBytesRead())
      << "Correct bytes read after forward rewinding";
  EXPECT_EQ((int32_t)3, dataInput.getBytesRemaining())
      << "Correct bytes remaining after forward rewinding";
  dataInput.rewindCursor(3);
  EXPECT_EQ((int32_t)2, dataInput.getBytesRead())
      << "Correct bytes read after rearward rewinding";
  EXPECT_EQ((int32_t)6, dataInput.getBytesRemaining())
      << "Correct bytes remaining after rearward rewinding";
}

TEST_F(DataInputTest, TestReset) {
  TestDataInput dataInput("123456789ABCDEF0");
  EXPECT_EQ((int32_t)0, dataInput.getBytesRead())
      << "Correct bytes read before any reads";
  EXPECT_EQ((int32_t)8, dataInput.getBytesRemaining())
      << "Correct bytes remaining before any reads";
  uint8_t value = 0U;
  dataInput.read(&value);
  dataInput.read(&value);
  dataInput.read(&value);
  dataInput.read(&value);
  EXPECT_EQ((int32_t)4, dataInput.getBytesRead())
      << "Correct bytes read after the reads";
  EXPECT_EQ((int32_t)4, dataInput.getBytesRemaining())
      << "Correct bytes remaining after the reads";
  dataInput.reset();
  EXPECT_EQ((int32_t)0, dataInput.getBytesRead())
      << "Correct bytes read after the reset";
  EXPECT_EQ((int32_t)8, dataInput.getBytesRemaining())
      << "Correct bytes remaining after the reset";
}

TEST_F(DataInputTest, TestSetBuffer) {
  TestDataInput dataInput("123456789ABCDEF0");
  EXPECT_EQ((int32_t)0, dataInput.getBytesRead())
      << "Correct bytes read before any reads";
  EXPECT_EQ((int32_t)8, dataInput.getBytesRemaining())
      << "Correct bytes remaining before any reads";
  uint8_t value = 0U;
  dataInput.read(&value);
  dataInput.read(&value);
  dataInput.read(&value);
  dataInput.read(&value);
  EXPECT_EQ((int32_t)4, dataInput.getBytesRead())
      << "Correct bytes read after the reads";
  EXPECT_EQ((int32_t)4, dataInput.getBytesRemaining())
      << "Correct bytes remaining after the reads";
  dataInput.setBuffer();
  EXPECT_EQ((int32_t)4, dataInput.getBytesRead())
      << "Correct bytes read after the setting";
  EXPECT_EQ((int32_t)0, dataInput.getBytesRemaining())
      << "Correct bytes remaining after the setting";
}

TEST_F(DataInputTest, TestSetPoolName) {
  std::string poolName = "Das Schwimmbad";

  TestDataInput dataInput("123456789ABCDEF0");
  EXPECT_TRUE(DataInputInternal::getPoolName(dataInput).empty())
      << "Empty pool name before setting";
  DataInputInternal::setPoolName(dataInput, poolName);
  EXPECT_FALSE(DataInputInternal::getPoolName(dataInput).empty())
      << " pool name after setting";
  EXPECT_EQ(poolName, DataInputInternal::getPoolName(dataInput))
      << "Correct pool name after setting";
}

}  // namespace
