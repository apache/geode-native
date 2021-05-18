#pragma once

#ifndef GEODE_INTEGRATION_TEST_FW_SPAWN_H_
#define GEODE_INTEGRATION_TEST_FW_SPAWN_H_

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
// Spawn.cpp,v 1.4 2004/01/07 22:40:16 shuston Exp

// @TODO, this out this include list..

#if defined(_WIN32)
#if (FD_SETSIZE != 1024)
+++bad fdsetsize...
#endif
#endif

#include <ace/Process.h>
#include <ace/Log_Msg.h>
#include <boost/iostreams/device/file_descriptor.hpp>

  namespace dunit {

  // Listing 1 code/ch10
  class Manager : virtual public ACE_Process {
   public:
    explicit Manager(const std::string &program_name)
        : ACE_Process{}, programName_{program_name} {}

    virtual int doWork(void) {
      // Spawn the new process; prepare() hook is called first.
      ACE_Process_Options options;
      pid_t pid = this->spawn(options);
      if (pid == -1) {
        ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("%p\n"), ACE_TEXT("spawn")), -1);
      }
      return pid;
    }

   protected:
    int prepare(ACE_Process_Options &options) override {
      options.command_line("%s", this->programName_.c_str());
      if (this->setStdHandles(options) == -1 ||
          this->setEnvVariable(options) == -1) {
        return -1;
      }
      return 0;
    }

    virtual int setStdHandles(ACE_Process_Options &options) {
      boost::filesystem::path p{this->programName_};

      std::string tmp = p.filename().string();
      boost::replace_all(tmp, " ", "_");
      boost::replace_all(tmp, "-", "_");

      auto stderr_path = p.parent_path();
      stderr_path += boost::filesystem::path::preferred_separator;
      stderr_path += tmp;

      std::string stderr_name = stderr_path.string();
      std::remove(stderr_name.c_str());

      outputfd_.open(stderr_name,
                     std::ios::in | std::ios::out | std::ios::trunc);
      return options.set_handles(ACE_STDIN, ACE_STDOUT, outputfd_.handle());
    }

    virtual int setEnvVariable(ACE_Process_Options &options) {
      return options.setenv("PRIVATE_VAR=/that/seems/to/be/it");
    }
    // Listing 2

   private:
   protected:
    ~Manager() noexcept override = default;

   private:
    std::string programName_;
    boost::iostreams::file_descriptor outputfd_;
  };

}  // namespace dunit.

#endif  // GEODE_INTEGRATION_TEST_FW_SPAWN_H_
