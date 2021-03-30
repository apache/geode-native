
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

#ifdef WIN32
// ace.dll was built with FD_SETSIZE of 1024, so ensure it stays that way.
#undef FD_SETSIZE
#define FD_SETSIZE 1024
#if WINVER == 0x0500
#undef _WINSOCKAPI_
#define NOMINMAX
#include <WinSock2.h>
#endif
#endif

#ifdef USE_SMARTHEAP
#include <smrtheap.h>
#endif

#include <string>
#include <list>
#include <map>

#include <boost/asio.hpp>
#include <boost/process.hpp>

// SW: Switching to framework BB on linux also since it is much faster.
#ifndef _WIN32
// On solaris, when ACE_Naming_Context maps file to memory using fixed mode, it
// interfere with malloc/brk system calls later cause failure. For now, we use
// the Black Board from the regression test framework. When the ACE problem is
// fixed in a new release we'll go back to original code by undefining
// SOLARIS_USE_BB
#define SOLARIS_USE_BB 1
#endif

#define VALUE_MAX 128

#ifdef SOLARIS_USE_BB
#include "BBNamingContext.hpp"
using apache::geode::client::testframework::BBNamingContextClient;
using apache::geode::client::testframework::BBNamingContextServer;
#else
#include <ace/Naming_Context.h>
#endif

#include <ace/Guard_T.h>
#include <ace/Get_Opt.h>
#include <ace/Time_Value.h>
#include <ace/SV_Semaphore_Complex.h>

#include "fw_spawn.hpp"
#include "fwklib/FwkException.hpp"

#define __DUNIT_NO_MAIN__
#include "fw_dunit.hpp"

static std::string g_programName;
static uint32_t g_coordinatorPid = 0;

ClientCleanup gClientCleanup;

namespace dunit {

void HostWaitForDebugger() {
  int done = 0;
  LOG("host wait for debugger.");
  while (!done) {
    sleep(1);
  }
}

void setupCRTOutput() {
#ifdef _WIN32
#ifdef DEBUG
  int reportMode = _CRTDBG_MODE_FILE | _CRTDBG_MODE_WNDW;
  _CrtSetReportMode(_CRT_ASSERT, reportMode);
  _CrtSetReportFile(_CRT_ASSERT, _CRTDBG_FILE_STDERR);
  _CrtSetReportMode(_CRT_ERROR, reportMode);
  _CrtSetReportFile(_CRT_ERROR, _CRTDBG_FILE_STDERR);
  _CrtSetReportMode(_CRT_WARN, reportMode);
  _CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDERR);
  SetErrorMode(SEM_FAILCRITICALERRORS);
#endif
#endif
}

void getTimeStr(char *bufPtr, size_t sizeOfBuf) {
  ACE_TCHAR timestamp[64] = {0};  // only 35 needed here
  ACE::timestamp(timestamp, sizeof timestamp);
  // timestamp is like "Tue May 17 2005 12:54:22.546780"
  // for our purpose we just want "12:54:22.546780"
  strncpy(bufPtr, &timestamp[0], sizeOfBuf);
}

// some common values..
#define WORKER_STATE_READY 1
#define WORKER_STATE_DONE 2
#define WORKER_STATE_TASK_ACTIVE 3
#define WORKER_STATE_TASK_COMPLETE 4
#define WORKER_STATE_SCHEDULED 5

void log(std::string s, int lineno, const char *filename);

/** Naming service for sharing data between processes. */
class NamingContextImpl : virtual public NamingContext {
 private:
#ifdef SOLARIS_USE_BB
  BBNamingContextClient
#else
  ACE_Naming_Context
#endif

      m_context;

  void millisleep(int msec) {
    std::this_thread::sleep_for(std::chrono::milliseconds{msec});
  }

  int checkResult(int result, const char *func) {
    if (result == -1) {
      LOGCOORDINATOR("NamingCtx operation failed for:");
      LOGCOORDINATOR(func);
      LOGCOORDINATOR("Dump follows:");
      dump();
      throw -1;
    }
    return result;
  }

 public:
  NamingContextImpl() : m_context() {
    open();
    LOGCOORDINATOR("Naming context ready.");
  }

  ~NamingContextImpl() noexcept override {
    m_context.close();
    std::remove(std::getenv("TESTNAME"));
  }

  /**
   * Share a string value, return -1 if there is a failure to store value,
   * otherwise returns 0.
   */
  int rebind(const char *key, const char *value) override {
    int res = -1;
    int attempts = 10;
    while ((res = m_context.rebind(key, value, const_cast<char *>(""))) == -1 &&
           attempts--) {
      millisleep(10);
    }
    return checkResult(res, "rebind");
  }

