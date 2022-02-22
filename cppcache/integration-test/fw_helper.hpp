#pragma once

#ifndef GEODE_INTEGRATION_TEST_FW_HELPER_H_
#define GEODE_INTEGRATION_TEST_FW_HELPER_H_

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

UNIT testing framework for C++.

By including fw_helper.hpp 3 macros are defined for use in writing
modular unit tests.

BEGIN_TEST(x) where x is the name of the test case.
  Use this to mark the beginning of a test case. When one test case
  fails through an ASSERT, testing continues with the next case.

END_TEST(x) where x is the name of the test case.
  Use this to mark the end of a test case.

The use of BEGIN_TEST and END_TEST actually define and create an instance of
a derivation of TestOp. When TestOps are created they are added to a list
of operations to be executed. The code between BEGIN_TEST and END_TEST becomes
the body of a void method that is invoked when the TestOp is executed.

ASSERT(cond,message) where cond is a conditional expression,
  and message is the failure message. If cond does not evaluate to a notion
  of true, then an TestException is created and thrown. The executor of
  TestOps will catch these exceptions, print them, including the file and
  line information from where the ASSERT was called. It will then move on
  to the next TestOp.

For Example... A test file may look like:
--------------------------------------------------------------------

// Include product classes prior to fw_helper.hpp

#include "fw_helper.hpp"

#include <iostream>

BEGIN_TEST(TestSomeCode)
  bool flag = false;
  ASSERT( flag == true, "This one will throw an exception." );
END_TEST(TestSomeCode)

BEGIN_TEST(AnotherTest)
  std::cout << "this test will get run, regardless of TestSomeCode failing." <<
std::endl;
END_TEST(AnotherTest)
---------------------------------------------------------------------

fw_helper.hpp contains the main for you, all you have to do is declare some
test behaviors, and they'll be handled run.

fwtest_Name is defined for you, as a (const char *) with the value given to
BEGIN_TEST.

*/

#ifdef WIN32
// Must include WinSock2 so winsock.h doesn't get included.
#if WINVER == 0x0500
#undef _WINSOCKAPI_
#define NOMINMAX
#include <WinSock2.h>
#endif
#endif

#include <cstdio>
#include <iostream>
#include <list>
#include <sstream>
#include <string>
#include <typeinfo>


#include <geode/Exception.hpp>

#include <CppCacheLibrary.hpp>
#include <util/Log.hpp>

#include "TimeBomb.hpp"

#define ASSERT(x, y) \
  if (!(x)) throw TestException(y, __LINE__, __FILE__)

#define ASSERT1(x) ASSERT(x, "Expected: " #x)

#define ASSERT_EQ(x, y) _ASSERT_EQ(x, y, __LINE__, __FILE__)

#define ASSERT_STREQ(x, y)                                            \
  ASSERT(x == y,                                         \
         (std::string("Assert: " #x " == " #y ", Expected: \"") + x + \
          "\", Actual: \"" + y + "\"")                                \
             .c_str())

#define FAIL(y) throw TestException(y, __LINE__, __FILE__)

#define LOG(y) dunit::log(y, __LINE__, __FILE__, 0)

#define SLEEP(x) dunit::sleep(x)

namespace dunit {
void log(std::string s, int lineno, const char* filename, int id);
void sleep(int millis);
void setupCRTOutput();  // disable windows popups.
}  // namespace dunit

class TestException {
 public:
  TestException(const std::string& msg, int lineno, const std::string& filename)
      : m_message(msg), m_lineno(lineno), m_filename(filename) {}

  void print() {
    std::cout << "--->"
              << apache::geode::client::Log::formatLogLine(
                     apache::geode::client::LogLevel::Error)
              << "TestException: " << m_message << " in " << m_filename
              << " at line " << m_lineno << "<---\n";
  }
  std::string m_message;
  int m_lineno;
  std::string m_filename;
};

// std::list holding names of all tests that failed.
std::list<std::string> failed;

class TestOp {
 public:
  TestOp() {}

  virtual ~TestOp() {}
  virtual void setup() {
    std::cout << "## running test - " << m_name << " ##\n";
  }
  virtual void doTest() { std::cout << "do something useful.\n"; }
  void run() {
    try {
      this->setup();
      this->doTest();
    } catch (TestException& e) {
      e.print();
      failed.push_back(m_name);
    } catch (apache::geode::client::Exception& ge) {
      std::cout << ge.getStackTrace() << "\n";
      failed.push_back(m_name);
    }
  }
  virtual std::string typeName() { return std::string(typeid(*this).name()); }
  virtual void init();
  std::string m_name;
};

class SuiteMember {
 public:
  explicit SuiteMember(TestOp* test) : m_test(test) {}
  TestOp* m_test;
};

// std::list holding all registered TestOp instances.
std::list<SuiteMember> tests;

void TestOp::init() {
  m_name = this->typeName();
  tests.push_back(SuiteMember(this));
  std::cout << "Queued test[" << m_name << "].\n";
}

// main - suite trigger

extern "C" {

#ifdef WIN32
int wmain()
#else
int main(int /*argc*/, char** /*argv*/)
#endif
{
  dunit::setupCRTOutput();
  apache::geode::client::CppCacheLibrary::initLib();
  // TimeBomb* tb = new TimeBomb();
  // tb->arm();
  try {
    int testsRun = 0;
    int failures = 0;
    std::cout << "Beginning test Suite.\n";
    while (!tests.empty()) {
      SuiteMember aTest = tests.front();
      aTest.m_test->run();
      tests.pop_front();
      testsRun++;
    }
    while (!failed.empty()) {
      std::string name = failed.front();
      std::cout << "test named \"" << name << "\" failed.\n";
      failures++;
      failed.pop_front();
    }
    if (failures != 0) {
      std::cout << failures << " tests failed.\n";
    }
    std::cout << (testsRun - failures) << " tests passed.\n";
    apache::geode::client::CppCacheLibrary::closeLib();
    return failures;
  } catch (...) {
    std::cout << "Exception: unhandled/unidentified exception reached main.\n"
              << std::flush;
  }
  return 1;
}
}

#define BEGIN_TEST(x)                      \
  class Test_##x : virtual public TestOp { \
   public:                                 \
    Test_##x() { init(); }                 \
                                           \
   public:                                 \
    void doTest() override {               \
      static const char* fwtest_Name = #x;
#define END_TEST(x) \
  }                 \
  }                 \
  a_##x;

template <class _Expected, class _Actual>
void _ASSERT_EQ(const _Expected& expected, const _Actual& actual, int line,
                const char* file) {
  if (expected != actual) {
    std::stringstream ss;
    ss << "ASSERT_EQ: Expected: " << expected << ", Actual: " << actual;
    throw TestException(ss.str().c_str(), line, file);
  }
}

#endif  // GEODE_INTEGRATION_TEST_FW_HELPER_H_
