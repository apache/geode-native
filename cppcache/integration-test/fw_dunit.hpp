#pragma once

#ifndef GEODE_INTEGRATION_TEST_FW_DUNIT_H_
#define GEODE_INTEGRATION_TEST_FW_DUNIT_H_

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

/***

A Dunit framework provides a structure for executing sequential portions of
test code in different processes. This framework provides C++ tests with
a tools to define tasks and associating them to different processes. It
automatically maintains the sequence order that these tasks are to be
executing in. Process startup and termination is handled by the framework.

Task control:

The framework spawns four child processes. Two processes run with GF_DEFAULT
set to distributed system 1, and the other two run with GF_DEFAULT set to
distributed system 2. The processes correspond with the ids:

  s1p1 and s1p2 for system 1, s2p1 and s2p2 for system 2.

Two macros are used to define blocks of test code, and what process it will
run in.

DUNIT_TASK(x,y) - beginning of block to run in process x, with uniq id y.
END_TASK(y)     - ending of block started by previous DUNIT_TASK call.

Tasks are executed in the order, from top to bottom, that they are found in
the test source file. If any task throws an uncaught exception, the framework
will catch the exception and consider it a test failure. If the exception is
of type dunit::TestException, then it will be logged.
(@TODO: catch and log apache::geode::client::Exception types, and
std::exceptions. )

DUNIT_TASK actually begins the definition of a subclass of dunit::Task. And the
task Method doTask. The block of code between DUNIT_TASK and END_TASK is
compiled as the body of doTask. END_TASK closes the class definition, while
declaring an instance to be initialized at module load time. This sets up the
schedule of tasks to execute and the process they are to be executed on.

Logging and handle errors:

The logging and error reporting macros include file and line number information.

ASSERT(x,y) - if condition x is false, throw a TestException with message y.
FAIL(y)     - throw a TestException with message y.
LOG(y)      - Log message y.

Sharing test data between process is done with a NamingContext which is set up
for you automatically. Use the globals() function to get a pointer to it.
With the naming context is like a map associating char* values with char*
keys. Convenience methods are made available that take and return int values.
See class NamingContext in this file.

The the covers, the framework is implemented in the fw_dunit.cpp and
fw_spawn.hpp sources. fw_dunit.cpp is compiled as a seperate object module
without dependence on the product. It is in a seperate module that is linked in
two each test executable. The seperation is designed to help keep the test
framework from being corrupt by the product, and prevent the framework from
mucking with the product in unforseen ways.

---------------------------------------
An example dunit test file:
---------------------------------------

#include <fw_dunit.hpp>

DUNIT_TASK(s1p1,setupp1)
{
  // test code... put ten entries...
  globals()->rebind( "entryCount", 10 );
}
END_TASK(setupp1)

DUNIT_TASK(s2p1,validate)
{
  // check results from distribution...
  int expectedEntries = globals()->getIntValue( "entryCount" );
  // ...
  ASSERT( foundEntries == expectedEntries, "wrong entryCount" );
}
END_TASK(validate)

----------------------------------------

*/

// Default task timeout in seconds
#ifndef TASK_TIMEOUT
#define TASK_TIMEOUT 1800
#endif

#ifdef WINVER
#if WINVER == 0x0500
#undef _WINSOCKAPI_
#define NOMINMAX
#include <WinSock2.h>
#endif
#endif

#include <iostream>
#include <string>

#include <geode/Exception.hpp>

#include <boost/interprocess/managed_shared_memory.hpp>

#include <signal.h>
#include "ClientCleanup.hpp"
#include "TimeBomb.hpp"

#define ASSERT(x, y)                                     \
  do {                                                   \
    if (!(x)) {                                          \
      throw dunit::TestException(y, __LINE__, __FILE__); \
    }                                                    \
  } while (false)
#define XASSERT(x)                                        \
  do {                                                    \
    if (!(x)) {                                           \
      throw dunit::TestException(#x, __LINE__, __FILE__); \
    }                                                     \
  } while (false)
#define FAIL(y) throw dunit::TestException(y, __LINE__, __FILE__)
#define LOG(y) dunit::log(y, __LINE__, __FILE__)
#define LOGCOORDINATOR(y) dunit::logCoordinator(y, __LINE__, __FILE__)
#define SLEEP(x) dunit::sleep(x)

#define SYMJOIN(x, y) SYMJOIN2(x, y)
#define SYMJOIN2(x, y) x##y

#define DCLASSNAME(y) SYMJOIN(SYMJOIN(Task_, y), __LINE__)
#define DMAKE_STR(y) #y
#define DTASKDESC(x, y) DMAKE_STR(x line : y)

#define DCLASSDEF(y) SYMJOIN(Task_, y)
#define DVARNAME(y) SYMJOIN(SYMJOIN(a_, y), __LINE__)

// Create a test task instance, x is the process tag identifying which process
//   to run in. y is a unique task identifier used in generating a class name.
#define DUNIT_TASK(x, y)                                       \
  class DCLASSNAME(y) : virtual public dunit::Task {           \
   public:                                                     \
    DCLASSNAME(y)() { init(x); }                               \
                                                               \
   public:                                                     \
    void doTask() override {                                   \
      static const char* fwtest_Name = DTASKDESC(y, __LINE__); \
      try {
// Close the class definition produced by DUNIT_TASK macro.
// y is a unique identifier used to generate an instance variable name.
#define END_TASK(y)                               \
  }                                               \
  catch (apache::geode::client::Exception & ex) { \
    LOG(ex.getStackTrace().c_str());              \
    FAIL(ex.what());                              \
  }                                               \
  catch (std::exception & ex) {                   \
    FAIL(ex.what());                              \
  }                                               \
  catch (dunit::TestException&) {                 \
    throw;                                        \
  }                                               \
  catch (...) {                                   \
    FAIL("Unknown exception type caught.");       \
  }                                               \
  }                                               \
  }                                               \
  SYMJOIN(a_, __LINE__);
#define ENDTASK                                   \
  }                                               \
  catch (apache::geode::client::Exception & ex) { \
    LOG(ex.getStackTrace().c_str());              \
    FAIL(ex.what());                              \
  }                                               \
  catch (std::exception & ex) {                   \
    FAIL(ex.what());                              \
  }                                               \
  catch (dunit::TestException&) {                 \
    throw;                                        \
  }                                               \
  catch (...) {                                   \
    FAIL("Unknown exception type caught.");       \
  }                                               \
  }                                               \
  }                                               \
  SYMJOIN(a_, __LINE__);