  /**
   * Share an int value, return -1 if there is a failure to store value,
   * otherwise returns 0.
   */
  int rebind(const char *key, int value) override {
    return rebind(key, std::to_string(value).c_str());
  }

  /**
   * retreive a value by key, storing the result in the users buf. If the key
   * is not found, the buf will contain the empty string "".
   */
  std::string getValue(const std::string &key) override {
#ifdef SOLARIS_USE_BB
    std::string value;
    char type[VALUE_MAX] = {0};
#else
    char *type = nullptr;
    char *value = nullptr;
#endif

    int attempts = 3;
    while (m_context.resolve(key.c_str(), value, type) != 0 && attempts--) {
      // we should not increase sleep to avoid increasing test run times.
      millisleep(5);
    }

#ifndef SOLARIS_USE_BB
    if (value == nullptr) {
      return {};
    }
#endif

    return value;
  }

  /**
   * return the value by key, as an int using the string to int conversion
   * rules of atoi.
   */
  int getIntValue(const std::string &key) override {
    auto val = getValue(key);
    return val.empty() ? 0 : std::stoi(val);
  }

  void open() {
#ifdef SOLARIS_USE_BB
    m_context.open();
#else
    ACE_Name_Options *name_options = m_context.name_options();
    name_options->process_name(getContextName().c_str());
    name_options->namespace_dir(".");
    name_options->context(ACE_Naming_Context::PROC_LOCAL);
    name_options->database(std::getenv("TESTNAME"));
    checkResult(m_context.open(name_options->context(), 0), "open");
#endif
    LOGCOORDINATOR("Naming context opened.");
  }

  std::string getContextName() {
    return "dunit.context." +
           boost::filesystem::path{g_programName}.filename().stem().string() +
           std::to_string(g_coordinatorPid);
  }

  std::string getMutexName() {
    return "dunit.mutex." +
           boost::filesystem::path{g_programName}.filename().stem().string() +
           std::to_string(g_coordinatorPid);
  }

  /** print out all the entries' keys and values in the naming context. */
  void dump() override {
#ifdef SOLARIS_USE_BB
    m_context.dump();
#else
    ACE_BINDING_SET set;
    if (this->m_context.list_name_entries(set, "") != 0) {
      char buf[1000] = {0};
      ::sprintf(buf, "There is nothing in the naming context.");
      LOGCOORDINATOR(buf);
    } else {
      ACE_BINDING_ITERATOR set_iterator(set);
      for (ACE_Name_Binding *entry = 0; set_iterator.next(entry) != 0;
           set_iterator.advance()) {
        ACE_Name_Binding binding(*entry);
        char buf[1000] = {0};
        ::sprintf(buf, "%s => %s", binding.name_.char_rep(),
                  binding.value_.char_rep());
        LOGCOORDINATOR(buf);
      }
    }
#endif
  }

  void resetContext() {
    char buf[30] = {0};
    sprintf(buf, "%d", boost::this_process::get_id());

    int res1 = -1;
    int attempts1 = 10;
    while ((res1 = m_context.rebind("Driver", buf)) == -1 && attempts1--) {
      millisleep(10);
    }
    checkResult(res1, "rebind1");

    int res2 = -1;
    int attempts2 = 10;
    while ((res2 = m_context.rebind("WorkerId", "0")) == -1 && attempts2--) {
      millisleep(10);
    }
    checkResult(res2, "rebind2");

    LOGCOORDINATOR("Naming context reset.");
  }
};

/** uniquely represent each different worker. */
class WorkerId {
 private:
  uint32_t m_id;
  static const char *m_idNames[];

 public:
  explicit WorkerId(uint32_t id) { m_id = id; }

  int getId() { return m_id; }

  const char *getIdName() { return m_idNames[m_id]; }

  /** return the system id for this process */
  int getSystem() { return 1; }
  /** return the process id for this system. */
  int getProcOnSys() { return ((m_id % 2) == 0) ? 2 : 1; }
};

const char *WorkerId::m_idNames[] = {"none", "s1p1", "s1p2", "s2p1", "s2p2"};

/** method for letting Task discover its name through RTTI. */
std::string Task::typeName() { return std::string(typeid(*this).name()); }

typedef std::list<Task *> TaskList;

/** contains a queue of Task* for each WorkerId. */
class TaskQueues {
 private:
  std::map<int, TaskList> m_qmap;
  std::list<int> m_schedule;

