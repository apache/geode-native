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



#include "Log.hpp"
#include "ExceptionTypes.hpp"
#include "impl/ManagedString.hpp"
#include "impl/SafeConvert.hpp"


using namespace System;

namespace Apache
{
  namespace Geode
  {
    namespace Client
    {

      void Log::Init(LogLevel level, String^ logFileName)
      {
        try {

          ManagedString mg_lfname(logFileName);
          apache::geode::client::Log::init(static_cast<apache::geode::client::LogLevel>(level),
                                         mg_lfname.CharPtr);

        }
        catch (const apache::geode::client::Exception& ex) {
          throw Apache::Geode::Client::GeodeException::Get(ex);
        }
        catch (System::AccessViolationException^ ex) {
          throw ex;
        }
      }

      void Log::Init(LogLevel level, String^ logFileName, System::Int32 logFileLimit)
      {
        try {

          ManagedString mg_lfname(logFileName);
          apache::geode::client::Log::init(static_cast<apache::geode::client::LogLevel>(level),
                                         mg_lfname.CharPtr, logFileLimit);

        }
        catch (const apache::geode::client::Exception& ex) {
          throw Apache::Geode::Client::GeodeException::Get(ex);
        }
        catch (System::AccessViolationException^ ex) {
          throw ex;
        }
      }

      void Log::Close()
      {
        apache::geode::client::Log::close();
      }

      LogLevel Log::Level()
      {
        return static_cast<LogLevel>(apache::geode::client::Log::logLevel());
      }

      void Log::SetLevel(LogLevel level)
      {
        apache::geode::client::Log::setLogLevel(
          static_cast<apache::geode::client::LogLevel>(level));
      }

      bool Log::Enabled(LogLevel level)
      {
        return apache::geode::client::Log::enabled(
          static_cast<apache::geode::client::LogLevel>(level));
      }

      void Log::Write(LogLevel level, String^ msg)
      {
        try {

          ManagedString mg_msg(msg);
          apache::geode::client::Log::log(static_cast<apache::geode::client::LogLevel>(level),
                                        mg_msg.CharPtr);

        }
        catch (const apache::geode::client::Exception& ex) {
          throw Apache::Geode::Client::GeodeException::Get(ex);
        }
        catch (System::AccessViolationException^ ex) {
          throw ex;
        }
      }

      void Log::LogThrow(LogLevel level, String^ msg, System::Exception^ ex)
      {
        if (ex != nullptr)
        {
          String^ logMsg = String::Format(
            System::Globalization::CultureInfo::CurrentCulture,
            "Geode exception {0} thrown: {1}{2}{3}", ex->GetType(),
            ex->Message, Environment::NewLine, msg);
          Log::Write(level, logMsg);
        }
      }

      void Log::LogCatch(LogLevel level, String^ msg, System::Exception^ ex)
      {
        if (ex != nullptr)
        {
          String^ logMsg = String::Format(
            System::Globalization::CultureInfo::CurrentCulture,
            "Geode exception {0} caught: {1}{2}{3}", ex->GetType(),
            ex->Message, Environment::NewLine, msg);
          Log::Write(level, logMsg);
        }  // namespace Client
      }  // namespace Geode
    }  // namespace Apache

  }
}