#define DUNIT_TASK_DEFINITION(x, y)                            \
  class DCLASSDEF(y) : virtual public dunit::Task {            \
   public:                                                     \
    DCLASSDEF(y)() { init(x, true); }                          \
                                                               \
   public:                                                     \
    void doTask() override {                                   \
      static const char* fwtest_Name = DTASKDESC(y, __LINE__); \
      try {
#define END_TASK_DEFINITION                       \
  }                                               \
  catch (apache::geode::client::Exception & ex) { \
    LOG(ex.getStackTrace().c_str());              \
    FAIL(ex.what());                              \
  }                                               \
  catch (std::exception & ex) {                   \
    FAIL(ex.what());                              \
  }                                               \
  catch (dunit::TestException&) {                 \
    throw;                                        \
  }                                               \
  catch (...) {                                   \
    FAIL("Unknown exception type caught.");       \
  }                                               \
  }                                               \
  }                                               \
  ;
#define CALL_TASK(y) \
  ;                  \
  DCLASSDEF(y) * DVARNAME(y) = new DCLASSDEF(y)()

#define DUNIT_MAIN         \
  class DCLASSNAME(Main) { \
   public:                 \
    DCLASSNAME(Main)() {   \
      try {
#define END_MAIN                                  \
  }                                               \
  catch (apache::geode::client::Exception & ex) { \
    LOG(ex.getStackTrace().c_str());              \
    FAIL(ex.what());                              \
  }                                               \
  catch (std::exception & ex) {                   \
    FAIL(ex.what());                              \
  }                                               \
  catch (dunit::TestException&) {                 \
    throw;                                        \
  }                                               \
  catch (...) {                                   \
    FAIL("Unknown exception type caught.");       \
  }                                               \
  }                                               \
  }                                               \
  SYMJOIN(a_, __LINE__);

// identifiers for the different processes.
#define s1p1 1
#define s1p2 2
#define s2p1 3
#define s2p2 4
extern ClientCleanup gClientCleanup;
namespace dunit {

void logCoordinator(std::string s, int lineno, const char* filename);
void log(std::string s, int lineno, const char* filename);
void sleep(int millis);
void HostWaitForDebugger();

// on windows this disables the popups.
void setupCRTOutput();

/** base class for all tasks... define a virtual doTest method to be called
 * when task is to be executed. init method sets up the task in the right
 * process queue for the workers to execute.
 */
class Task {
 public:
  std::string m_taskName;
  int m_id;
  bool m_isHeapAllocated;

  Task() {}
  virtual ~Task() {}

  /** register task with worker. */
  void init(int sId);

  void init(int sId, bool isHeapAllocated);

  // defined by block of code in DUNIT_TASK macro usage.
  virtual void doTask() = 0;

  void setTimeout(int seconds = 0);

  std::string typeName();
};

extern boost::interprocess::managed_shared_memory& globals();

/**
 * Exception type to use when test framework has trouble or if ASSERT and FAIL
 * are triggered. Use the macros ASSERT(cond,message) and FAIL(message) to
 * throw, use TestException.print() to dump the issue to stderr. If not caught
 * by your test code, the framework will catch it and assume the test block
 * failed.
 */
class TestException {
 public:
  TestException(const std::string& msg, int lineno, const std::string& filename)
      : m_message(msg), m_lineno(lineno), m_filename(filename) {}

  void print() {
    std::cout << "#### TestException: " << m_message << " in " << m_filename
              << " at line " << m_lineno << "\n"
              << std::flush;
  }
  std::string m_message;
  int m_lineno;
  std::string m_filename;
};

int dmain(int argc, char* argv[]);

}  // end namespace dunit.

#ifndef __DUNIT_NO_MAIN__

int main(int argc, char* argv[]) { return dunit::dmain(argc, argv); }

#endif  // __DUNIT_NO_MAIN__

namespace test {}  // namespace test
#endif             // GEODE_INTEGRATION_TEST_FW_DUNIT_H_