  TaskQueues() : m_qmap(), m_schedule() {}

  void registerTask(WorkerId sId, Task *task) {
    m_qmap[sId.getId()].push_back(task);
    m_schedule.push_back(sId.getId());
  }

  Task *nextTask(WorkerId &sId) {
    TaskList *tasks = &(m_qmap[sId.getId()]);
    if (tasks->empty()) {
      return nullptr;
    }
    Task *task = tasks->front();
    if (task != nullptr) {
      char logmsg[1024] = {0};
      sprintf(logmsg, "received task: %s ", task->m_taskName.c_str());
      LOG(logmsg);
      tasks->pop_front();
    }
    return task;
  }

  int nextWorkerId() {
    if (m_schedule.empty()) {
      return 0;
    }
    int sId = m_schedule.front();
    char logmsg[1024] = {0};
    sprintf(logmsg, "Next worker id is : %d", sId);
    LOGCOORDINATOR(logmsg);
    m_schedule.pop_front();
    return sId;
  }

  static TaskQueues *taskQueues;

 public:
  static void addTask(WorkerId sId, Task *task) {
    if (taskQueues == nullptr) {
      taskQueues = new TaskQueues();
    }
    taskQueues->registerTask(sId, task);
  }

  static int getWorkerId() {
    ASSERT(taskQueues != nullptr, "failure to initialize fw_dunit module.");
    return taskQueues->nextWorkerId();
  }

  static Task *getTask(WorkerId sId) {
    ASSERT(taskQueues != nullptr, "failure to initialize fw_dunit module.");
    return taskQueues->nextTask(sId);
  }
};

TaskQueues *TaskQueues::taskQueues = nullptr;

/** register task with worker. */
void Task::init(int sId) { init(sId, false); }

void Task::init(int sId, bool isHeapAllocated) {
  m_isHeapAllocated = isHeapAllocated;
  m_id = sId;
  m_taskName = this->typeName();
  TaskQueues::addTask(WorkerId(sId), this);
}

/** main framework entry */
class Dunit {
 private:
  NamingContextImpl m_globals;
  static Dunit *singleton;
  bool m_close_down;

  Dunit() : m_globals(), m_close_down(false) {}

  void resetContext() {
    m_close_down = true;
    m_globals.resetContext();
  }

 public:
  /** call this once just inside main... */
  static void init(bool initContext = false) {
    if (initContext) {
      std::remove("localnames");
      std::remove("name_space_localnames");
      std::remove("backing_store_localnames");
    }
    singleton = new Dunit();
    if (initContext) {
      singleton->resetContext();
    }
  }

  /** return the already initialized singleton Dunit instance. */
  static Dunit *getSingleton() {
    ASSERT(singleton != nullptr, "singleton not created yet.");
    return singleton;
  }

  /** delete the existing singleton */
  static void close() {
    Dunit *tmp = singleton;
    singleton = nullptr;
    delete tmp;
  }

  /** set the next worker id */
  void setNextWorker(WorkerId &sId) {
    m_globals.rebind("WorkerId", sId.getId());
  }

  /** get the next worker id */
  int getNextWorker() { return m_globals.getIntValue("WorkerId"); }

  /** return true if all workers are to terminate. */
  bool mustQuit() {
    return m_globals.getIntValue("TerminateAllWorkers") ? true : false;
  }

  /** signal all workers to terminate. */
  void setMustQuit() { m_globals.rebind("TerminateAllWorkers", 1); }

  /** signal to test driver that an error occurred. */
  void setFailed() { m_globals.rebind("Failure", 1); }

  bool getFailed() { return m_globals.getIntValue("Failure") ? true : false; }

  void setWorkerState(WorkerId sId, int state) {
    m_globals.rebind(("ReadyWorker" + std::to_string(sId.getId())).c_str(),
                     state);
  }

  int getWorkerState(WorkerId sId) {
    return m_globals.getIntValue("ReadyWorker" + std::to_string(sId.getId()));
  }

  void setWorkerTimeout(WorkerId sId, int seconds) {
    m_globals.rebind(("TimeoutWorker" + std::to_string(sId.getId())).c_str(),
                     seconds);
  }

  int getWorkerTimeout(WorkerId sId) {
    return m_globals.getIntValue("TimeoutWorker" + std::to_string(sId.getId()));
  }

  /** return the NamingContext for global (amongst all processes) values. */
  NamingContext *globals() { return &m_globals; }

  ~Dunit() {}
};

#define DUNIT dunit::Dunit::getSingleton()

Dunit *Dunit::singleton = nullptr;

