
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

#ifdef USE_SMARTHEAP
#include <smrtheap.h>
#endif

#include <string>
#include <iostream>
#include <iomanip>
#include <list>
#include <map>

#include <ace/Get_Opt.h>

#include <boost/asio.hpp>
#include <boost/process.hpp>
#include <boost/interprocess/mapped_region.hpp>

#ifdef _WIN32
#include <boost/interprocess/windows_shared_memory.hpp>
#else
#include <boost/interprocess/shared_memory_object.hpp>
#endif

#include "fw_spawn.hpp"
#include "fwklib/FwkException.hpp"

#define __DUNIT_NO_MAIN__
#include "fw_dunit.hpp"

namespace bip = boost::interprocess;

static std::string g_programName;
static uint32_t g_coordinatorPid = 0;

ClientCleanup gClientCleanup;

namespace dunit {

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
// some common values..
#define WORKER_STATE_READY 1
#define WORKER_STATE_DONE 2
#define WORKER_STATE_TASK_ACTIVE 3
#define WORKER_STATE_TASK_COMPLETE 4
#define WORKER_STATE_SCHEDULED 5

void log(std::string s, int lineno, const char *filename);

/** uniquely represent each different worker. */
class WorkerId {
 private:
  uint32_t m_id;
  static const char *m_idNames[];

 public:
  explicit WorkerId(uint32_t id) { m_id = id; }

  int getId() const { return m_id; }

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

class TestState {
 public:
  static const auto WORKER_COUNT = 4U;

  void reset();

  void setWorkerTimeout(int id, int seconds);

  int getWorkerTimeout(int id) const;

  void setWorkerState(int id, uint8_t state);

  int getWorkerState(int id) const;

  void setNextWorker(int id);

  int getNextWorker();

  void fail();

  bool failed() const;

  void terminate();

  bool terminated() const;

 private:
  bool failure_;
  bool terminate_;
  int next_worker_;
  int worker_timeout_[WORKER_COUNT];
  uint8_t worker_state_[WORKER_COUNT];
};

/** main framework entry */
class Dunit {
 private:
  static const auto MANAGED_STATE_SIZE = 1UL << 17UL;
  static Dunit *singleton;

  bool coordinator_;
  bip::mapped_region globals_region_;
#ifdef _WIN32
  bip::windows_shared_memory globals_shm_;
#else
  bip::shared_memory_object globals_shm_;
#endif
  bip::managed_shared_memory managed_state_;

  explicit Dunit(bool coordinator) : coordinator_(coordinator) {
    if (coordinator) {
      removeStates();

#ifdef _WIN32
      globals_shm_ =
          bip::windows_shared_memory{bip::create_only, getSharedName(),
                                     bip::read_write, sizeof(TestState)};
#else
      globals_shm_ = bip::shared_memory_object{
          bip::create_only, getSharedName(), bip::read_write};
      globals_shm_.truncate(sizeof(TestState));
#endif

      managed_state_ = bip::managed_shared_memory{
          bip::create_only, getManagedStateName(), MANAGED_STATE_SIZE};
    } else {
      using shared_memory =
#ifdef _WIN32
          bip::windows_shared_memory;
#else
          bip::shared_memory_object;
#endif

      globals_shm_ =
          shared_memory{bip::open_only, getSharedName(), bip::read_write};
      managed_state_ =
          bip::managed_shared_memory{bip::open_only, getManagedStateName()};
    }

    globals_region_ = bip::mapped_region{globals_shm_, bip::read_write};

    if (coordinator) {
      getState()->reset();
    }
  }

  ~Dunit() {
    if (coordinator_) {
      removeStates();
    }
  }

  static void removeStates() {
    bip::shared_memory_object::remove(getSharedName());
    bip::shared_memory_object::remove(getManagedStateName());
  }

  static const char *getSharedName() {
    static std::string name = std::string{std::getenv("TESTNAME")} + '.' +
                              std::to_string(g_coordinatorPid);
    return name.c_str();
  }

  static const char *getManagedStateName() {
    static std::string name = std::string{std::getenv("TESTNAME")} +
                              ".managed." + std::to_string(g_coordinatorPid);
    return name.c_str();
  }

 public:
  /** call this once just inside main... */
  static void init(bool coordinator) { singleton = new Dunit(coordinator); }

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

  TestState *getState() {
    return reinterpret_cast<TestState *>(globals_region_.get_address());
  }