void Task::setTimeout(int seconds) {
  if (seconds > 0) {
    DUNIT->setWorkerTimeout(WorkerId(m_id), seconds);
  } else {
    DUNIT->setWorkerTimeout(WorkerId(m_id), TASK_TIMEOUT);
  }
}

class TestProcess : virtual public dunit::Manager {
 private:
  WorkerId m_sId;

 public:
  TestProcess(const std::string &cmdline, uint32_t id)
      : Manager(cmdline), m_sId(id) {}

  WorkerId &getWorkerId() { return m_sId; }

 protected:
 public:
  ~TestProcess() noexcept override = default;
};

/**
 * Container of TestProcess(es) held in driver. each represents one of the
 * legal WorkerIds spawned when TestDriver is created.
 */
class TestDriver {
 private:
  TestProcess *m_workers[4];
#ifdef SOLARIS_USE_BB
  BBNamingContextServer *m_bbNamingContextServer;
#endif

 public:
  TestDriver() {
#ifdef SOLARIS_USE_BB
    m_bbNamingContextServer = new BBNamingContextServer();
    std::this_thread::sleep_for(std::chrono::seconds(5));
    fprintf(stdout, "Blackboard started\n");
    fflush(stdout);
#endif

    dunit::Dunit::init(true);
    fprintf(stdout, "Coordinator starting workers.\n");
    for (uint32_t i = 1; i < 5; i++) {
      std::string cmdline;
      auto profilerCmd = std::getenv("PROFILERCMD");
      if (profilerCmd != nullptr && profilerCmd[0] != '$' &&
          profilerCmd[0] != '\0') {
        // replace %d's in profilerCmd with PID and worker ID
        char cmdbuf[2048] = {0};
        auto now = std::chrono::time_point_cast<std::chrono::milliseconds>(
                       std::chrono::system_clock::now())
                       .time_since_epoch()
                       .count();
        ::sprintf(cmdbuf, profilerCmd, now, g_coordinatorPid, i);
        cmdline = std::string{cmdbuf} + ' ' + g_programName + " -s" +
                  std::to_string(i) + " -m" + std::to_string(g_coordinatorPid);
      } else {
        cmdline = g_programName + " -s" + std::to_string(i) + " -m" +
                  std::to_string(g_coordinatorPid);
      }
      fprintf(stdout, "%s\n", cmdline.c_str());
      m_workers[i - 1] = new TestProcess(cmdline, i);
    }
    fflush(stdout);
    // start each of the workers...
    for (uint32_t j = 1; j < 5; j++) {
      m_workers[j - 1]->doWork();
      std::this_thread::sleep_for(
          std::chrono::seconds(2));  // do not increase this to avoid precheckin
                                     // runs taking much longer.
    }
  }

  ~TestDriver() {
    // kill off any children that have not yet terminated.
    for (uint32_t i = 1; i < 5; i++) {
      if (m_workers[i - 1]->running() == 1) {
        delete m_workers[i - 1];  // worker destructor should terminate process.
      }
    }
    dunit::Dunit::close();
#ifdef SOLARIS_USE_BB
    delete m_bbNamingContextServer;
    m_bbNamingContextServer = nullptr;
#endif
  }

  int begin() {
    fprintf(stdout, "Coordinator started with pid %d\n",
            boost::this_process::get_id());
    fflush(stdout);
    waitForReady();
    // dispatch task...

    int nextWorker;
    while ((nextWorker = TaskQueues::getWorkerId()) != 0) {
      WorkerId sId(nextWorker);
      DUNIT->setWorkerState(sId, WORKER_STATE_SCHEDULED);
      fprintf(stdout, "Set next process to %s\n", sId.getIdName());
      fflush(stdout);
      DUNIT->setNextWorker(sId);
      waitForCompletion(sId);
      // check special conditions.
      if (DUNIT->getFailed()) {
        DUNIT->setMustQuit();
        waitForDone();
        return 1;
      }
    }

    // end all work..
    DUNIT->setMustQuit();
    waitForDone();
    return 0;
  }

  /** wait for an individual worker to finish a task. */
  void waitForCompletion(WorkerId &sId) {
    int secs = DUNIT->getWorkerTimeout(sId);
    DUNIT->setWorkerTimeout(sId, TASK_TIMEOUT);
    if (secs <= 0) secs = TASK_TIMEOUT;
    fprintf(stdout, "Waiting %d seconds for %s to finish task.\n", secs,
            sId.getIdName());
    fflush(stdout);
    auto end = std::chrono::steady_clock::now() + std::chrono::seconds{secs};
    while (DUNIT->getWorkerState(sId) != WORKER_STATE_TASK_COMPLETE) {
      // sleep a bit..
      if (DUNIT->getFailed()) return;
      std::this_thread::sleep_for(std::chrono::milliseconds{100});
      checkWorkerDeath();
      auto now = std::chrono::steady_clock::now();
      if (now >= end) {
        handleTimeout(sId);
        break;
      }
    }
  }

  void handleTimeout() {
    fprintf(stdout, "Error: Timed out waiting for all workers to be ready.\n");
    fflush(stdout);
    DUNIT->setMustQuit();
    DUNIT->setFailed();
  }

  void handleTimeout(WorkerId &sId) {
    fprintf(stdout, "Error: Timed out waiting for %s to finish task.\n",
            sId.getIdName());
    fflush(stdout);
    DUNIT->setMustQuit();
    DUNIT->setFailed();
  }

  /** wait for all workers to be done initializing. */
  void waitForReady() {
    fprintf(stdout, "Waiting %d seconds for all workers to be ready.\n",
            TASK_TIMEOUT);
    fflush(stdout);
    auto end =
        std::chrono::steady_clock::now() + std::chrono::seconds{TASK_TIMEOUT};
    uint32_t readyCount = 0;
    while (readyCount < 4) {
      fprintf(stdout, "Ready Count: %d\n", readyCount);
      fflush(stdout);

      if (DUNIT->getFailed()) {
        return;
      }

      std::this_thread::sleep_for(std::chrono::seconds{1});

      readyCount = 0;
      for (uint32_t i = 1; i < 5; i++) {
        int state = DUNIT->getWorkerState(WorkerId(i));
        if (state == WORKER_STATE_READY) {
          readyCount++;
        }
      }
      checkWorkerDeath();
      auto now = std::chrono::steady_clock::now();
      if (now >= end) {
        handleTimeout();
        break;
      }
    }
  }

  /** wait for all workers to be destroyed. */
  void waitForDone() {
    fprintf(stdout, "Waiting %d seconds for all workers to complete.\n",
            TASK_TIMEOUT);
    fflush(stdout);

    uint32_t doneCount = 0;
    auto end =
        std::chrono::steady_clock::now() + std::chrono::seconds{TASK_TIMEOUT};

    while (doneCount < 4) {
      // if ( DUNIT->getFailed() ) return;
      // sleep a bit..
      std::this_thread::sleep_for(std::chrono::milliseconds{100});
      doneCount = 0;
      for (uint32_t i = 1; i < 5; i++) {
        int state = DUNIT->getWorkerState(WorkerId(i));
        if (state == WORKER_STATE_DONE) {
          doneCount++;
        }
      }
      auto now = std::chrono::steady_clock::now();
      if (now >= end) {
        handleTimeout();
        break;
      }
    }
  }

  /** test to see that all the worker processes are still around, or throw
      a TestException so the driver doesn't get hung. */
  void checkWorkerDeath() {
    for (uint32_t i = 0; i < 4; i++) {
      if (!m_workers[i]->running()) {
        char msg[1000] = {0};
        sprintf(msg, "Error: Worker %s terminated prematurely.",
                m_workers[i]->getWorkerId().getIdName());
        LOG(msg);
        DUNIT->setFailed();
        DUNIT->setMustQuit();
        FAIL(msg);
      }
    }
  }
};

class TestWorker {
 private:
  WorkerId m_sId;

 public:
  static WorkerId *procWorkerId;

  explicit TestWorker(int id) : m_sId(id) {
    procWorkerId = new WorkerId(id);
    dunit::Dunit::init();
    DUNIT->setWorkerState(m_sId, WORKER_STATE_READY);
  }

  ~TestWorker() {
    DUNIT->setWorkerState(m_sId, WORKER_STATE_DONE);
    dunit::Dunit::close();
  }