  bip::managed_shared_memory &getManagedState() { return managed_state_; }
};

#define DUNIT dunit::Dunit::getSingleton()

Dunit *Dunit::singleton = nullptr;

void TestState::reset() {
  next_worker_ = 0;
  failure_ = false;
  terminate_ = false;

  for (auto i = 0U; i < WORKER_COUNT; ++i) {
    worker_state_[i] = 0;
    worker_timeout_[i] = -1;
  }
}

void TestState::setWorkerTimeout(int id, int seconds) {
  worker_timeout_[id - 1] = seconds;
}

int TestState::getWorkerTimeout(int id) const {
  return worker_timeout_[id - 1];
}

void TestState::setWorkerState(int id, uint8_t state) {
  worker_state_[id - 1] = state;
}

int TestState::getWorkerState(int id) const { return worker_state_[id - 1]; }

void TestState::setNextWorker(int id) { next_worker_ = id; }

int TestState::getNextWorker() { return next_worker_; }

void TestState::fail() { failure_ = true; }

bool TestState::failed() const { return failure_; }

void TestState::terminate() { terminate_ = true; }

bool TestState::terminated() const { return terminate_; }

void Task::setTimeout(int seconds) {
  auto state = DUNIT->getState();
  if (seconds > 0) {
    state->setWorkerTimeout(m_id, seconds);
  } else {
    state->setWorkerTimeout(m_id, TASK_TIMEOUT);
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

 public:
  TestDriver() {
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
    for (uint32_t i = 0; i < TestState::WORKER_COUNT;) {
      auto worker = m_workers[i++];
      if (worker->running() == 1) {
        delete worker;  // worker destructor should terminate process.
      }
    }
    dunit::Dunit::close();
  }

  int begin() {
    fprintf(stdout, "Coordinator started with pid %d\n",
            boost::this_process::get_id());
    fflush(stdout);
    waitForReady();
    // dispatch task...

    int nextWorker;
    auto state = DUNIT->getState();
    while ((nextWorker = TaskQueues::getWorkerId()) != 0) {
      WorkerId sId(nextWorker);
      state->setWorkerState(nextWorker, WORKER_STATE_SCHEDULED);
      fprintf(stdout, "Set next process to %s\n", sId.getIdName());
      fflush(stdout);

      state->setNextWorker(nextWorker);
      waitForCompletion(sId);
      // check special conditions.
      if (state->failed()) {
        state->terminate();
        waitForDone();
        return 1;
      }
    }

    // end all work..
    state->terminate();
    waitForDone();
    return 0;
  }

  /** wait for an individual worker to finish a task. */
  void waitForCompletion(WorkerId &sId) {
    auto id = sId.getId();
    auto state = DUNIT->getState();

    int secs = state->getWorkerTimeout(id);
    state->setWorkerTimeout(id, TASK_TIMEOUT);
    if (secs <= 0) {
      secs = TASK_TIMEOUT;
    }

    fprintf(stdout, "Waiting %d seconds for %s to finish task.\n", secs,
            sId.getIdName());
    fflush(stdout);
    auto end = std::chrono::steady_clock::now() + std::chrono::seconds{secs};
    while (state->getWorkerState(id) != WORKER_STATE_TASK_COMPLETE) {
      // sleep a bit..
      if (state->failed()) {
        return;
      }

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

    auto state = DUNIT->getState();
    state->terminate();
    state->fail();
  }

  void handleTimeout(WorkerId &sId) {
    fprintf(stdout, "Error: Timed out waiting for %s to finish task.\n",
            sId.getIdName());
    fflush(stdout);

    auto state = DUNIT->getState();
    state->terminate();
    state->fail();
  }

  /** wait for all workers
   * to be done initializing. */
  void waitForReady() {
    auto state = DUNIT->getState();
    fprintf(stdout, "Waiting %d seconds for all workers to be ready.\n",
            TASK_TIMEOUT);
    fflush(stdout);

    auto end =
        std::chrono::steady_clock::now() + std::chrono::seconds{TASK_TIMEOUT};

    uint32_t readyCount = 0;
    while (readyCount < TestState::WORKER_COUNT) {
      fprintf(stdout, "Ready Count: %d\n", readyCount);
      fflush(stdout);

      if (state->failed()) {
        return;
      }

      std::this_thread::sleep_for(std::chrono::seconds{1});

      readyCount = 0;
      for (uint32_t i = 1; i < 5; i++) {
        int status = state->getWorkerState(i);
        if (status == WORKER_STATE_READY) {
          ++readyCount;
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
    auto state = DUNIT->getState();
    fprintf(stdout, "Waiting %d seconds for all workers to complete.\n",
            TASK_TIMEOUT);
    fflush(stdout);

    uint32_t doneCount = 0;
    auto end =
        std::chrono::steady_clock::now() + std::chrono::seconds{TASK_TIMEOUT};

    while (doneCount < TestState::WORKER_COUNT) {
      // if ( DUNIT->getFailed() ) return;
      // sleep a bit..
      std::this_thread::sleep_for(std::chrono::milliseconds{100});
      doneCount = 0;
      for (uint32_t i = 1; i < 5; i++) {
        int status = state->getWorkerState(i);
        if (status == WORKER_STATE_DONE) {
          ++doneCount;
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
    auto state = DUNIT->getState();
    for (uint32_t i = 0; i < TestState::WORKER_COUNT; i++) {
      if (!m_workers[i]->running()) {
        char msg[1000] = {0};
        sprintf(msg, "Error: Worker %s terminated prematurely.",
                m_workers[i]->getWorkerId().getIdName());
        LOG(msg);

        state->fail();
        state->terminate();
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
    dunit::Dunit::init(false);
    DUNIT->getState()->setWorkerState(m_sId.getId(), WORKER_STATE_READY);
    std::clog << "Started worker " << id << std::endl;
  }

  ~TestWorker() {
    DUNIT->getState()->setWorkerState(m_sId.getId(), WORKER_STATE_DONE);
    dunit::Dunit::close();
  }

  void begin() {
    auto state = DUNIT->getState();
    fprintf(stdout, "Worker %s started with pid %d\n", m_sId.getIdName(),
            boost::this_process::get_id());
    fflush(stdout);

    // consume tasks of this workers queue, only when it is his turn..
    while (!state->terminated()) {
      if (state->getNextWorker() == m_sId.getId()) {
        // set next worker to zero so I don't accidently run twice.
        state->setNextWorker(0);
        // do next task...
        Task *task = TaskQueues::getTask(m_sId);
        // perform task.
        if (task != nullptr) {
          state->setWorkerState(m_sId.getId(), WORKER_STATE_TASK_ACTIVE);
          try {
            task->doTask();
            if (task->m_isHeapAllocated) {
              delete task;
            }
            fflush(stdout);
            state->setWorkerState(m_sId.getId(), WORKER_STATE_TASK_COMPLETE);
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
    auto state = DUNIT->getState();

    state->fail();
    state->terminate();
    state->setWorkerState(m_sId.getId(), WORKER_STATE_TASK_COMPLETE);
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
  using std::chrono::duration_cast;
  using std::chrono::microseconds;
  using std::chrono::system_clock;

  auto now = system_clock::now();
  auto in_time_t = system_clock::to_time_t(now);
  auto localtime = std::localtime(&in_time_t);
  auto usec =
      duration_cast<microseconds>(now.time_since_epoch()).count() % 1000;

  std::cout << "[TEST " << std::put_time(localtime, "%Y/%m/%d %H:%M:%S") << '.'
            << std::setfill('0') << std::setw(6) << usec << std::setw(0)
            << " coordinator:pid(" << boost::this_process::get_id() << ")] "
            << s << " at line: " << lineno << std::endl
            << std::flush;
}

// log a message and print the worker id as well.. used by fw_helper with no
// worker id.
void log(std::string s, int lineno, const char * /*filename*/, int /*id*/) {
  using std::chrono::duration_cast;
  using std::chrono::microseconds;
  using std::chrono::system_clock;

  auto now = system_clock::now();
  auto in_time_t = system_clock::to_time_t(now);
  auto localtime = std::localtime(&in_time_t);
  auto usec =
      duration_cast<microseconds>(now.time_since_epoch()).count() % 1000;

  std::cout << "[TEST " << std::put_time(localtime, "%Y/%m/%d %H:%M:%S") << '.'
            << std::setfill('0') << std::setw(6) << usec << std::setw(0)
            << " 0:pid(" << boost::this_process::get_id() << ")] " << s
            << " at line: " << lineno << std::endl
            << std::flush;
}

// log a message and print the worker id as well..
void log(std::string s, int lineno, const char * /*filename*/) {
  using std::chrono::duration_cast;
  using std::chrono::microseconds;
  using std::chrono::system_clock;

  auto now = system_clock::now();
  auto in_time_t = system_clock::to_time_t(now);
  auto localtime = std::localtime(&in_time_t);
  auto usec =
      duration_cast<microseconds>(now.time_since_epoch()).count() % 1000;

  std::cout << "[TEST " << std::put_time(localtime, "%Y/%m/%d %H:%M:%S") << '.'
            << std::setfill('0') << std::setw(6) << usec << std::setw(0) << ' '
            << (dunit::TestWorker::procWorkerId
                    ? dunit::TestWorker::procWorkerId->getIdName()
                    : "coordinator")
            << ":pid(" << boost::this_process::get_id() << ")] " << s
            << " at line: " << lineno << std::endl
            << std::flush;
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
bip::managed_shared_memory &globals() { return DUNIT->getManagedState(); }

}  // namespace dunit