  void begin() {
    fprintf(stdout, "Worker %s started with pid %d\n", m_sId.getIdName(),
            boost::this_process::get_id());
    fflush(stdout);
    WorkerId workerZero(0);

    // consume tasks of this workers queue, only when it is his turn..
    while (!DUNIT->mustQuit()) {
      if (DUNIT->getNextWorker() == m_sId.getId()) {
        // set next worker to zero so I don't accidently run twice.
        DUNIT->setNextWorker(workerZero);
        // do next task...
        Task *task = TaskQueues::getTask(m_sId);
        // perform task.
        if (task != nullptr) {
          DUNIT->setWorkerState(m_sId, WORKER_STATE_TASK_ACTIVE);
          try {
            task->doTask();
            if (task->m_isHeapAllocated) {
              delete task;
            }
            fflush(stdout);
            DUNIT->setWorkerState(m_sId, WORKER_STATE_TASK_COMPLETE);
          } catch (TestException te) {
            if (task->m_isHeapAllocated) {
              delete task;
            }
            te.print();
            handleError();
            return;
          } catch (...) {
            if (task->m_isHeapAllocated) {
              delete task;
            }
            LOG("Unhandled exception, terminating.");
            handleError();
            return;
          }
        }
      }

      std::this_thread::sleep_for(std::chrono::milliseconds{100});
    }
  }

  void handleError() {
    DUNIT->setFailed();
    DUNIT->setMustQuit();
    DUNIT->setWorkerState(m_sId, WORKER_STATE_TASK_COMPLETE);
  }
};

WorkerId *TestWorker::procWorkerId = nullptr;

void sleep(int millis) {
  if (millis == 0) {
    std::this_thread::yield();
  } else {
    std::this_thread::sleep_for(std::chrono::milliseconds{millis});
  }
}

void logCoordinator(std::string s, int lineno, const char * /*filename*/) {
  char buf[128] = {0};
  dunit::getTimeStr(buf, sizeof(buf));

  fprintf(stdout, "[TEST coordinator:pid(%d)] %s at line: %d\n",
          boost::this_process::get_id(), s.c_str(), lineno);
  fflush(stdout);
}

// log a message and print the worker id as well.. used by fw_helper with no
// worker id.
void log(std::string s, int lineno, const char * /*filename*/, int /*id*/) {
  char buf[128] = {0};
  dunit::getTimeStr(buf, sizeof(buf));

  fprintf(stdout, "[TEST 0:pid(%d)] %s at line: %d\n",
          boost::this_process::get_id(), s.c_str(), lineno);
  fflush(stdout);
}

// log a message and print the worker id as well..
void log(std::string s, int lineno, const char * /*filename*/) {
  char buf[128] = {0};
  dunit::getTimeStr(buf, sizeof(buf));

  fprintf(stdout, "[TEST %s %s:pid(%d)] %s at line: %d\n", buf,
          (dunit::TestWorker::procWorkerId
               ? dunit::TestWorker::procWorkerId->getIdName()
               : "coordinator"),
          boost::this_process::get_id(), s.c_str(), lineno);
  fflush(stdout);
}

void cleanup() { gClientCleanup.callClientCleanup(); }

int dmain(int argc, char *argv[]) {
#ifdef USE_SMARTHEAP
  MemRegisterTask();
#endif
  setupCRTOutput();
  TimeBomb tb(&cleanup);
  // tb->arm(); // leak this on purpose.
  try {
    g_programName = argv[0];
    const ACE_TCHAR options[] = ACE_TEXT("s:m:");
    ACE_Get_Opt cmd_opts(argc, argv, options);

    int result = 0;

    int workerId = 0;
    int option = 0;
    while ((option = cmd_opts()) != EOF) {
      switch (option) {
        case 's':
          workerId = std::stoul(cmd_opts.opt_arg());
          fprintf(stdout, "Using process id: %d\n", workerId);
          fflush(stdout);
          break;
        case 'm':
          g_coordinatorPid = std::stoul(cmd_opts.opt_arg());
          fprintf(stdout, "Using coordinator id: %d\n", g_coordinatorPid);
          fflush(stdout);
          break;
        default:
          fprintf(stdout, "ignoring option: %s  with value %s\n",
                  cmd_opts.last_option(), cmd_opts.opt_arg());
          fflush(stdout);
      }
    }

    //  perf::NamingServiceThread nsvc( 12045 );
    //  nsvc.activate( THR_NEW_LWP | THR_DETACHED | THR_DAEMON, 1 );
    //  dunit::Dunit::init( true );
    //
    //  for ( int i = cmd_opts.opt_ind(); i < argc; i++ ) {
    //    char buf[1024], * name, * value;
    //    strcpy( buf, argv[i] );
    //    name = &buf[0];
    //    value = strchr( name, '=' );
    //    if ( value != 0 ) {
    //      *value = '\0';
    //      value++;
    //      // add to context
    //      dunit::globals()->rebind( name, value );
    //    }
    //  }

    // record the coordinator pid if it wasn't passed to us on the command line.
    // the TestDriver will pass this to the child processes.
    // currently this is used for giving a unique per run id to shared
    // resources.
    if (g_coordinatorPid == 0) {
      g_coordinatorPid = boost::this_process::get_id();
    }

    if (workerId > 0) {
      dunit::TestWorker worker(workerId);
      worker.begin();
    } else {
      dunit::TestDriver tdriver;
      result = tdriver.begin();
      if (result == 0) {
        printf("#### All Tasks completed successfully. ####\n");
      } else {
        printf("#### FAILED. ####\n");
      }

      fflush(stdout);
    }
    printf("final worker id %d, result %d\n", workerId, result);
    printf("before calling cleanup %d \n", workerId);
    gClientCleanup.callClientCleanup();
    printf("after calling cleanup\n");
    return result;

  } catch (dunit::TestException &te) {
    te.print();
  } catch (apache::geode::client::testframework::FwkException &fe) {
    printf("Exception: %s\n", fe.what());
    fflush(stdout);
  } catch (std::exception &ex) {
    printf("Exception: system exception reached main: %s.\n", ex.what());
    fflush(stdout);
  } catch (...) {
    printf("Exception: unhandled/unidentified exception reached main.\n");
    fflush(stdout);
  }

  gClientCleanup.callClientCleanup();
  return 1;
}

/** entry point for test code modules to access the naming service. */
NamingContext *globals() { return DUNIT->globals(); }

}  // namespace dunit

namespace perf {

TimeStamp::TimeStamp(int64_t msec) : m_msec(msec) {}

TimeStamp::TimeStamp() {
  m_msec = std::chrono::time_point_cast<std::chrono::milliseconds>(
               std::chrono::system_clock::now())
               .time_since_epoch()
               .count();
}

TimeStamp::TimeStamp(const TimeStamp &other) : m_msec(other.m_msec) {}

TimeStamp &TimeStamp::operator=(const TimeStamp &other) {
  m_msec = other.m_msec;
  return *this;
}

TimeStamp::~TimeStamp() {}

int64_t TimeStamp::msec() const { return m_msec; }

void TimeStamp::msec(int64_t t) { m_msec = t; }

Record::Record(std::string testName, int64_t ops, const TimeStamp &start,
               const TimeStamp &stop)
    : m_testName(testName),
      m_operations(ops),
      m_startTime(start),
      m_stopTime(stop) {}

Record::Record()
    : m_testName(""), m_operations(0), m_startTime(0), m_stopTime(0) {}

Record::Record(const Record &other)
    : m_testName(other.m_testName),
      m_operations(other.m_operations),
      m_startTime(other.m_startTime),
      m_stopTime(other.m_stopTime) {}

Record &Record::operator=(const Record &other) {
  m_testName = other.m_testName;
  m_operations = other.m_operations;
  m_startTime = other.m_startTime;
  m_stopTime = other.m_stopTime;
  return *this;
}

void Record::write(apache::geode::client::DataOutput &output) {
  output.writeString(m_testName);
  output.writeInt(m_operations);
  output.writeInt(m_startTime.msec());
  output.writeInt(m_stopTime.msec());
}

void Record::read(apache::geode::client::DataInput &input) {
  m_testName = input.readString();
  m_operations = input.readInt64();
  m_startTime.msec(input.readInt64());
  m_stopTime.msec(input.readInt64());
}

Record::~Record() {}

int Record::elapsed() {
  return static_cast<int>(m_stopTime.msec() - m_startTime.msec());
}

int Record::perSec() {
  return static_cast<int>(((static_cast<double>(1000) * m_operations) /
                           static_cast<double>(elapsed())) +
                          0.5);
}

std::string Record::asString() {
  std::string tmp = m_testName;
  char *buf = new char[1000];
  sprintf(buf, " -- %d ops/sec, ", perSec());
  tmp += buf;
  sprintf(buf, "%d ops, ", static_cast<int>(m_operations));
  tmp += buf;
  sprintf(buf, "%d millis", elapsed());
  tmp += buf;
  return tmp;
}

PerfSuite::PerfSuite(const char *suiteName) : m_suiteName(suiteName) {}

void PerfSuite::addRecord(std::string testName, int64_t ops,
                          const TimeStamp &start, const TimeStamp &stop) {
  Record tmp(testName, ops, start, stop);
  fprintf(stdout, "[PerfSuite] %s\n", tmp.asString().c_str());
  fflush(stdout);
}

/** create a file in cwd, named "<suite>_results.<host>" */
void PerfSuite::save() {
  /* Currently having trouble with windows... not useful until the compare
     function is written anyway...

  apache::geode::client::DataOutput output;
  output.writeASCII( m_suiteName.c_str(), m_suiteName.length() );

  char hname[100];
  ACE_OS::hostname( hname, 100 );
  std::string fname = m_suiteName + "_results." + hname;

  output.writeASCII( hname );

  for( RecordMap::iterator iter = m_records.begin(); iter != m_records.end();
  iter++ ) {
    Record record = (*iter).second;
    record.write( output );
  }
  fprintf( stdout, "[PerfSuite] finished serializing results.\n" );
  fflush( stdout );

  fprintf( stdout, "[PerfSuite] writing results to %s\n", fname.c_str() );
  FILE* of = ACE_OS::fopen( fname.c_str(), "a+" );
  if ( of == 0 ) {
    FAIL( "failed to open result file handle for perfSuite." );
  }
  LOG( "opened perf output file for a+" );
  uint32_t len = 0;
  char* buf = (char*) output.getBuffer( &len );
  LOG( "got buffer." );
  ACE_OS::fwrite( buf, len, 1, of );
  LOG( "wrote buffer" );
  ACE_OS::fflush( of );
  LOG( "flushed of" );
  ACE_OS::fclose( of );
  LOG( "closed of" );
  fprintf( stdout, "[PerfSuite] finished saving results file %s\n",
  fname.c_str() );
  fflush( stdout );
  */
}

/** load data saved in $ENV{'baselines'} named "<suite>_baseline.<host>" */
void PerfSuite::compare() {
  /*
    char hname[100] = {0};
    ACE_OS::hostname(hname, 100);
    std::string fname = m_suiteName + "_baseline." + hname;
    */
}

ThreadLauncher::ThreadLauncher(int thrCount, Thread &thr)
    : m_thrCount(thrCount),
      m_initSemaphore((-1 * thrCount) + 1),
      m_startSemaphore(0),
      m_stopSemaphore((-1 * thrCount) + 1),
      m_cleanSemaphore(0),
      m_termSemaphore((-1 * thrCount) + 1),
      m_startTime(nullptr),
      m_stopTime(nullptr),
      m_threadDef(thr) {
  m_threadDef.init(this);
}

void ThreadLauncher::go() {
#ifdef WIN32
  int thrAttrs = THR_NEW_LWP | THR_JOINABLE;
#else
  int thrAttrs = THR_NEW_LWP | THR_JOINABLE | THR_INHERIT_SCHED;
#endif
  int res = m_threadDef.activate(thrAttrs, m_thrCount);
  ASSERT(res == 0, "Failed to start threads properly");
  m_initSemaphore.acquire();
  LOG("[ThreadLauncher] all threads ready.");
  m_startTime = new TimeStamp();
  m_startSemaphore.release(m_thrCount);
  m_stopSemaphore.acquire();
  m_stopTime = new TimeStamp();
  m_cleanSemaphore.release(m_thrCount);
  m_termSemaphore.acquire();
  LOG("[ThreadLauncher] joining threads.");
  m_threadDef.wait();
  LOG("[ThreadLauncher] all threads stopped.");
}

ThreadLauncher::~ThreadLauncher() {
  if (m_startTime) {
    delete m_startTime;
  }
  if (m_stopTime) {
    delete m_stopTime;
  }
}

int Thread::svc() {
  m_used = true;
  int res = 0;
  try {
    setup();  // do per thread setup
  } catch (...) {
    LOG("[Thread] unknown exception thrown in setup().");
    res = 1;
  }
  m_launcher->initSemaphore().release();
  m_launcher->startSemaphore().acquire();
  try {
    perftask();  // do measured iterations
  } catch (...) {
    LOG("[Thread] unknown exception thrown in perftask().");
    res = 2;
  }
  m_launcher->stopSemaphore().release();
  m_launcher->cleanSemaphore().acquire();
  try {
    cleanup();  // cleanup after thread.
  } catch (...) {
    LOG("[Thread] unknown exception thrown in cleanup()");
    res = 3;
  }
  m_launcher->termSemaphore().release();
  return res;
}

Semaphore::Semaphore(int count) : m_mutex(), m_cond(m_mutex), m_count(count) {}

Semaphore::~Semaphore() {}

void Semaphore::acquire(int t) {
  ACE_Guard<ACE_Thread_Mutex> _guard(m_mutex);

  while (m_count < t) {
    m_cond.wait();
  }
  m_count -= t;
}

void Semaphore::release(int t) {
  ACE_Guard<ACE_Thread_Mutex> _guard(m_mutex);

  m_count += t;
  if (m_count > 0) {
    m_cond.broadcast();
  }
}

}  // namespace perf
